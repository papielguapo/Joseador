#include "../Installer/InstallCore.h"
#include <fstream>
#include <iostream>
int main(){namespace fs=std::filesystem;const auto root=fs::temp_directory_path()/(L"JOSEADOR-Installer-Test-"+std::to_wstring(GetCurrentProcessId()));if(fs::exists(root))return 2;const auto source=root/L"source",dest=root/L"vst3",backup=root/L"backup";bool ok=true;
 try{for(const auto*name:setup::requiredFiles){const auto path=source/name;fs::create_directories(path.parent_path());std::ofstream(path)<<"synthetic";}
  setup::install(source,dest,backup);ok=fs::exists(dest/L"JOSEADOR.vst3/Contents/x86_64-win/BrowserRuntime/libcef.dll");std::ofstream(dest/L"JOSEADOR.vst3/old-version")<<"keep";
  bool failed=false;try{setup::install(source,dest,backup,{},[]{throw std::runtime_error("simulated commit failure");});}catch(...){failed=true;}ok=ok&&failed&&fs::exists(dest/L"JOSEADOR.vst3/old-version");
  setup::install(source,dest,backup);ok=ok&&!fs::exists(dest/L"JOSEADOR.vst3/old-version");bool kept=false;for(const auto&e:fs::directory_iterator(backup))kept=kept||fs::exists(e.path()/L"old-version");ok=ok&&kept;
  fs::remove(source/L"Contents/x86_64-win/BrowserRuntime/libcef.dll");failed=false;try{setup::install(source,dest,backup);}catch(...){failed=true;}ok=ok&&failed&&fs::exists(dest/L"JOSEADOR.vst3/Contents/x86_64-win/BrowserRuntime/libcef.dll");
 }catch(const std::exception&e){std::cerr<<e.what()<<"\n";ok=false;}
 if(setup::inside(root,fs::temp_directory_path())){std::error_code error;fs::remove_all(root,error);}std::cout<<(ok?"PASS":"FAIL")<<": installer fresh copy, update backup, rollback and incomplete payload\n";return ok?0:1;
}
