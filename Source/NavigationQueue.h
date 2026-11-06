#pragma once
#include <cstdint>
#include <cwchar>
struct NavigationRequest{uint32_t action=0;wchar_t address[2048]{};};
struct NavigationQueue {
    static constexpr uint32_t capacity=16;
    NavigationRequest entries[capacity]{};uint32_t head=0,count=0;
    // Caller holds the navigation mutex. Never overwrite pending actions.
    bool push(uint32_t action,const wchar_t* address=L""){
        if(count>=capacity||head>=capacity)return false;
        auto&entry=entries[(head+count)%capacity];entry.action=action;
        size_t i=0;if(address)for(;i<2047&&address[i];++i)entry.address[i]=address[i];entry.address[i]=0;++count;return true;
    }
    bool pop(NavigationRequest&request){if(!count||head>=capacity||count>capacity)return false;request=entries[head];head=(head+1)%capacity;--count;return true;}
    uint32_t nextAction()const{return count&&head<capacity&&count<=capacity?entries[head].action:0;}
};
