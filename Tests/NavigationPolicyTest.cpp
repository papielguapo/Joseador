#include "../Source/NavigationPolicy.h"
#include <iostream>
int main(){
    int failed=0;const auto check=[&](bool value){if(!value)++failed;};
    for(const auto*url:{L"https://example.com/a?q=1",L"http://localhost:8080/",L"HTTPS://EXAMPLE.COM",L"webcapture://home"})check(navigation::allowed(url));
    for(const auto*url:{L"javascript:alert(1)",L"file:///C:/private.txt",L"data:text/html,test",L"https:///",L"https://user:pass@example.com",L"https://example.com\n",L"https://example.com\\evil"})check(!navigation::allowed(url));
    check(!navigation::allowed(L"https://example.com/"+std::wstring(2048,L'x')));
    check(navigation::projectAddress(L"https://youtube.com/watch?v=123&access_token=secret&list=ok")==L"https://youtube.com/watch?v=123&list=ok");
    check(navigation::projectAddress(L"https://example.com/?co%64e=secret&ok=1")==L"https://example.com/?ok=1");
    check(navigation::projectAddress(L"https://example.com/#access_token=secret")==L"https://example.com/");
    check(navigation::projectAddress(L"https://example.com/a#section")==L"https://example.com/a#section");
    check(navigation::projectAddress(L"file:///C:/private.txt")==L"webcapture://home");
    check(navigation::projectAddress(L"https://example.com/?code=secret")==L"https://example.com/");
    std::cout<<(failed?"FAIL":"PASS")<<": 18 URL policy and project-state privacy cases\n";return failed?1:0;
}
