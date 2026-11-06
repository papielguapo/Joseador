#pragma once
#include <string>
#include <cwctype>
#include <initializer_list>
namespace navigation {
inline std::wstring lower(std::wstring value){for(auto&c:value)c=(wchar_t)towlower(c);return value;}
inline std::wstring host(const std::wstring& address){const auto value=lower(address);const auto start=value.rfind(L"https://",0)==0?8:value.rfind(L"http://",0)==0?7:0;if(!start)return{};const auto end=value.find_first_of(L"/?#",start);auto authority=value.substr(start,end==std::wstring::npos?end:end-start);if(authority.find(L'@')!=std::wstring::npos)return{};return authority.substr(0,authority.find(L':'));}
inline bool domain(const std::wstring& value,const std::wstring& parent){return value==parent||(value.size()>parent.size()&&value.compare(value.size()-parent.size()-1,parent.size()+1,L"."+parent)==0);}
inline bool tiktok(const std::wstring& address){return domain(host(address),L"tiktok.com");}
inline bool menuSite(const std::wstring& address){const auto h=host(address);for(const auto*site:{L"youtube.com",L"youtu.be",L"tiktok.com",L"instagram.com",L"facebook.com",L"fb.com",L"x.com",L"twitter.com",L"reddit.com",L"twitch.tv",L"soundcloud.com",L"spotify.com",L"google.com",L"gmail.com",L"part-time-gang.tangycondor.chatgpt.site"})if(domain(h,site))return true;return false;}
inline bool appBridge(const std::wstring& address){const auto h=host(address);return domain(h,L"onelink.me")||domain(h,L"app.link")||domain(h,L"page.link");}
inline bool tiktokAppLink(const std::wstring& target){const auto h=host(target);return h==L"snssdk1233.onelink.me"||h==L"snssdk1180.onelink.me"||h==L"tiktok.onelink.me";}
inline bool appRedirect(const std::wstring& target,const std::wstring& current){return tiktokAppLink(target)||(menuSite(current)&&appBridge(target));}
inline bool browserScheme(const std::wstring& address){const auto value=lower(address);const auto scheme=value.substr(0,value.find(L':'));return scheme==L"http"||scheme==L"https"||scheme==L"file"||scheme==L"data"||scheme==L"blob"||value==L"about:blank"||value==L"about:srcdoc"||value.rfind(L"about:blank#",0)==0;}
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
    if(tiktokAppLink(address))return L"https://www.tiktok.com/foryou";
    if(appBridge(address))return L"webcapture://home";
    if(!allowed(address))return L"webcapture://home";
    const auto hash=address.find(L'#'),query=address.find(L'?');const auto end=hash==std::wstring::npos?address.size():hash;
    std::wstring result=address.substr(0,query<end?query:end);
    if(query<end){bool first=true;size_t start=query+1;while(start<end){auto next=address.find(L'&',start);if(next==std::wstring::npos||next>end)next=end;auto part=address.substr(start,next-start);if(!sensitive(part.substr(0,part.find(L'=')))){result+=first?L'?':L'&';result+=part;first=false;}start=next+1;}}
    // OAuth fragments can contain bearer tokens. Ordinary anchors remain valid.
    if(hash!=std::wstring::npos){const auto fragment=address.substr(hash+1);bool secret=false;size_t start=0;while(start<fragment.size()){const auto next=fragment.find(L'&',start);const auto part=fragment.substr(start,next==std::wstring::npos?next:next-start);secret=secret||sensitive(part.substr(0,part.find(L'=')));if(next==std::wstring::npos)break;start=next+1;}if(!secret)result+=L"#"+fragment;}
    return result;
}
}
