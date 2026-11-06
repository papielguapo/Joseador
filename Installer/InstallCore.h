#pragma once
#include <windows.h>
#include <filesystem>
#include <functional>
#include <vector>
#include <stdexcept>
#include <string>
namespace setup {
namespace fs=std::filesystem;
inline constexpr const wchar_t* requiredFiles[]={L"Contents/x86_64-win/JOSEADOR.vst3",L"Contents/Resources/moduleinfo.json",L"Contents/x86_64-win/BrowserRuntime/MobileBrowser.exe",L"Contents/x86_64-win/BrowserRuntime/MobileBrowser.dll",L"Contents/x86_64-win/BrowserRuntime/libcef.dll",L"Contents/x86_64-win/BrowserRuntime/start.html",L"Contents/x86_64-win/BrowserRuntime/home-chat.js",L"Contents/x86_64-win/BrowserRuntime/mobile-input.js",L"Contents/x86_64-win/BrowserRuntime/passwords.js",L"Contents/x86_64-win/BrowserRuntime/resources.pak",L"Contents/x86_64-win/BrowserRuntime/icudtl.dat",L"Contents/x86_64-win/BrowserRuntime/v8_context_snapshot.bin",L"Contents/x86_64-win/BrowserRuntime/chrome_elf.dll",L"Contents/x86_64-win/BrowserRuntime/locales/en-US.pak"};
inline bool inside(const fs::path& candidate,const fs::path& root){auto part=fs::weakly_canonical(candidate).lexically_relative(fs::weakly_canonical(root));return !part.empty()&&!part.is_absolute()&&*part.begin()!=L"..";}
inline void validate(const fs::path& bundle){
 for(const auto*name:requiredFiles)if(!fs::is_regular_file(bundle/name))throw std::runtime_error("Extrae el ZIP completo. Falta un archivo del VST3 o del navegador.");
 for(const auto&entry:fs::recursive_directory_iterator(bundle))if(entry.is_symlink()||!inside(entry.path(),bundle))throw std::runtime_error("El paquete contiene una ruta no permitida.");
}
inline fs::path unused(const fs::path& root,const std::wstring& prefix){const auto stamp=std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());for(unsigned i=0;i<1000;++i){auto path=root/(prefix+stamp+L"-"+std::to_wstring(i));if(!fs::exists(path))return path;}throw std::runtime_error("No se pudo preparar una carpeta temporal.");}
inline fs::path install(const fs::path& bundle,const fs::path& destination,const fs::path& backups,const std::function<void(unsigned)>& progress={},const std::function<void()>& beforeCommit={}){
 validate(bundle);fs::create_directories(destination);fs::create_directories(backups);
 const auto staging=unused(destination,L"JOSEADOR-stage-");if(!inside(staging,destination))throw std::runtime_error("Ruta temporal no permitida.");
 std::vector<std::pair<fs::path,fs::path>> moved;
 try{
  std::vector<fs::path> files;for(const auto&entry:fs::recursive_directory_iterator(bundle))if(entry.is_regular_file())files.push_back(entry.path());
  fs::create_directory(staging);unsigned copied=0;
  for(const auto&file:files){const auto target=staging/file.lexically_relative(bundle);if(!inside(target,staging))throw std::runtime_error("Ruta de copia no permitida.");fs::create_directories(target.parent_path());fs::copy_file(file,target,fs::copy_options::none);if(progress)progress((unsigned)(++copied*90/files.size()));}
  for(const auto*name:{L"JOSEADOR.vst3",L"WebCapture Mobile.vst3"}){const auto current=destination/name;if(fs::exists(current)){const auto backup=unused(backups,std::wstring(name)+L"-");if(!inside(current,destination)||!inside(backup,backups))throw std::runtime_error("Ruta de respaldo no permitida.");fs::rename(current,backup);moved.emplace_back(current,backup);}}
  if(beforeCommit)beforeCommit();const auto target=destination/L"JOSEADOR.vst3";if(!inside(target,destination))throw std::runtime_error("Destino no permitido.");fs::rename(staging,target);if(progress)progress(100);return target;
 }catch(...){
  for(auto it=moved.rbegin();it!=moved.rend();++it){std::error_code error;if(!fs::exists(it->first)&&inside(it->first,destination)&&inside(it->second,backups))fs::rename(it->second,it->first,error);}
  std::error_code error;if(fs::exists(staging)&&inside(staging,destination))fs::remove_all(staging,error);throw;
 }
}
}
