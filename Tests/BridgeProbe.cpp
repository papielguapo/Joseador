#define NOMINMAX
#include "../Source/Bridge.h"
#include <audioclient.h>
#include <audioclientactivationparams.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <wrl/implements.h>
#include <iostream>
#include <cmath>
#include <string>
using namespace Microsoft::WRL;
struct Completion:RuntimeClass<RuntimeClassFlags<ClassicCom>,IActivateAudioInterfaceCompletionHandler,FtmBase>{
 HANDLE done=CreateEvent(0,1,0,0);HRESULT hr=E_PENDING;ComPtr<IAudioClient> client;
 ~Completion(){CloseHandle(done);}HRESULT STDMETHODCALLTYPE ActivateCompleted(IActivateAudioInterfaceAsyncOperation*op)override{ComPtr<IUnknown>o;HRESULT r;hr=op->GetActivateResult(&r,&o);if(SUCCEEDED(hr))hr=r;if(SUCCEEDED(hr))hr=o.As(&client);SetEvent(done);return S_OK;}};
int wmain(int argc,wchar_t**argv){
 if(argc<2)return 1;CoInitializeEx(nullptr,COINIT_MULTITHREADED);
 const auto name=L"Local\\WebCaptureNativeTest-"+std::to_wstring(GetCurrentProcessId());
 HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(webcapture::Bridge),name.c_str());
 auto*b=(webcapture::Bridge*)MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(webcapture::Bridge));new(b)webcapture::Bridge();
 HANDLE mutex=CreateMutexW(nullptr,FALSE,(name+L"-ui").c_str());
 const std::wstring folder=argv[1];auto command=L"\""+folder+L"\\MobileBrowser.exe\" --autoplay-policy=no-user-gesture-required --bridge="+name+L" --self-test";
 const bool persistence=argc>2&&std::wstring(argv[2]).rfind(L"--profile-test=",0)==0;
 const bool history=argc>2&&std::wstring(argv[2])==L"--history-test";
 const bool reload=argc>2&&std::wstring(argv[2])==L"--reload-test";
 const bool stress=argc>2&&std::wstring(argv[2])==L"--joseador-stress-test",crash=argc>2&&std::wstring(argv[2])==L"--joseador-recovery-test";
 const bool nativeInput=argc>2&&std::wstring(argv[2])==L"--joseador-native-input-test";
 if(persistence||history||reload||stress||crash||nativeInput)command+=L" "+std::wstring(argv[2]);else if(argc>2)command+=L" --home-test";
 if(argc>3)command+=L" --small";
 STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION proc{};
 if(!CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,folder.c_str(),&startup,&proc))return 2;
 int result=0;
 for(int i=0;i<600&&!b->streaming&&!b->fault;++i){Sleep(50);if(WaitForSingleObject(proc.hProcess,0)==WAIT_OBJECT_0)break;}
 if(!b->streaming){DWORD exit;GetExitCodeProcess(proc.hProcess,&exit);std::cout<<"Browser audio failed: process="<<exit<<" ready="<<b->ready<<" fault="<<b->fault<<"\n";result=3;}
 else{
  auto cb=Make<Completion>();AUDIOCLIENT_ACTIVATION_PARAMS p{};p.ActivationType=AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;p.ProcessLoopbackParams.TargetProcessId=proc.dwProcessId;p.ProcessLoopbackParams.ProcessLoopbackMode=PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;
  PROPVARIANT v{};v.vt=VT_BLOB;v.blob.cbSize=sizeof(p);v.blob.pBlobData=(BYTE*)&p;ComPtr<IActivateAudioInterfaceAsyncOperation>op;
  auto hr=ActivateAudioInterfaceAsync(VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK,__uuidof(IAudioClient),&v,cb.Get(),&op);
  if(FAILED(hr)||WaitForSingleObject(cb->done,10000)!=WAIT_OBJECT_0||FAILED(cb->hr))result=4;
  else{
   WAVEFORMATEX f{};f.wFormatTag=WAVE_FORMAT_IEEE_FLOAT;f.nChannels=2;f.nSamplesPerSec=48000;f.wBitsPerSample=32;f.nBlockAlign=8;f.nAvgBytesPerSec=384000;
   hr=cb->client->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_LOOPBACK|AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,0,0,&f,nullptr);
   if(FAILED(hr))result=5;
   else{ComPtr<IAudioCaptureClient>cap;cb->client->GetService(IID_PPV_ARGS(&cap));cb->client->Start();b->read=b->write.load();double e=0,direct=0;uint64_t count=0,dc=0;
    for(int i=0;i<(stress?4500:crash?700:nativeInput?420:220);++i){Sleep(10);auto r=b->read.load(),w=b->write.load();while(r!=w){const double x=b->audio[(r&(webcapture::capacity-1))*2];e+=x*x;++count;++r;}b->read=r;
     UINT32 n;while(SUCCEEDED(cap->GetNextPacketSize(&n))&&n){BYTE*data;DWORD flags;cap->GetBuffer(&data,&n,&flags,nullptr,nullptr);if(!(flags&AUDCLNT_BUFFERFLAGS_SILENT))for(UINT32 j=0;j<n*2;++j){const double x=((float*)data)[j];direct+=x*x;}dc+=n*2;cap->ReleaseBuffer(n);}}
    cb->client->Stop();const auto rms=std::sqrt(e/(count?count:1)),physical=std::sqrt(direct/(dc?dc:1));
    std::cout<<"Bridge RMS="<<rms<<" frames="<<count<<" rate="<<b->audioRate<<"; physical browser RMS="<<physical<<" frames="<<dc<<"\n";
    std::wcout<<b->error<<"\n";if(rms<0.03||physical>0.00001)result=6;
    if(argc>2&&!wcsstr(b->error,L"FIT:yes"))result=7;
    if(argc==2&&!wcsstr(b->error,L"TOUCH:yes"))result=8;
    if(persistence&&!wcsstr(b->error,L"PERSIST:yes"))result=9;
    if(history&&!wcsstr(b->error,L"HISTORY:yes"))result=10;
    if(reload&&!wcsstr(b->error,L"RELOAD:yes"))result=11;
    if(stress&&!wcsstr(b->error,L"STRESS:yes"))result=12;
    if(crash&&(!wcsstr(b->error,L"RECOVERY:yes")||b->fault.load()!=0))result=13;
    if(nativeInput&&!wcsstr(b->error,L"NATIVE_INPUT:yes"))result=14;
   }
  }
 }
 b->quit=1;WaitForSingleObject(proc.hProcess,6000);CloseHandle(proc.hThread);CloseHandle(proc.hProcess);CloseHandle(mutex);UnmapViewOfFile(b);CloseHandle(mapping);
 std::cout<<(result?"FAIL":"PASS")<<"\n";return result;
}
