#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "InstallCore.h"
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <wintrust.h>
#include <softpub.h>
#include <wincrypt.h>
#include <thread>
#include <string>
#include <memory>
#include <versionhelpers.h>
namespace {
HWND mainWindow,statusText,progressBar,installButton,cancelButton;bool busy=false;HFONT font,titleFont;
std::filesystem::path sourceRoot,pluginRoot,backupRoot;
constexpr UINT progressMessage=WM_APP+1,resultMessage=WM_APP+2;
struct Result{bool ok;std::wstring text;};
std::wstring widen(const std::string& text){const int count=MultiByteToWideChar(CP_UTF8,0,text.data(),(int)text.size(),nullptr,0);std::wstring value(count,0);MultiByteToWideChar(CP_UTF8,0,text.data(),(int)text.size(),value.data(),count);return value;}
bool runtimeReady(){DWORD installed=0,major=0,minor=0,build=0;constexpr auto key=L"SOFTWARE\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64";const auto get=[&](const wchar_t*name,DWORD&value){DWORD size=sizeof(value);return RegGetValueW(HKEY_LOCAL_MACHINE,key,name,RRF_RT_REG_DWORD|RRF_SUBKEY_WOW6464KEY,nullptr,&value,&size)==ERROR_SUCCESS;};return get(L"Installed",installed)&&installed&&get(L"Major",major)&&get(L"Minor",minor)&&get(L"Bld",build)&&(major>14||(major==14&&(minor>51||(minor==51&&build>=36231))));}
bool microsoftSigned(const std::filesystem::path& file){
 WINTRUST_FILE_INFO info{};info.cbStruct=sizeof(info);info.pcwszFilePath=file.c_str();WINTRUST_DATA trust{};trust.cbStruct=sizeof(trust);trust.dwUIChoice=WTD_UI_NONE;trust.fdwRevocationChecks=WTD_REVOKE_NONE;trust.dwUnionChoice=WTD_CHOICE_FILE;trust.pFile=&info;trust.dwStateAction=WTD_STATEACTION_VERIFY;trust.dwProvFlags=WTD_CACHE_ONLY_URL_RETRIEVAL;GUID policy=WINTRUST_ACTION_GENERIC_VERIFY_V2;
 const auto verified=WinVerifyTrust(nullptr,&policy,&trust);trust.dwStateAction=WTD_STATEACTION_CLOSE;WinVerifyTrust(nullptr,&policy,&trust);if(verified!=ERROR_SUCCESS)return false;
 HCERTSTORE store=nullptr;HCRYPTMSG message=nullptr;DWORD encoding=0,content=0,format=0;bool valid=false;
 if(CryptQueryObject(CERT_QUERY_OBJECT_FILE,file.c_str(),CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,CERT_QUERY_FORMAT_FLAG_BINARY,0,&encoding,&content,&format,&store,&message,nullptr)){
  DWORD size=0;if(CryptMsgGetParam(message,CMSG_SIGNER_INFO_PARAM,0,nullptr,&size)){std::vector<BYTE> buffer(size);if(CryptMsgGetParam(message,CMSG_SIGNER_INFO_PARAM,0,buffer.data(),&size)){const auto*signer=(CMSG_SIGNER_INFO*)buffer.data();CERT_INFO cert{};cert.Issuer=signer->Issuer;cert.SerialNumber=signer->SerialNumber;auto context=CertFindCertificateInStore(store,encoding,0,CERT_FIND_SUBJECT_CERT,&cert,nullptr);if(context){wchar_t name[256]{};CertGetNameStringW(context,CERT_NAME_SIMPLE_DISPLAY_TYPE,0,nullptr,name,256);valid=std::wstring(name)==L"Microsoft Corporation";CertFreeCertificateContext(context);}}}
 }if(message)CryptMsgClose(message);if(store)CertCloseStore(store,0);return valid;
}
void requireRuntime(){
 if(runtimeReady())return;const auto installer=sourceRoot/L"Prerequisites/VC_redist.x64.exe";
 if(!std::filesystem::exists(installer)||!microsoftSigned(installer))throw std::runtime_error("No se encontro el runtime oficial de Microsoft con una firma valida. Extrae de nuevo el ZIP completo.");
 SHELLEXECUTEINFOW launch{};launch.cbSize=sizeof(launch);launch.fMask=SEE_MASK_NOCLOSEPROCESS;launch.hwnd=mainWindow;launch.lpVerb=L"open";launch.lpFile=installer.c_str();launch.lpParameters=L"/install /norestart";launch.nShow=SW_SHOWNORMAL;
 if(!ShellExecuteExW(&launch)||!launch.hProcess)throw std::runtime_error("No se pudo abrir el instalador de Visual C++. JOSEADOR no se ha modificado.");
 WaitForSingleObject(launch.hProcess,INFINITE);DWORD exit=0;GetExitCodeProcess(launch.hProcess,&exit);CloseHandle(launch.hProcess);
 if(!runtimeReady())throw std::runtime_error("Visual C++ no se ha instalado o actualizado. Completa su instalador antes de instalar JOSEADOR.");
}
HWND control(const wchar_t*kind,const wchar_t*text,DWORD style,int x,int y,int w,int h,int id=0){auto item=CreateWindowW(kind,text,WS_CHILD|WS_VISIBLE|style,x,y,w,h,mainWindow,(HMENU)(INT_PTR)id,GetModuleHandleW(nullptr),nullptr);SendMessageW(item,WM_SETFONT,(WPARAM)font,TRUE);return item;}
void begin(){
 busy=true;EnableWindow(installButton,FALSE);EnableWindow(cancelButton,FALSE);EnableMenuItem(GetSystemMenu(mainWindow,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_GRAYED);SetWindowTextW(statusText,L"Comprobando paquete y Visual C++...");
 std::thread([]{auto result=std::make_unique<Result>();try{setup::validate(sourceRoot/L"JOSEADOR.vst3");requireRuntime();const auto path=setup::install(sourceRoot/L"JOSEADOR.vst3",pluginRoot,backupRoot,[](unsigned value){PostMessageW(mainWindow,progressMessage,value,0);});result->ok=true;result->text=L"JOSEADOR instalado.\n\nAbre FL Studio, busca plugins instalados y carga JOSEADOR como instrumento.\n\nDestino: "+path.wstring();}catch(const std::exception&e){result->ok=false;result->text=widen(e.what());}PostMessageW(mainWindow,resultMessage,0,(LPARAM)result.release());}).detach();
}
LRESULT CALLBACK proc(HWND window,UINT msg,WPARAM wp,LPARAM lp){
 if(msg==WM_COMMAND){if(LOWORD(wp)==101&&!busy)begin();else if(LOWORD(wp)==102&&!busy)DestroyWindow(window);return 0;}
 if(msg==progressMessage){SetWindowTextW(statusText,L"Instalando VST3 y motor del navegador...");SendMessageW(progressBar,PBM_SETPOS,wp,0);return 0;}
 if(msg==resultMessage){std::unique_ptr<Result> result((Result*)lp);busy=false;EnableWindow(cancelButton,TRUE);SetWindowTextW(cancelButton,L"Cerrar");EnableMenuItem(GetSystemMenu(window,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_ENABLED);SetWindowTextW(statusText,result->ok?L"Instalacion completada.":L"La instalacion no se ha completado.");if(!result->ok)EnableWindow(installButton,TRUE);MessageBoxW(window,result->text.c_str(),L"JOSEADOR",MB_OK|(result->ok?MB_ICONINFORMATION:MB_ICONERROR));return 0;}
 if(msg==WM_CLOSE){if(!busy)DestroyWindow(window);return 0;}
 if(msg==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProcW(window,msg,wp,lp);
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR command,int){
 wchar_t executable[32768];GetModuleFileNameW(nullptr,executable,32768);sourceRoot=std::filesystem::path(executable).parent_path();
 if(std::wstring(command)==L"--verify-payload"){try{setup::validate(sourceRoot/L"JOSEADOR.vst3");return microsoftSigned(sourceRoot/L"Prerequisites/VC_redist.x64.exe")?0:3;}catch(...){return 2;}}
 HANDLE single=CreateMutexW(nullptr,TRUE,L"Local\\JOSEADOR-Installer");if(!single||GetLastError()==ERROR_ALREADY_EXISTS){MessageBoxW(nullptr,L"Ya hay una instalacion de JOSEADOR abierta.",L"JOSEADOR",MB_OK|MB_ICONINFORMATION);if(single)CloseHandle(single);return 1;}
 PWSTR common=nullptr;if(FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFilesCommonX64,0,nullptr,&common))){CloseHandle(single);return 4;}pluginRoot=std::filesystem::path(common)/L"VST3";CoTaskMemFree(common);
 PWSTR data=nullptr;if(FAILED(SHGetKnownFolderPath(FOLDERID_ProgramData,0,nullptr,&data))){CloseHandle(single);return 4;}backupRoot=std::filesystem::path(data)/L"JOSEADOR/PreviousVersions";CoTaskMemFree(data);
 INITCOMMONCONTROLSEX controls{sizeof(controls),ICC_PROGRESS_CLASS};InitCommonControlsEx(&controls);SetProcessDPIAware();WNDCLASSW wc{};wc.lpfnWndProc=proc;wc.hInstance=instance;wc.lpszClassName=L"JOSEADOR-Installer";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&wc);
 mainWindow=CreateWindowExW(0,wc.lpszClassName,L"Instalar JOSEADOR 0.4.2",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,610,365,nullptr,nullptr,instance,nullptr);
 font=CreateFontW(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");titleFont=CreateFontW(-27,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
 auto title=control(L"STATIC",L"JOSEADOR",0,24,20,540,40);SendMessageW(title,WM_SETFONT,(WPARAM)titleFont,TRUE);control(L"STATIC",L"Instrumento VST3 x64 por PAPIELGUAPO",0,25,61,540,25);
 control(L"STATIC",L"Cierra tu DAW. Instalacion completa, sin necesitar una version anterior.\nSi falta Visual C++, se abrira el instalador oficial de Microsoft.",0,25,101,550,48);control(L"STATIC",L"Destino compartido para todos los usuarios:",0,25,160,540,22);control(L"EDIT",pluginRoot.c_str(),ES_READONLY|ES_AUTOHSCROLL|WS_BORDER,25,185,545,26);
 statusText=control(L"STATIC",L"Listo para instalar JOSEADOR 0.4.2 y su navegador.",0,25,225,545,22);progressBar=control(PROGRESS_CLASSW,L"",0,25,252,545,13);SendMessageW(progressBar,PBM_SETRANGE32,0,100);
 installButton=control(L"BUTTON",L"Instalar",BS_DEFPUSHBUTTON,350,284,105,31,101);cancelButton=control(L"BUTTON",L"Cancelar",0,465,284,105,31,102);
 ShowWindow(mainWindow,SW_SHOW);MSG message{};while(GetMessageW(&message,nullptr,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}DeleteObject(font);DeleteObject(titleFont);ReleaseMutex(single);CloseHandle(single);return 0;
}
