#include "../Source/NavigationQueue.h"
#include <iostream>
int main(){NavigationQueue queue;NavigationRequest request;bool ok=queue.push(1,L"https://example.com/first")&&queue.push(6)&&queue.pop(request)&&request.action==1&&std::wstring(request.address)==L"https://example.com/first"&&queue.pop(request)&&request.action==6&&!queue.pop(request);
 for(int round=0;round<4;++round){for(unsigned i=0;i<queue.capacity;++i)ok=queue.push(i+1)&&ok;ok=!queue.push(99)&&ok;for(unsigned i=0;i<queue.capacity;++i)ok=queue.pop(request)&&request.action==i+1&&ok;}
 std::cout<<(ok?"PASS":"FAIL")<<": ordered commands, wraparound and no overwrite on overflow\n";return ok?0:1;}
