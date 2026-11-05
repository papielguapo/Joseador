#pragma once
#include <cstdint>
// Renderer messages describe gestures; only recent native input authorizes one.
struct InputGate {
    int axis=0,direction=0;uint64_t timestamp=0;bool pending=false;
    void record(int inputAxis,int inputDirection,uint64_t now){axis=inputAxis;direction=inputDirection;timestamp=now;pending=true;}
    void consume(){pending=false;}
    bool accept(bool horizontal,bool stories,int requestedDirection,uint64_t now){
        if(!pending||now<timestamp||now-timestamp>500||requestedDirection!=direction)return false;
        if(axis==2&&!horizontal&&!stories)return false;
        if(axis!=2&&horizontal)return false;
        pending=false;return true;
    }
};
