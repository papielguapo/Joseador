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
