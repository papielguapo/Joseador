#define NOMINMAX
#include "../Source/PasswordStore.h"
#include <iostream>
int main(){
    const std::wstring site=L"https://webcapture-test.invalid";
    std::wstring user,secret;
    const bool ok=passwords::origin(L"https://EXAMPLE.com/path?q=1")==L"https://example.com"
      &&passwords::origin(L"http://example.com").empty()
      &&passwords::origin(L"https://user@example.com").empty()
      &&passwords::origin(L"https:///").empty()
      &&passwords::sameTopOrigin(L"https://example.com",L"https://example.com/login")
      &&!passwords::sameTopOrigin(L"https://iframe.invalid",L"https://example.com/login")
      &&!passwords::sameTopOrigin(L"https://example.com",L"http://example.com/login")
      &&!passwords::save(L"http://example.com",L"synthetic",L"synthetic")
      &&!passwords::save(site,std::wstring(CRED_MAX_USERNAME_LENGTH+1,L'x'),L"synthetic")
      &&passwords::save(site,L"synthetic-account",L"synthetic-test-value")
      &&passwords::read(site,user,secret)&&user==L"synthetic-account"&&secret==L"synthetic-test-value";
    passwords::wipe(secret);
    const bool removed=CredDeleteW(passwords::target(site).c_str(),CRED_TYPE_GENERIC,0)!=FALSE;
    std::cout<<(ok&&removed?"PASS: credential save/read/delete, HTTPS/top origin scope and limits":"FAIL")<<"\n";
    return ok&&removed?0:1;
}
