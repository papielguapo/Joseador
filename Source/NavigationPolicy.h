#pragma once
#include <string>
#include <cwctype>
#include <initializer_list>
namespace navigation {
inline std::wstring lower(std::wstring value){for(auto&c:value)c=(wchar_t)towlower(c);return value;}
inline bool allowed(const std::wstring& address){
    if(address==L"webcapture://home")return true;
    if(address.size()>2047)return false;
    for(auto c:address)if(c<=32||c==127||c==L'\\')return false;
    const auto value=lower(address);const auto start=value.rfind(L"https://",0)==0?8:value.rfind(L"http://",0)==0?7:0;
    if(!start)return false;const auto end=value.find_first_of(L"/?#",start);const auto authority=value.substr(start,end==std::wstring::npos?end:end-start);
    return !authority.empty()&&authority.find(L'@')==std::wstring::npos;
}
inline bool sensitive(std::wstring key){
    // Decode escaped parameter names before comparing, e.g. co%64e.
    std::wstring decoded;const auto hex=[](wchar_t c)->int{c=(wchar_t)towlower(c);return c>=L'0'&&c<=L'9'?c-L'0':c>=L'a'&&c<=L'f'?c-L'a'+10:-1;};
    for(size_t i=0;i<key.size();++i){if(key[i]==L'%'&&i+2<key.size()&&hex(key[i+1])>=0&&hex(key[i+2])>=0){decoded+=(wchar_t)(hex(key[i+1])*16+hex(key[i+2]));i+=2;}else decoded+=key[i];}
    decoded=lower(decoded);
    for(const auto*name:{L"code",L"token",L"access_token",L"id_token",L"refresh_token",L"password",L"passwd",L"pass",L"pwd",L"session",L"sessionid",L"auth",L"authorization",L"state"})if(decoded==name)return true;
    return false;
}
inline std::wstring projectAddress(const std::wstring& address){
    if(!allowed(address))return L"webcapture://home";
    const auto hash=address.find(L'#'),query=address.find(L'?');const auto end=hash==std::wstring::npos?address.size():hash;
    std::wstring result=address.substr(0,query<end?query:end);
    if(query<end){bool first=true;size_t start=query+1;while(start<end){auto next=address.find(L'&',start);if(next==std::wstring::npos||next>end)next=end;auto part=address.substr(start,next-start);if(!sensitive(part.substr(0,part.find(L'=')))){result+=first?L'?':L'&';result+=part;first=false;}start=next+1;}}
    // OAuth fragments can contain bearer tokens. Ordinary anchors remain valid.
    if(hash!=std::wstring::npos){const auto fragment=address.substr(hash+1);bool secret=false;size_t start=0;while(start<fragment.size()){const auto next=fragment.find(L'&',start);const auto part=fragment.substr(start,next==std::wstring::npos?next:next-start);secret=secret||sensitive(part.substr(0,part.find(L'=')));if(next==std::wstring::npos)break;start=next+1;}if(!secret)result+=L"#"+fragment;}
    return result;
}
}
