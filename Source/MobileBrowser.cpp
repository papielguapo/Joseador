#include "Bridge.h"
#include "include/capi/cef_app_capi.h"
#include "include/capi/cef_audio_handler_capi.h"
#include "include/capi/cef_client_capi.h"
#include "include/capi/cef_devtools_message_observer_capi.h"
#include "include/capi/cef_jsdialog_handler_capi.h"
#include "include/capi/cef_request_handler_capi.h"
#include "include/capi/cef_parser_capi.h"
#include "PasswordStore.h"
#include "NavigationPolicy.h"
#include "InputGate.h"
#include "ChatState.h"
#include "RecoveryPolicy.h"
#include "include/cef_sandbox_win.h"
#include "include/cef_api_hash.h"
#include <shellapi.h>
#include <string>
#include <fstream>
#include <cmath>

namespace {
void trace(const char*){}
template<class T>struct Ref {
    T api{};std::atomic<int> refs{1};
    Ref(){api.base.size=sizeof(T);api.base.add_ref=[](cef_base_ref_counted_t*b){++reinterpret_cast<Ref*>(b)->refs;};
        api.base.release=[](cef_base_ref_counted_t*b)->int{auto*r=reinterpret_cast<Ref*>(b);if(--r->refs==0){delete r;return 1;}return 0;};
        api.base.has_one_ref=[](cef_base_ref_counted_t*b)->int{return reinterpret_cast<Ref*>(b)->refs==1;};
        api.base.has_at_least_one_ref=[](cef_base_ref_counted_t*b)->int{return reinterpret_cast<Ref*>(b)->refs>0;};}
    T* retain(){api.base.add_ref(&api.base);return &api;}
    void release(){api.base.release(&api.base);}
};
template<class T>void release(T*p){if(p)p->base.release(&p->base);}
struct String {
    cef_string_t value{};
    explicit String(const std::wstring& s){cef_string_utf16_set((const char16_t*)s.data(),s.size(),&value,1);}
    ~String(){cef_string_utf16_clear(&value);}
};
webcapture::Bridge* shared=nullptr;
Ref<cef_life_span_handler_t>* lifeHandler=nullptr;
Ref<cef_audio_handler_t>* audioHandler=nullptr;
Ref<cef_load_handler_t>* loadHandler=nullptr;
Ref<cef_display_handler_t>* displayHandler=nullptr;
Ref<cef_jsdialog_handler_t>* dialogHandler=nullptr;
Ref<cef_request_handler_t>* requestHandler=nullptr;
RecoveryPolicy recovery;
ULONGLONG recoverAt=0;
HANDLE profileMutex=nullptr;
parttimegang::ReadState chatRead;
ULONGLONG testStarted=0;int testStep=0;
HANDLE navigationMutex=nullptr;
HWND root=nullptr;
cef_browser_t* active=nullptr;
bool closing=false,selfTest=false,homeTest=false;
bool stressTest=false,crashTest=false,crashIssued=false;
bool nativeInputTest=false;
bool returningBack=false;
std::wstring currentAddress;
bool historyTest=false,historyOk=true;int historyStep=-1;ULONGLONG historyStarted=0;
bool reloadTest=false,reloadIssued=false;int reloadLoads=0;ULONGLONG reloadStarted=0;
std::wstring profileTest;
int width=370,height=493,channelCount=2,protocolId=0,lastGestureId=0;
ULONGLONG reloadWaitStarted=0;
int wheelMode=0;HHOOK mouseHook=nullptr,keyboardHook=nullptr;InputGate inputGate;
bool isHome(const std::wstring& address);
struct SwipeState{bool active=false,tap=false;int direction=1;ULONGLONG started=0;double x=0,y=0,distance=0;bool horizontal=false;} swipe;
void protocol(const std::string& method,const std::string& params){
    if(!active)return;auto*host=active->get_host(active);
    auto json="{\"id\":"+std::to_string(++protocolId)+",\"method\":\""+method+"\",\"params\":"+params+"}";
    host->send_dev_tools_message(host,json.data(),json.size());release(host);
}
void mobileMetrics(){
    const double dpi=root?GetDpiForWindow(root)/96.0:1.0;
    const int cssWidth=(int)((width>0?width:370)/(dpi>0?dpi:1));
    const int cssHeight=(int)((height>0?height:493)/(dpi>0?dpi:1));
    protocol("Emulation.setDeviceMetricsOverride","{\"width\":"+std::to_string(cssWidth)+",\"height\":"+std::to_string(cssHeight)+",\"deviceScaleFactor\":"+std::to_string(dpi>0?dpi:1)+",\"mobile\":true}");
    protocol("Emulation.setTouchEmulationEnabled","{\"enabled\":true,\"maxTouchPoints\":1}");
}
void fitDocument(cef_frame_t*frame){
    if(isHome(currentAddress)){String state(L"window.dispatchEvent(new CustomEvent('ptg-state',{detail:{read:"+std::to_wstring(chatRead.lastRead)+L"}}));"),origin(parttimegang::origin);frame->execute_java_script(frame,&state.value,&origin.value,0);}
    String origin(L"https://webcapture.local");
    wchar_t file[32768];GetModuleFileNameW(nullptr,file,32768);std::wstring path=file;path=path.substr(0,path.find_last_of(L"\\/"))+L"\\mobile-input.js";
    std::ifstream stream(path);std::string input((std::istreambuf_iterator<char>(stream)),std::istreambuf_iterator<char>());
    // This controlled data document has no HTTP origin. Only the self-test
    // enables its scroll route; production script and scheme checks stay intact.
    if(nativeInputTest){const std::string guard="if(!/^https?:$/.test(location.protocol))return '';";const auto pos=input.find(guard);if(pos!=std::string::npos)input.erase(pos,guard.size());}
    {std::ifstream fitFile(path.substr(0,path.find_last_of(L"\\/"))+L"\\mobile-fit.js");std::string code((std::istreambuf_iterator<char>(fitFile)),{});if(!code.empty()){String fit(std::wstring(code.begin(),code.end()));frame->execute_java_script(frame,&fit.value,&origin.value,0);}}
    if(!input.empty()){String wheel(std::wstring(input.begin(),input.end()));frame->execute_java_script(frame,&wheel.value,&origin.value,0);}
    path=path.substr(0,path.find_last_of(L"\\/"))+L"\\passwords.js";
    std::ifstream credentialFile(path);std::string credentialScript((std::istreambuf_iterator<char>(credentialFile)),std::istreambuf_iterator<char>());
    if(!credentialScript.empty()){String credential(std::wstring(credentialScript.begin(),credentialScript.end()));frame->execute_java_script(frame,&credential.value,&origin.value,0);}
}
void touch(const char*type,double x,double y){
    const std::string points=(std::string(type)=="touchEnd"||std::string(type)=="touchCancel")?"[]":"[{\"x\":"+std::to_string(x)+",\"y\":"+std::to_string(y)+",\"id\":0,\"radiusX\":5,\"radiusY\":5,\"force\":1}]";
    protocol("Input.dispatchTouchEvent","{\"type\":\""+std::string(type)+"\",\"touchPoints\":"+points+"}");lastGestureId=protocolId;
}
void wheelGesture(bool stories,int direction,bool horizontal=false,bool gentle=false){
    static ULONGLONG last=0;const auto now=GetTickCount64();if(closing||swipe.active||now-last<(gentle?450:700))return;last=now;
    const double dpi=root?GetDpiForWindow(root)/96.0:1.0;
    const double w=width/(dpi>0?dpi:1),h=height/(dpi>0?dpi:1);
    const double startFraction=gentle?.72:.85,endFraction=gentle?.28:.15;
    swipe={true,stories,direction,now,w*((stories||horizontal)?(direction>0?startFraction:endFraction):.5),h*((stories||horizontal)?.5:(direction>0?startFraction:endFraction)),-direction*(horizontal?w:h)*(gentle?.44:.70),horizontal};
    touch("touchStart",swipe.x,swipe.y);
}
void updateSwipe(){if(!swipe.active)return;const double t=(GetTickCount64()-swipe.started)/(swipe.tap?70.0:260.0);
    const double distance=swipe.distance*(t>=1?1:(.5-.5*std::cos(t*3.141592653589793)));
    if(!swipe.tap)touch("touchMove",swipe.x+(swipe.horizontal?distance:0),swipe.y+(swipe.horizontal?0:distance));
    if(t>=1){touch("touchEnd",0,0);swipe.active=false;}
}
void cancelSwipe(bool send=true){if(swipe.active&&send)touch("touchCancel",0,0);swipe.active=false;inputGate.consume();wheelMode=0;}
LRESULT CALLBACK mouseProc(int code,WPARAM message,LPARAM param){
    if(code==HC_ACTION&&message==WM_MOUSEWHEEL&&active&&!(GetKeyState(VK_CONTROL)&0x8000)&&!(GetKeyState(VK_SHIFT)&0x8000)&&!(GetKeyState(VK_MENU)&0x8000)){
        auto*info=(MOUSEHOOKSTRUCTEX*)param;RECT bounds{};GetClientRect(root,&bounds);POINT point=info->pt;ScreenToClient(root,&point);
        if(PtInRect(&bounds,point)){static int amount=0;const int delta=(short)HIWORD(info->mouseData);if(delta)inputGate.record(0,delta<0?1:-1,GetTickCount64());
            // Mobile emulation does not reliably deliver DOM wheel events.
            // The renderer reports whether the current pointer is over a feed
            // or a nested editor/comment scroller; native input drives gestures.
            if(!wheelMode)return CallNextHookEx(mouseHook,code,message,param);
            if((amount>0)!=(delta>0))amount=0;amount+=delta;
            if(std::abs(amount)>=35){wheelGesture(wheelMode==2,amount<0?1:-1,false,wheelMode==3);amount=0;inputGate.consume();}return 1;}
    }
    return CallNextHookEx(mouseHook,code,message,param);
}
LRESULT CALLBACK keyboardProc(int code,WPARAM key,LPARAM details){
    if(code==HC_ACTION&&!(static_cast<ULONG_PTR>(details)&(1ull<<31))&&active){const auto focus=GetFocus();if(focus&&(focus==root||IsChild(root,focus))){
        if(key==VK_UP||key==VK_DOWN)inputGate.record(1,key==VK_DOWN?1:-1,GetTickCount64());
        else if(key==VK_LEFT||key==VK_RIGHT)inputGate.record(2,key==VK_RIGHT?1:-1,GetTickCount64());
    }}return CallNextHookEx(keyboardHook,code,key,details);
}
void navigate(const std::wstring& address){if(active&&navigation::allowed(address)){auto target=address;
    cancelSwipe();recoverAt=0;
    if(target==L"webcapture://home"){wchar_t path[32768];GetModuleFileNameW(nullptr,path,32768);target=std::wstring(path);target=target.substr(0,target.find_last_of(L"\\/"))+L"\\start.html";for(auto&c:target)if(c==L'\\')c=L'/';target=L"file:///"+target;}
    auto*f=active->get_main_frame(active);String s(target);f->load_url(f,&s.value);release(f);}}
bool isHome(const std::wstring& address){return address==L"webcapture://home"||(address.rfind(L"file:///",0)==0&&address.size()>=11&&address.substr(address.size()-11)==L"/start.html");}
void backOrHome(){
    if(!active)return;
    cancelSwipe();recoverAt=0;
    if(historyTest&&WaitForSingleObject(navigationMutex,100)==WAIT_OBJECT_0){const auto label=isHome(currentAddress)?L"B:home":active->can_go_back(active)?L"B:back":L"B:new";if(wcslen(shared->error)+12<std::size(shared->error))wcscat_s(shared->error,(std::wstring(L" | ")+label).c_str());ReleaseMutex(navigationMutex);}
    if(isHome(currentAddress))return;
    if(active->can_go_back(active)){returningBack=true;active->go_back(active);}else navigate(L"webcapture://home");
}
void closeBrowser(){cancelSwipe();recoverAt=0;if(active){auto*h=active->get_host(active);h->close_browser(h,1);release(h);}else cef_quit_message_loop();}
LRESULT CALLBACK windowProc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp){
    if(message==WM_SIZE){const int nextWidth=LOWORD(lp),nextHeight=HIWORD(lp);if(nextWidth==width&&nextHeight==height)return 0;cancelSwipe();width=nextWidth;height=nextHeight;if(active&&width>0&&height>0){auto*h=active->get_host(active);SetWindowPos(h->get_window_handle(h),nullptr,0,0,width,height,SWP_NOZORDER|SWP_NOACTIVATE);release(h);mobileMetrics();}return 0;}
    if(message==WM_TIMER){shared->heartbeat=GetTickCount64();
        updateSwipe();
        if(crashTest&&testStarted&&!crashIssued&&GetTickCount64()-testStarted>700){crashIssued=true;protocol("Page.crash","{}");}
        if(recoverAt&&active&&!closing&&GetTickCount64()>=recoverAt){recoverAt=0;active->reload(active);}
        if(reloadTest&&reloadLoads==1&&!reloadIssued&&GetTickCount64()-reloadStarted>500&&WaitForSingleObject(navigationMutex,0)==WAIT_OBJECT_0){shared->navigation.push(6);reloadIssued=true;ReleaseMutex(navigationMutex);}
        if(historyTest&&historyStep>=0&&historyStep<4&&GetTickCount64()-historyStarted>(ULONGLONG)(400*(historyStep+1))){
            const std::wstring current=shared->url;
            if(WaitForSingleObject(navigationMutex,100)==WAIT_OBJECT_0){const auto hash=current.find(L'#');const auto label=isHome(current)?L"home":hash==std::wstring::npos?L"other":current.substr(hash);const auto marker=L" | H"+std::to_wstring(historyStep)+L":"+label;if(wcslen(shared->error)+marker.size()+1<std::size(shared->error))wcscat_s(shared->error,marker.c_str());ReleaseMutex(navigationMutex);}
            historyOk=historyOk&&(historyStep==0?current.find(L"#two")!=std::wstring::npos:historyStep==1?current.find(L"#one")!=std::wstring::npos:isHome(current));
            if(historyStep<3)backOrHome();
            else if(WaitForSingleObject(navigationMutex,100)==WAIT_OBJECT_0){wcscat_s(shared->error,historyOk?L" | HISTORY:yes":L" | HISTORY:no");ReleaseMutex(navigationMutex);}
            ++historyStep;
        }
        if(selfTest&&!homeTest&&!crashTest&&testStarted){const auto elapsed=GetTickCount64()-testStarted;
            if(nativeInputTest){if(testStep<4&&elapsed>250+testStep*900){const int direction=testStep%2?-1:1;if(testStep<2){MOUSEHOOKSTRUCTEX event{};event.pt={width/2,height/2};ClientToScreen(root,&event.pt);event.mouseData=MAKEWPARAM(0,direction>0?-120:120);mouseProc(HC_ACTION,WM_MOUSEWHEEL,(LPARAM)&event);}else{inputGate.record(1,direction,GetTickCount64());const std::string key=direction>0?"ArrowDown":"ArrowUp";protocol("Input.dispatchKeyEvent","{\"type\":\"keyDown\",\"key\":\""+key+"\",\"code\":\""+key+"\",\"windowsVirtualKeyCode\":"+std::to_string(direction>0?40:38)+"}");protocol("Input.dispatchKeyEvent","{\"type\":\"keyUp\",\"key\":\""+key+"\",\"code\":\""+key+"\"}");}++testStep;}}
            else if(stressTest){if(testStep<60&&elapsed>250+testStep*720){wheelGesture(false,testStep%2?-1:1);++testStep;}}
            else if(testStep==0&&elapsed>250){wheelGesture(false,1);++testStep;}
            else if(testStep==1&&elapsed>1150){wheelGesture(false,-1);++testStep;}}
        if(shared->quit&&!closing){closing=true;closeBrowser();}
        if(active&&!closing&&WaitForSingleObject(navigationMutex,0)==WAIT_OBJECT_0){NavigationRequest request;const bool loadingReload=shared->navigation.nextAction()==6&&active->is_loading(active);if(loadingReload&&!reloadWaitStarted)reloadWaitStarted=GetTickCount64();if(!loadingReload||GetTickCount64()-reloadWaitStarted>=500){shared->navigation.pop(request);reloadWaitStarted=0;}const auto action=request.action;std::wstring address=request.address;ReleaseMutex(navigationMutex);
            if(action==1){returningBack=false;navigate(address);}else if(action==2)backOrHome();else if(action==3){cancelSwipe();recoverAt=0;active->go_forward(active);}
            else if(action==6){cancelSwipe();recoverAt=0;returningBack=false;active->reload(active);}
            else if(action==4||action==5){if(wheelMode){wheelGesture(wheelMode==2,action==4?1:-1,false,wheelMode==3);}else{auto*host=active->get_host(active);cef_mouse_event_t event{};event.x=width/2;event.y=height/2;host->send_mouse_wheel_event(host,&event,0,action==4?-120:120);release(host);}}}
        return 0;}
    if(message==WM_CLOSE){if(closing)DestroyWindow(hwnd);else{closing=true;closeBrowser();}return 0;}
    return DefWindowProc(hwnd,message,wp,lp);
}
int run(HINSTANCE instance,void* sandbox){
    trace("entry\n");cef_api_hash(CEF_API_VERSION,0);trace("api\n");cef_main_args_t args{};args.instance=instance;
    int subprocess=cef_execute_process(&args,nullptr,sandbox);if(subprocess>=0)return subprocess;trace("browser process\n");
    int count=0;auto argv=CommandLineToArgvW(GetCommandLineW(),&count);std::wstring name;
    for(int i=1;i<count;++i){std::wstring a=argv[i];if(a.rfind(L"--bridge=",0)==0)name=a.substr(9);if(a==L"--self-test")selfTest=true;if(a==L"--home-test")homeTest=true;if(a==L"--history-test"){historyTest=true;homeTest=true;}if(a.rfind(L"--profile-test=",0)==0)profileTest=a.substr(15);if(a==L"--small"){width=320;height=280;}}LocalFree(argv);
    HANDLE mapping=OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,name.c_str());if(!mapping)return 31;
    shared=(webcapture::Bridge*)MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(webcapture::Bridge));if(!shared)return 32;
    if(shared->signature!=0x4a4f5332||shared->version!=2)return 38;
    navigationMutex=OpenMutexW(SYNCHRONIZE|MUTEX_MODIFY_STATE,FALSE,(name+L"-ui").c_str());if(!navigationMutex)return 33;
    reloadTest=std::wstring(GetCommandLineW()).find(L"--reload-test")!=std::wstring::npos;if(reloadTest)homeTest=true;
    stressTest=selfTest&&std::wstring(GetCommandLineW()).find(L"--joseador-stress-test")!=std::wstring::npos;crashTest=selfTest&&std::wstring(GetCommandLineW()).find(L"--joseador-recovery-test")!=std::wstring::npos;
    nativeInputTest=selfTest&&std::wstring(GetCommandLineW()).find(L"--joseador-native-input-test")!=std::wstring::npos;
    if(!sandbox){shared->fault=1;return 34;}
    cef_settings_t settings{};settings.size=sizeof(settings);settings.log_severity=LOGSEVERITY_DISABLE;
    wchar_t local[MAX_PATH];GetEnvironmentVariableW(L"LOCALAPPDATA",local,MAX_PATH);
    auto cache=std::wstring(local)+L"\\WebCaptureMobile\\"+name.substr(name.find_last_of(L'\\')+1);
    if(selfTest&&!profileTest.empty())cache=std::wstring(local)+L"\\WebCaptureMobile\\Tests\\Persistence";
    if(!selfTest){for(int slot=1;slot<=128;++slot){
        auto key=L"Local\\WebCaptureMobileProfileSlot"+std::to_wstring(slot);auto mutex=CreateMutexW(nullptr,FALSE,key.c_str());if(!mutex)continue;
        const auto acquired=WaitForSingleObject(mutex,0);if(acquired==WAIT_OBJECT_0||acquired==WAIT_ABANDONED){profileMutex=mutex;cache=std::wstring(local)+L"\\WebCaptureMobile\\Profiles\\Slot"+std::to_wstring(slot);break;}CloseHandle(mutex);
    }if(!profileMutex){shared->fault=1;return 37;}}
    chatRead.open(cache);
    String rootCache(cache),profile(cache+L"\\Profile"),agent(L"Mozilla/5.0 (Linux; Android 13; Pixel 7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/150.0.0.0 Mobile Safari/537.36");
    settings.root_cache_path=rootCache.value;settings.cache_path=profile.value;settings.user_agent=agent.value;
    settings.persist_session_cookies=1;
    trace("initialise\n");if(!cef_initialize(&args,&settings,nullptr,sandbox)){shared->fault=1;return 35;}trace("initialised\n");
    WNDCLASSW wc{};wc.lpfnWndProc=windowProc;wc.hInstance=instance;wc.lpszClassName=L"WebCaptureMobileBrowser";RegisterClassW(&wc);
    root=CreateWindowExW(0,wc.lpszClassName,L"JOSEADOR",WS_POPUP|WS_CLIPCHILDREN,0,0,width,height,nullptr,nullptr,instance,nullptr);
    auto*life=new Ref<cef_life_span_handler_t>;
    life->api.on_after_created=[](cef_life_span_handler_t*,cef_browser_t*b){trace("after created\n");active=b;b->base.add_ref(&b->base);shared->ready=1;};
    life->api.on_before_close=[](cef_life_span_handler_t*,cef_browser_t*){release(active);active=nullptr;shared->window=0;cef_quit_message_loop();};
    life->api.on_before_popup=[](cef_life_span_handler_t*,cef_browser_t*,cef_frame_t*,int,const cef_string_t* url,const cef_string_t*,cef_window_open_disposition_t,int,const cef_popup_features_t*,cef_window_info_t*,cef_client_t**,cef_browser_settings_t*,cef_dictionary_value_t**,int*)->int{navigate(std::wstring((wchar_t*)url->str,url->length));return 1;};
    auto*audio=new Ref<cef_audio_handler_t>;
    audio->api.get_audio_parameters=[](cef_audio_handler_t*,cef_browser_t*,cef_audio_parameters_t*p)->int{p->sample_rate=(int)shared->sampleRate.load();p->channel_layout=CEF_CHANNEL_LAYOUT_STEREO;p->frames_per_buffer=480;return 1;};
    audio->api.on_audio_stream_started=[](cef_audio_handler_t*,cef_browser_t*,const cef_audio_parameters_t*p,int channels){channelCount=channels;shared->audioRate=p->sample_rate;uint32_t expected=1;shared->fault.compare_exchange_strong(expected,0);shared->streaming=1;};
    audio->api.on_audio_stream_packet=[](cef_audio_handler_t*,cef_browser_t*,const float**data,int frames,int64_t){webcapture::push(*shared,data,frames,channelCount);};
    audio->api.on_audio_stream_stopped=[](cef_audio_handler_t*,cef_browser_t*){shared->streaming=0;shared->peak=0;};
    audio->api.on_audio_stream_error=[](cef_audio_handler_t*,cef_browser_t*,const cef_string_t*){shared->fault=1;};
    auto*load=new Ref<cef_load_handler_t>;
    load->api.on_load_start=[](cef_load_handler_t*,cef_browser_t*,cef_frame_t*frame,cef_transition_type_t){if(frame->is_main(frame)){cancelSwipe();recoverAt=0;}};
    load->api.on_load_error=[](cef_load_handler_t*,cef_browser_t*,cef_frame_t*frame,cef_errorcode_t code,const cef_string_t*,const cef_string_t*){if(!frame->is_main(frame)||code==ERR_ABORTED)return;shared->fault=3;cancelSwipe();if(WaitForSingleObject(navigationMutex,0)==WAIT_OBJECT_0){const auto text=L"Error al cargar la web ("+std::to_wstring(code)+L"). Pulsa Actualizar.";wcsncpy_s(shared->error,text.c_str(),_TRUNCATE);ReleaseMutex(navigationMutex);}};
    load->api.on_load_end=[](cef_load_handler_t*,cef_browser_t*,cef_frame_t*frame,int httpStatus){if(frame->is_main(frame)){if((httpStatus>=200&&httpStatus<400||isHome(currentAddress)||(selfTest&&httpStatus==0))&&(shared->fault==2||shared->fault==3)){shared->fault=0;if(WaitForSingleObject(navigationMutex,0)==WAIT_OBJECT_0){shared->error[0]=0;ReleaseMutex(navigationMutex);}}mobileMetrics();fitDocument(frame);if(selfTest){
        String js(L"window.a=new AudioContext();window.o=a.createOscillator();window.g=a.createGain();g.gain.value=0.18;o.frequency.value=440;o.connect(g);g.connect(a.destination);o.start();a.resume();console.log('WEBCAPTURE_CODECS:'+document.createElement('video').canPlayType('video/mp4; codecs=\"avc1.42E01E, mp4a.40.2\"')+' | APPS:'+document.querySelectorAll('main a').length+' | WIDTH:'+innerWidth);"),origin(L"https://webcapture.test");
        frame->execute_java_script(frame,&js.value,&origin.value,0);
        if(reloadTest&&isHome(currentAddress)){++reloadLoads;if(reloadLoads==1)reloadStarted=GetTickCount64();else if(reloadLoads==2){String result(L"console.debug('WEBCAPTURE_RELOAD:yes');");frame->execute_java_script(frame,&result.value,&origin.value,0);}}
        if(historyTest&&historyStep<0){auto*u=frame->get_url(frame);std::wstring page;if(u){page.assign((wchar_t*)u->str,u->length);cef_string_userfree_utf16_free(u);}if(isHome(page)){
            historyStep=0;historyStarted=GetTickCount64();protocol("Runtime.evaluate","{\"expression\":\"history.pushState({},'' , '#one');history.pushState({},'' , '#two');\",\"userGesture\":true}");
        }}
        if(!profileTest.empty()){
            String persistence(profileTest==L"write"?L"localStorage.setItem('wc_test','ok');document.cookie='wc_test=ok;path=/;SameSite=Lax';console.debug('WEBCAPTURE_PERSIST:'+((localStorage.getItem('wc_test')==='ok'&&document.cookie.includes('wc_test=ok'))?'yes':'no'));":L"console.debug('WEBCAPTURE_PERSIST:'+((localStorage.getItem('wc_test')==='ok'&&document.cookie.includes('wc_test=ok'))?'yes':'no'));");
            frame->execute_java_script(frame,&persistence.value,&origin.value,0);
        }
        String check(L"setTimeout(()=>console.debug('WEBCAPTURE_FIT:'+((document.documentElement.scrollHeight<=innerHeight+2&&document.documentElement.scrollWidth<=innerWidth+2)?'yes':'no')),300)");frame->execute_java_script(frame,&check.value,&origin.value,0);
        if(nativeInputTest){String fixture(L"document.body.style='margin:0;height:100vh;touch-action:none';let starts=[],moves=0,trusted=true;addEventListener('touchstart',e=>{starts.push([e.touches[0].clientY,0]);trusted=trusted&&e.isTrusted;},{passive:false});addEventListener('touchmove',e=>{e.preventDefault();moves++;starts[starts.length-1][1]=e.touches[0].clientY;trusted=trusted&&e.isTrusted;},{passive:false});setTimeout(()=>console.debug('WEBCAPTURE_NATIVE_INPUT:'+((trusted&&moves>8&&starts.length===4&&starts.every((s,i)=>i%2?s[1]>s[0]+30:s[1]<s[0]-30))?'yes':'no')),3550);");frame->execute_java_script(frame,&fixture.value,&origin.value,0);testStarted=GetTickCount64();}
        else if(stressTest){String fixture(L"document.body.style='margin:0;height:100vh;touch-action:none';let starts=0,ends=0,moves=0,active=0,valid=true;addEventListener('touchstart',e=>{starts++;active++;valid=valid&&e.isTrusted&&active===1;},{passive:false});addEventListener('touchmove',e=>{e.preventDefault();moves++;valid=valid&&e.isTrusted;},{passive:false});addEventListener('touchend',e=>{ends++;active--;valid=valid&&e.isTrusted;const n=document.createElement('div');n.textContent='Reel '+ends;document.body.replaceChildren(n);},{passive:false});setTimeout(()=>console.debug('WEBCAPTURE_STRESS:'+((valid&&starts===60&&ends===60&&moves>300&&active===0)?'yes':'no')),44200);");frame->execute_java_script(frame,&fixture.value,&origin.value,0);testStarted=GetTickCount64();}
        else if(!homeTest&&!crashTest){String fixture(L"document.body.style='margin:0;height:100vh;touch-action:none';let starts=[],moves=0,trusted=true;addEventListener('touchstart',e=>{starts.push([e.touches[0].clientY,0]);trusted=trusted&&e.isTrusted;},{passive:false});addEventListener('touchmove',e=>{e.preventDefault();moves++;starts[starts.length-1][1]=e.touches[0].clientY;trusted=trusted&&e.isTrusted;},{passive:false});setTimeout(()=>console.debug('WEBCAPTURE_TOUCH:'+((trusted&&moves>4&&starts.length===2&&starts[0][1]<starts[0][0]-30&&starts[1][1]>starts[1][0]+30)?'yes':'no')),1600);");frame->execute_java_script(frame,&fixture.value,&origin.value,0);testStarted=GetTickCount64();}
        if(crashTest&&!crashIssued)testStarted=GetTickCount64();
        if(crashTest&&crashIssued){String result(L"console.debug('WEBCAPTURE_RECOVERY:yes')");frame->execute_java_script(frame,&result.value,&origin.value,0);}
        }}};
    auto*display=new Ref<cef_display_handler_t>;
    auto*request=new Ref<cef_request_handler_t>;
    request->api.on_render_process_terminated=[](cef_request_handler_t*,cef_browser_t*,cef_termination_status_t status,int code,const cef_string_t*){if(closing)return;cancelSwipe(false);shared->streaming=0;shared->fault=2;const auto now=GetTickCount64();const bool retry=recovery.allow(now);recoverAt=retry?now+500:0;if(WaitForSingleObject(navigationMutex,0)==WAIT_OBJECT_0){const auto text=L"La web se cerro ("+std::to_wstring(status)+L"/"+std::to_wstring(code)+(retry?L"). Recuperando...":L"). Pulsa Actualizar.");wcsncpy_s(shared->error,text.c_str(),_TRUNCATE);ReleaseMutex(navigationMutex);}};
    display->api.on_address_change=[](cef_display_handler_t*,cef_browser_t*,cef_frame_t*frame,const cef_string_t* url){
        if(!frame->is_main(frame))return;std::wstring address((wchar_t*)url->str,url->length);currentAddress=address;
        const bool blankBack=returningBack&&address==L"about:blank";returningBack=false;
        if(blankBack){navigate(L"webcapture://home");return;}
        if(isHome(address))address=L"webcapture://home";
        if(WaitForSingleObject(navigationMutex,0)==WAIT_OBJECT_0){wcsncpy_s(shared->url,address.c_str(),_TRUNCATE);ReleaseMutex(navigationMutex);}
    };
    display->api.on_console_message=[](cef_display_handler_t*,cef_browser_t*,cef_log_severity_t,const cef_string_t*msg,const cef_string_t*,int)->int{
        std::wstring command((wchar_t*)msg->str,msg->length);
        if(command==L"__WC_MEDIA__:ok"){uint32_t expected=4;shared->fault.compare_exchange_strong(expected,0);return 1;}
        if(command.size()==14&&command.rfind(L"__WC_MEDIA__:",0)==0&&command.back()>=L'1'&&command.back()<=L'4'){const auto fault=shared->fault.load();if(fault&&fault!=4)return 1;shared->fault=4;if(WaitForSingleObject(navigationMutex,0)==WAIT_OBJECT_0){const auto text=L"Error de video "+command.substr(13)+L" (1: cancelado, 2: red, 3: decodificacion, 4: formato). Pulsa Actualizar.";wcsncpy_s(shared->error,text.c_str(),_TRUNCATE);ReleaseMutex(navigationMutex);}return 1;}
        if(command.rfind(L"__PTG_READ__:",0)==0){chatRead.accept(passwords::origin(currentAddress),command,root&&IsWindowVisible(root)&&!IsIconic(GetAncestor(root,GA_ROOT)));return 1;}
        if(command==L"__WC_MODE__:reels"||command==L"__WC_MODE__:stories"||command==L"__WC_MODE__:scroll"||command==L"__WC_MODE__:normal"){wheelMode=command==L"__WC_MODE__:reels"?1:(command==L"__WC_MODE__:stories"?2:(command==L"__WC_MODE__:scroll"?3:0));return 1;}
        if(selfTest&&(command.rfind(L"WEBCAPTURE_NATIVE_INPUT:",0)==0||command.rfind(L"WEBCAPTURE_STRESS:",0)==0||command.rfind(L"WEBCAPTURE_RECOVERY:",0)==0||command.rfind(L"WEBCAPTURE_FIT:",0)==0||command.rfind(L"WEBCAPTURE_TOUCH:",0)==0||command.rfind(L"WEBCAPTURE_PERSIST:",0)==0||command.rfind(L"WEBCAPTURE_RELOAD:",0)==0)&&WaitForSingleObject(navigationMutex,100)==WAIT_OBJECT_0){if(wcslen(shared->error)+command.size()+4<std::size(shared->error))wcscat_s(shared->error,(L" | "+command).c_str());ReleaseMutex(navigationMutex);return 1;}
        if(command==L"__WC_WHEEL__:reels:1"||command==L"__WC_WHEEL__:reels:-1"||command==L"__WC_WHEEL__:stories:1"||command==L"__WC_WHEEL__:stories:-1"||command==L"__WC_WHEEL__:scroll:1"||command==L"__WC_WHEEL__:scroll:-1"||command==L"__WC_WHEEL__:horizontal:1"||command==L"__WC_WHEEL__:horizontal:-1"){
            const bool stories=command.find(L":stories:")!=std::wstring::npos,horizontal=command.find(L":horizontal:")!=std::wstring::npos;const int direction=command[command.size()-2]==L'-'?-1:1;
            if(inputGate.accept(horizontal,stories,direction,GetTickCount64()))wheelGesture(stories,direction,horizontal,command.find(L":scroll:")!=std::wstring::npos);return 1;
        }
        if(selfTest){std::wstring text((wchar_t*)msg->str,msg->length);if(text.rfind(L"WEBCAPTURE_CODECS:",0)==0&&WaitForSingleObject(navigationMutex,100)==WAIT_OBJECT_0){wcsncpy_s(shared->error,text.c_str(),_TRUNCATE);ReleaseMutex(navigationMutex);}}return 0;};
    auto*dialog=new Ref<cef_jsdialog_handler_t>;
    dialog->api.on_jsdialog=[](cef_jsdialog_handler_t*,cef_browser_t*,const cef_string_t* url,cef_jsdialog_type_t type,const cef_string_t* message,const cef_string_t* prompt,cef_jsdialog_callback_t*callback,int*)->int{
        const std::wstring text((wchar_t*)message->str,message->length);
        if(type!=JSDIALOGTYPE_PROMPT||(text!=L"__WC_PASSWORD_LOAD__"&&text!=L"__WC_PASSWORD_SAVE__"))return 0;
        const std::wstring dialogOrigin((wchar_t*)url->str,url->length);
        if(!passwords::sameTopOrigin(dialogOrigin,currentAddress)||!prompt||prompt->length>16384){String empty(L"{}");callback->cont(callback,1,&empty.value);return 1;}
        const auto site=passwords::origin(std::wstring((wchar_t*)url->str,url->length));std::wstring reply=L"{}",user,secret;
        if(!site.empty()&&text==L"__WC_PASSWORD_LOAD__"){
            if(passwords::read(site,user,secret))reply=L"{\"user\":"+passwords::json(user)+L",\"password\":"+passwords::json(secret)+L"}";
        }else if(!site.empty()){
            auto*value=cef_parse_json(prompt,JSON_PARSER_RFC);
            if(value){auto*dict=value->get_type(value)==VTYPE_DICTIONARY?value->get_dictionary(value):nullptr;if(dict){String u(L"user"),p(L"password");auto a=dict->get_string(dict,&u.value),b=dict->get_string(dict,&p.value);
                if(a){user.assign((wchar_t*)a->str,a->length);cef_string_userfree_utf16_free(a);}if(b){secret.assign((wchar_t*)b->str,b->length);if(b->str)SecureZeroMemory(b->str,b->length*sizeof(char16_t));cef_string_userfree_utf16_free(b);}release(dict);}release(value);}
            reply=L"cancel";
            if(!secret.empty()){
                std::wstring oldUser,oldSecret;const bool same=passwords::read(site,oldUser,oldSecret)&&oldUser==user&&oldSecret==secret;passwords::wipe(oldSecret);
                if(same)reply=L"saved";
                else if(MessageBoxW(root,(L"Guardar la contrasena de "+site+L" en el Administrador de credenciales de Windows?\nSe usara al volver a iniciar sesion en esta web.").c_str(),L"JOSEADOR - Guardar contrasena",MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)==IDYES){
                    if(passwords::save(site,user,secret))reply=L"saved";else{reply=L"retry";MessageBoxW(root,L"Windows no ha podido guardar la contrasena.",L"JOSEADOR",MB_OK|MB_ICONINFORMATION);}}
            }
        }
        {String response(reply);callback->cont(callback,1,&response.value);if(response.value.str)SecureZeroMemory(response.value.str,response.value.length*sizeof(char16_t));}
        passwords::wipe(secret);passwords::wipe(reply);return 1;
    };
    // The client is owned here until CEF shuts down. Its handler getters return retained references.
    auto*client=new Ref<cef_client_t>;lifeHandler=life;audioHandler=audio;loadHandler=load;displayHandler=display;requestHandler=request;
    client->api.get_request_handler=[](cef_client_t*){return requestHandler->retain();};
    client->api.get_life_span_handler=[](cef_client_t*){trace("life handler\n");return lifeHandler->retain();};
    client->api.get_audio_handler=[](cef_client_t*){trace("audio handler\n");return audioHandler->retain();};
    client->api.get_load_handler=[](cef_client_t*){trace("load handler\n");return loadHandler->retain();};
    client->api.get_display_handler=[](cef_client_t*){trace("display handler\n");return displayHandler->retain();};
    dialogHandler=dialog;client->api.get_jsdialog_handler=[](cef_client_t*){return dialogHandler->retain();};
    cef_window_info_t info{};info.size=sizeof(info);info.parent_window=root;info.style=WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;info.bounds={0,0,width,height};info.runtime_style=CEF_RUNTIME_STYLE_ALLOY;
    cef_browser_settings_t browserSettings{};browserSettings.size=sizeof(browserSettings);browserSettings.background_color=0xff10151e;
    String first(selfTest?(!profileTest.empty()?L"http://127.0.0.1:18947/":L"data:text/html,<html><body>Audio bridge test</body></html>"):L"about:blank");
    trace("create browser\n");auto*created=cef_browser_host_create_browser_sync(&info,&client->api,&first.value,&browserSettings,nullptr,nullptr);trace("created browser\n");
    if(!created){shared->fault=1;cef_shutdown();return 36;}release(created);
    auto*observer=new Ref<cef_dev_tools_message_observer_t>;
    observer->api.on_dev_tools_method_result=[](cef_dev_tools_message_observer_t*,cef_browser_t*,int,int,const void*,size_t){};
    auto*host=active->get_host(active);auto*registration=host->add_dev_tools_message_observer(host,&observer->api);release(host);
    if(homeTest)navigate(L"webcapture://home");
    shared->window=(uint64_t)(uintptr_t)root;mouseHook=SetWindowsHookExW(WH_MOUSE,mouseProc,nullptr,GetCurrentThreadId());keyboardHook=SetWindowsHookExW(WH_KEYBOARD,keyboardProc,nullptr,GetCurrentThreadId());SetTimer(root,1,16,nullptr);cef_run_message_loop();if(mouseHook)UnhookWindowsHookEx(mouseHook);if(keyboardHook)UnhookWindowsHookEx(keyboardHook);KillTimer(root,1);DestroyWindow(root);
    release(registration);observer->release();cef_shutdown();client->release();life->release();audio->release();load->release();display->release();dialog->release();request->release();if(profileMutex){ReleaseMutex(profileMutex);CloseHandle(profileMutex);}CloseHandle(navigationMutex);UnmapViewOfFile(shared);CloseHandle(mapping);return 0;
}
}
extern "C" __declspec(dllexport) int RunWinMain(HINSTANCE instance,LPTSTR,int,void*sandbox,cef_version_info_t*){return run(instance,sandbox);}
