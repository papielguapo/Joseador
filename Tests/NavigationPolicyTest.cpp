#include "../Source/NavigationPolicy.h"
#include <iostream>
int main(){
    int failed=0,checks=0;const auto check=[&](bool value){++checks;if(!value)++failed;};
    for(const auto*url:{L"https://example.com/a?q=1",L"http://localhost:8080/",L"HTTPS://EXAMPLE.COM",L"webcapture://home"})check(navigation::allowed(url));
    for(const auto*url:{L"javascript:alert(1)",L"file:///C:/private.txt",L"data:text/html,test",L"https:///",L"https://user:pass@example.com",L"https://example.com\n",L"https://example.com\\evil"})check(!navigation::allowed(url));
    check(!navigation::allowed(L"https://example.com/"+std::wstring(2048,L'x')));
    check(navigation::projectAddress(L"https://youtube.com/watch?v=123&access_token=secret&list=ok")==L"https://youtube.com/watch?v=123&list=ok");
    check(navigation::projectAddress(L"https://example.com/?co%64e=secret&ok=1")==L"https://example.com/?ok=1");
    check(navigation::projectAddress(L"https://example.com/#access_token=secret")==L"https://example.com/");
    check(navigation::projectAddress(L"https://example.com/a#section")==L"https://example.com/a#section");
    check(navigation::projectAddress(L"file:///C:/private.txt")==L"webcapture://home");
    check(navigation::projectAddress(L"https://example.com/?code=secret")==L"https://example.com/");
    check(navigation::tiktok(L"https://www.tiktok.com/foryou"));check(!navigation::tiktok(L"https://not-tiktok.com/"));
    check(navigation::appRedirect(L"https://snssdk1233.onelink.me/test?af_dp=example",L"https://www.tiktok.com/foryou"));
    check(!navigation::appRedirect(L"https://example.com/onelink.me",L"https://www.tiktok.com/foryou"));
    for(const auto*url:{L"intent://aweme/detail/123#Intent;scheme=snssdk1233;end",L"snssdk1233://aweme/detail/123",L"market://details?id=test",L"spotify:track:test",L"instagram://media?id=123",L"fb://reel/123",L"fb-messenger://thread/123",L"youtube://watch/123",L"twitter://status/123",L"reddit://post/123",L"twitch://stream/test",L"soundcloud://tracks/123",L"googlegmail://",L"intent://test#Intent;scheme=instagram;end"})check(!navigation::browserScheme(url));
    for(const auto*url:{L"https://www.tiktok.com/foryou",L"http://localhost/",L"file:///C:/app/start.html",L"about:blank",L"data:text/html,test",L"blob:https://example.com/id"})check(navigation::browserScheme(url));
    for(const auto*site:{L"https://m.youtube.com/",L"https://www.tiktok.com/@test",L"https://www.instagram.com/",L"https://m.facebook.com/",L"https://x.com/",L"https://www.reddit.com/",L"https://m.twitch.tv/",L"https://m.soundcloud.com/",L"https://open.spotify.com/",L"https://mail.google.com/",L"https://part-time-gang.tangycondor.chatgpt.site/"}){
        check(navigation::menuSite(site));check(navigation::appRedirect(L"https://example.app.link/open",site));check(!navigation::appRedirect(L"https://example.com/app.link",site));
    }
    check(!navigation::menuSite(L"https://tiktok.com.evil.example/"));
    check(!navigation::appRedirect(L"https://example.app.link/open",L"https://example.com/"));
    check(navigation::projectAddress(L"https://snssdk1233.onelink.me/test")==L"https://www.tiktok.com/foryou");
    check(navigation::projectAddress(L"https://example.app.link/test")==L"webcapture://home");
    std::cout<<(failed?"FAIL":"PASS")<<": "<<checks<<" URL policy and project-state privacy cases\n";return failed?1:0;
}
