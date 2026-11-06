#include "../Source/ChatState.h"
#include <iostream>
int main(){parttimegang::ReadState state;bool ok=!state.accept(L"https://evil.invalid",L"__PTG_READ__:10",true)&&!state.accept(parttimegang::origin,L"__PTG_READ__:10",false)&&state.accept(parttimegang::origin,L"__PTG_READ__:10",true)&&state.lastRead==10&&!state.accept(parttimegang::origin,L"__PTG_READ__:9",true)&&!state.accept(parttimegang::origin,L"__PTG_READ__:NaN",true)&&!state.accept(parttimegang::origin,L"__PTG_READ__:9999999999999999",true);
 std::cout<<(ok?"PASS":"FAIL")<<": unread cursor scope, visibility, monotonicity and validation\n";return ok?0:1;}
