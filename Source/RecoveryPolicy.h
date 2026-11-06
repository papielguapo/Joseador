#pragma once
#include <cstdint>
struct RecoveryPolicy {
 uint64_t last=0;bool attempted=false;
 bool allow(uint64_t now){if(attempted&&(now<last||now-last<30000))return false;last=now;attempted=true;return true;}
};
