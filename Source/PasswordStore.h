#pragma once
#include <windows.h>
#include <wincred.h>
#include <string>
#include <cwctype>
#include "NavigationPolicy.h"
namespace passwords {
inline std::wstring origin(std::wstring url){
    if(!navigation::allowed(url))return{};
    const auto pos=url.find(L"://");if(pos==std::wstring::npos)return{};
    auto value=url.substr(0,url.find_first_of(L"/?#",pos+3));
    for(auto&c:value)c=(wchar_t)towlower(c);
    if(value.rfind(L"https://",0)!=0||value.find(L'@')!=std::wstring::npos)return{};
    return value;
}
inline std::wstring target(const std::wstring&site){return L"WebCaptureMobile:"+site;}
inline bool sameTopOrigin(const std::wstring&dialog,const std::wstring&top){const auto site=origin(dialog);return !site.empty()&&site==origin(top);}
inline bool read(const std::wstring&site,std::wstring&user,std::wstring&secret){
    if(site.empty()||origin(site)!=site)return false;PCREDENTIALW cred=nullptr;
    if(!CredReadW(target(site).c_str(),CRED_TYPE_GENERIC,0,&cred))return false;
    if(!cred->CredentialBlobSize||cred->CredentialBlobSize>CRED_MAX_CREDENTIAL_BLOB_SIZE||cred->CredentialBlobSize%sizeof(wchar_t)||!cred->CredentialBlob){CredFree(cred);return false;}
    if(cred->UserName)user=cred->UserName;
    secret.assign((wchar_t*)cred->CredentialBlob,cred->CredentialBlobSize/sizeof(wchar_t));
    if(cred->CredentialBlobSize)SecureZeroMemory(cred->CredentialBlob,cred->CredentialBlobSize);CredFree(cred);return true;
}
inline bool save(const std::wstring&site,const std::wstring&user,const std::wstring&secret){
    if(site.empty()||origin(site)!=site||user.size()>CRED_MAX_USERNAME_LENGTH||secret.empty()||secret.size()*sizeof(wchar_t)>CRED_MAX_CREDENTIAL_BLOB_SIZE)return false;
    auto name=target(site);CREDENTIALW cred{};cred.Type=CRED_TYPE_GENERIC;cred.TargetName=name.data();
    cred.UserName=(LPWSTR)user.c_str();cred.CredentialBlob=(LPBYTE)secret.data();cred.CredentialBlobSize=(DWORD)(secret.size()*sizeof(wchar_t));cred.Persist=CRED_PERSIST_LOCAL_MACHINE;
    return CredWriteW(&cred,0)!=FALSE;
}
inline std::wstring json(const std::wstring&text){std::wstring result=L"\"";for(auto c:text){switch(c){case L'"':result+=L"\\\"";break;case L'\\':result+=L"\\\\";break;case L'\n':result+=L"\\n";break;case L'\r':result+=L"\\r";break;case L'\t':result+=L"\\t";break;default:if(c>=32)result+=c;break;}}return result+L"\"";}
inline void wipe(std::wstring&secret){if(!secret.empty())SecureZeroMemory(secret.data(),secret.size()*sizeof(wchar_t));secret.clear();}
}
