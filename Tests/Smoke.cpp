#include "../Source/Session.h"
#include "../Source/SystemStatus.h"
#include <audioclient.h>
#include <audioclientactivationparams.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <wrl/implements.h>
#include <iostream>
using namespace Microsoft::WRL;
struct Activation:RuntimeClass<RuntimeClassFlags<ClassicCom>,IActivateAudioInterfaceCompletionHandler,FtmBase>{
 HANDLE done=CreateEvent(nullptr,TRUE,FALSE,nullptr);HRESULT result=E_PENDING;ComPtr<IAudioClient> client;
 ~Activation(){CloseHandle(done);}
 HRESULT STDMETHODCALLTYPE ActivateCompleted(IActivateAudioInterfaceAsyncOperation* op)override{ComPtr<IUnknown> o;HRESULT hr=E_FAIL;result=op->GetActivateResult(&hr,&o);if(SUCCEEDED(result))result=hr;if(SUCCEEDED(result))result=o.As(&client);SetEvent(done);return S_OK;}
};
int main(int argc,char** argv){
 juce::ScopedJuceInitialiser_GUI init;
 if(argc==3&&std::string(argv[1])=="--live-state-test"){
  juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> types;format.findAllTypesForFile(types,argv[2]);if(types.size()!=1)return 40;
  juce::String error;auto instance=format.createInstanceFromDescription(*types[0],48000,512,error);if(!instance)return 41;
  const auto set=[&](const juce::String& url){juce::XmlElement xml("WebCaptureMobile");xml.setAttribute("url",url);juce::MemoryBlock processor;juce::AudioProcessor::copyXmlToBinary(xml,processor);juce::XmlElement host("VST3PluginState");host.createNewChildElement("IComponent")->addTextElement(processor.toBase64Encoding());juce::MemoryBlock data;juce::AudioProcessor::copyXmlToBinary(host,data);instance->setStateInformation(data.getData(),(int)data.getSize());};
  const auto get=[&](){juce::MemoryBlock data;instance->getStateInformation(data);auto host=juce::AudioProcessor::getXmlFromBinary(data.getData(),(int)data.getSize());auto*component=host?host->getChildByName("IComponent"):nullptr;if(!component)return juce::String();juce::MemoryBlock processor;processor.fromBase64Encoding(component->getAllSubText());auto xml=juce::AudioProcessor::getXmlFromBinary(processor.getData(),(int)processor.getSize());return xml?xml->getStringAttribute("url"):juce::String();};
  set("http://127.0.0.1:18947/state-first");std::unique_ptr<juce::AudioProcessorEditor> editor(instance->createEditorIfNeeded());Sleep(1800);for(int i=0;i<120&&get()!="http://127.0.0.1:18947/state-first";++i)Sleep(50);
  const bool first=get()=="http://127.0.0.1:18947/state-first";set("http://127.0.0.1:18947/state-second");for(int i=0;i<60&&get()!="http://127.0.0.1:18947/state-second";++i)Sleep(50);
  const auto actual=get();const bool changed=actual=="http://127.0.0.1:18947/state-second";editor.reset();instance.reset();std::cout<<"Live state first page: "<<first<<"; restored page applied: "<<changed<<"; actual="<<actual<<"\n";return first&&changed?0:42;
 }
 if(argc==3&&std::string(argv[1])=="--session-regression"){
  BrowserSession session;const auto missing=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("JoseadorMissingRuntime","",false);
  if(session.start(missing,true))return 30;
  const bool started=session.start(juce::File(juce::String::fromUTF8(argv[2])),true);if(!started)return 31;
  const juce::String destination="http://127.0.0.1:18947/queue-regression";
  session.command(1,destination);session.command(6);
  for(int i=0;i<120&&(!session.data()->ready||session.address()!=destination);++i)Sleep(50);
  const bool cleared=!session.status().contains("Falta el motor"),ordered=session.address()==destination;
  std::cout<<"Startup error cleared: "<<cleared<<"; navigate then reload retained: "<<ordered<<"\n";
  session.shutdown();return cleared&&ordered?0:32;
 }
 if(argc==2&&std::string(argv[1])=="--status"){
  SystemStatus pc;Sleep(500);
  std::cout<<"Clock "<<juce::Time::getCurrentTime().formatted("%H:%M")<<"; network="<<pc.network.load()<<"; battery="<<pc.battery.load()<<"; AC="<<pc.power.load()<<"\n";
  return pc.network>=-1&&pc.network<=2&&pc.battery>=-2&&pc.battery<=100&&pc.power>=-1&&pc.power<=1?0:20;
 }
 if(argc<3)return 1;
 juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> types;format.findAllTypesForFile(types,argv[1]);
 if(types.size()!=1||!types[0]->isInstrument){std::cerr<<"Not an instrument\n";return 2;}
 for(auto rate:{44100.0,48000.0,96000.0}){
  juce::String error;auto instance=format.createInstanceFromDescription(*types[0],rate,512,error);
  if(!instance||!instance->acceptsMidi()||instance->getTotalNumInputChannels()!=0||instance->getTotalNumOutputChannels()!=2)return 3;
  instance->prepareToPlay(rate,512);
  for(int n:{1,64,127,512}){juce::AudioBuffer<float> block(2,n);juce::MidiBuffer midi;midi.addEvent(juce::MidiMessage::noteOn(1,60,(juce::uint8)100),0);instance->processBlock(block,midi);if(block.getMagnitude(0,n)!=0||!midi.isEmpty())return 4;}
  juce::MemoryBlock state;instance->getStateInformation(state);instance->setStateInformation(state.getData(),(int)state.getSize());instance->releaseResources();
  std::cout<<"Instrument/MIDI/state OK "<<rate<<" Hz\n";
 }
 BrowserSession session;session.rate=48000;
 if(!session.start(juce::File(juce::String::fromUTF8(argv[2])),true)){std::cerr<<session.status();return 5;}
 for(int i=0;i<600&&!session.data()->streaming&&!session.data()->fault;++i)Sleep(50);
 if(!session.data()->streaming){std::cerr<<"Browser did not start audio: "<<session.status()<<" pid="<<session.pid()<<"\n";return 6;}
 const auto com=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
 // JUCE initialises STA; run the agile process-loopback activation on that thread too.
 auto cb=Make<Activation>();AUDIOCLIENT_ACTIVATION_PARAMS p{};p.ActivationType=AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;p.ProcessLoopbackParams.TargetProcessId=session.pid();p.ProcessLoopbackParams.ProcessLoopbackMode=PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;
 PROPVARIANT v{};v.vt=VT_BLOB;v.blob.cbSize=sizeof(p);v.blob.pBlobData=(BYTE*)&p;ComPtr<IActivateAudioInterfaceAsyncOperation> op;
 auto hr=ActivateAudioInterfaceAsync(VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK,__uuidof(IAudioClient),&v,cb.Get(),&op);
 if(FAILED(hr)||WaitForSingleObject(cb->done,10000)!=WAIT_OBJECT_0||FAILED(cb->result))return 7;
 WAVEFORMATEX f{};f.wFormatTag=WAVE_FORMAT_IEEE_FLOAT;f.nChannels=2;f.nSamplesPerSec=48000;f.wBitsPerSample=32;f.nBlockAlign=8;f.nAvgBytesPerSec=384000;
 hr=cb->client->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_LOOPBACK|AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,0,0,&f,nullptr);if(FAILED(hr))return 8;
 ComPtr<IAudioCaptureClient> capture;cb->client->GetService(IID_PPV_ARGS(&capture));cb->client->Start();
 juce::AudioBuffer<float> block(2,480);double bridgeEnergy=0,endpointEnergy=0;uint64_t count=0,endpointCount=0;
 for(int i=0;i<150;++i){Sleep(10);session.pull(block);for(int j=0;j<480;++j){const auto x=block.getSample(0,j);bridgeEnergy+=x*x;++count;}
  UINT32 n=0;while(SUCCEEDED(capture->GetNextPacketSize(&n))&&n){BYTE* bytes=nullptr;DWORD flags=0;capture->GetBuffer(&bytes,&n,&flags,nullptr,nullptr);if(!(flags&AUDCLNT_BUFFERFLAGS_SILENT))for(UINT32 j=0;j<n*2;++j){double x=((float*)bytes)[j];endpointEnergy+=x*x;}endpointCount+=n*2;capture->ReleaseBuffer(n);}}
 cb->client->Stop();const auto rms=std::sqrt(bridgeEnergy/count);const auto physical=std::sqrt(endpointEnergy/(endpointCount?endpointCount:1));
 std::cout<<"CEF bridge RMS="<<rms<<"; direct Windows audio RMS="<<physical<<"; packets="<<endpointCount<<"\n";
 std::wcout<<session.data()->error<<"\n";session.shutdown();if(SUCCEEDED(com))CoUninitialize();
 if(rms<0.03||physical>0.00001)return 9;
 std::cout<<"PASS: audio only through instrument bridge\n";return 0;
}
