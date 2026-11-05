#define NOMINMAX
#include "../Source/PasswordStore.h"
#include <iostream>
int main(){
    const std::wstring site=L"https://webcapture-test.invalid";
    std::wstring user,secret;
    const bool ok=passwords::origin(L"https://EXAMPLE.com/path?q=1")==L"https://example.com"
      &&passwords::origin(L"http://example.com").empty()
      &&passwords::origin(L"https://user@example.com").empty()
      &&passwords::save(site,L"synthetic-account",L"synthetic-test-value")
      &&passwords::read(site,user,secret)&&user==L"synthetic-account"&&secret==L"synthetic-test-value";
    passwords::wipe(secret);
    const bool removed=CredDeleteW(passwords::target(site).c_str(),CRED_TYPE_GENERIC,0)!=FALSE;
    std::cout<<(ok&&removed?"PASS: isolated credential save/read/delete and HTTPS scope":"FAIL")<<"\n";
    return ok&&removed?0:1;
}
