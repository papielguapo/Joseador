#pragma once
#include <windows.h>
#include <atomic>
#include <cstdint>
#include <cstring>

namespace webcapture {
constexpr uint32_t capacity=262144;
struct alignas(64) Bridge {
    std::atomic<uint32_t> read {0},write {0},sampleRate {48000},audioRate {0};
    std::atomic<uint32_t> ready {0},quit {0},streaming {0},fault {0},dropped {0};
    std::atomic<uint64_t> window {0},heartbeat {0};
    std::atomic<float> peak {0};
    // Navigation is confined to the UI threads. Named mutex protects these fields.
    uint32_t command=0;
    wchar_t url[2048] {};
    wchar_t error[256] {};
    float audio[capacity*2] {};
};
static_assert(std::atomic<uint32_t>::is_always_lock_free);
static_assert(std::atomic<float>::is_always_lock_free);
inline void push(Bridge& b,const float** channels,int frames,int channelCount) {
    const auto w=b.write.load(std::memory_order_relaxed),r=b.read.load(std::memory_order_acquire);
    const auto available=capacity-(w-r);
    const auto count=(uint32_t)frames<available?(uint32_t)frames:available;
    float peak=0;
    for(uint32_t i=0;i<count;++i) for(int c=0;c<2;++c) {
        const auto value=channels[c<channelCount?c:0][i];
        b.audio[((w+i)&(capacity-1))*2+c]=value;
        const auto abs=value<0?-value:value;if(abs>peak)peak=abs;
    }
    b.peak.store(peak,std::memory_order_relaxed);
    b.write.store(w+count,std::memory_order_release);
    if(count<(uint32_t)frames)b.dropped.fetch_add((uint32_t)frames-count);
}
}
