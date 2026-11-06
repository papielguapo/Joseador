#pragma once
#include <windows.h>
#include <string>
#include <fstream>
#include <cstdint>
namespace parttimegang {
inline constexpr wchar_t origin[]=L"https://part-time-gang.tangycondor.chatgpt.site";
struct ReadState {
    uint64_t lastRead=0;std::wstring path;
    void open(const std::wstring& profile){path=profile+L"\\PartTimeGang.read";std::ifstream file(path);uint64_t value=0;if(file>>value&&value<=9007199254740991ull)lastRead=value;}
    bool accept(const std::wstring& pageOrigin,const std::wstring& message,bool visible){
        constexpr auto prefix=L"__PTG_READ__:";
        if(pageOrigin!=origin||!visible||message.rfind(prefix,0)!=0)return false;
        const auto digits=message.substr(13);if(digits.empty()||digits.size()>16)return false;
        uint64_t value=0;for(auto c:digits){if(c<L'0'||c>L'9')return false;value=value*10+(c-L'0');}
        if(value<=lastRead||value>9007199254740991ull)return false;lastRead=value;
        if(!path.empty()){const auto temp=path+L".tmp";{std::ofstream file(temp,std::ios::trunc);if(!file)return true;file<<lastRead;if(!file)return true;}MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);}
        return true;
    }
};
}
