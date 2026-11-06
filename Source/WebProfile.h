#pragma once
#include <string>
namespace webprofile {
inline std::string parameters(bool tiktok){
 const std::string agent=tiktok?"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/150.0.0.0 Safari/537.36":"Mozilla/5.0 (Linux; Android 13; Pixel 7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/150.0.0.0 Mobile Safari/537.36";
 return "{\"userAgent\":\""+agent+"\",\"platform\":\""+(tiktok?"Win32":"Linux armv8l")+"\",\"userAgentMetadata\":{\"brands\":[{\"brand\":\"Chromium\",\"version\":\"150\"}],\"platform\":\""+(tiktok?"Windows":"Android")+"\",\"platformVersion\":\""+(tiktok?"10.0.0":"13.0.0")+"\",\"architecture\":\""+(tiktok?"x86":"arm")+"\",\"model\":\""+(tiktok?"":"Pixel 7")+"\",\"mobile\":"+(tiktok?"false":"true")+",\"bitness\":\"64\"}}";
}
}
