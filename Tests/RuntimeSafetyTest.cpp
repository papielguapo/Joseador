#include "../Source/AudioAccess.h"
#include "../Source/RecoveryPolicy.h"
#include "../Source/ConnectivityState.h"
#include <thread>
#include <iostream>
int main(){
 int failures=0;auto check=[&](bool v){if(!v)++failures;};
 RecoveryPolicy recovery;check(recovery.allow(0));check(!recovery.allow(1));check(!recovery.allow(29999));check(recovery.allow(30000));check(!recovery.allow(29999));
 ConnectivityState connection;check(connection.update(-1,-1)==ConnectivityState::none);check(connection.update(2,1)==ConnectivityState::none);
 check(connection.update(2,0)==ConnectivityState::lost);check(connection.offline);check(connection.update(2,0)==ConnectivityState::none);
 check(connection.update(-1,-1)==ConnectivityState::none);check(connection.offline);check(connection.update(2,1)==ConnectivityState::restored);check(!connection.offline);
 check(connection.update(1,1)==ConnectivityState::none);check(connection.update(0,-1)==ConnectivityState::lost);check(connection.update(1,1)==ConnectivityState::restored);
 struct Block{std::atomic<bool> retired{false};};
 AudioAccess<Block> access;std::atomic<bool> stop{false};std::atomic<int> invalid{0};
 std::thread audio([&]{while(!stop){AudioAccess<Block>::Read read(access);if(read.value){if(read.value->retired)++invalid;std::this_thread::yield();if(read.value->retired)++invalid;}}});
 for(int i=0;i<20000;++i){Block block;access.pointer.store(&block);std::this_thread::yield();access.pointer.store(nullptr);while(access.readers.load())std::this_thread::yield();block.retired=true;}
 stop=true;audio.join();check(invalid==0);
 std::cout<<(failures?"FAIL":"PASS")<<": 20000 audio publication/retirement cycles; bounded renderer recovery\n";
 return failures?1:0;
}
