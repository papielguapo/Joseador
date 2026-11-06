#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Bridge.h"

class BrowserSession {
public:
    BrowserSession():resampleInput(2,(int)webcapture::capacity){}
    ~BrowserSession(){shutdown();}
    bool start(const juce::File& runtime,bool selfTest=false) {
        if(bridge)return true;
        error.clear();
        name=L"Local\\WebCaptureMobile-"+std::wstring(juce::Uuid().toString().toWideCharPointer());
        mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(webcapture::Bridge),name.c_str());
        if(!mapping){error="No se pudo crear el puente de audio";return false;}
        bridge=(webcapture::Bridge*)MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(webcapture::Bridge));
        if(!bridge){shutdown();error="No se pudo abrir el puente de audio";return false;}
        new(bridge) webcapture::Bridge(); bridge->sampleRate=(uint32_t)rate.load();
        mutex=CreateMutexW(nullptr,FALSE,(name+L"-ui").c_str());
        if(!mutex){error="No se pudo crear el control del navegador";shutdown();return false;}
        auto exe=runtime.getChildFile("MobileBrowser.exe");
        if(!exe.existsAsFile()){error="Falta el motor BrowserRuntime junto al VST3";shutdown();return false;}
        auto command=L"\""+std::wstring(exe.getFullPathName().toWideCharPointer())+L"\" --autoplay-policy=no-user-gesture-required --bridge=\""+name+L"\""+(selfTest?L" --self-test":L"");
        STARTUPINFOW startup{};startup.cb=sizeof(startup);
        const auto cwd=std::wstring(runtime.getFullPathName().toWideCharPointer());
        if(!CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,cwd.c_str(),&startup,&process)) {
            error="No se pudo iniciar el navegador: "+juce::String((int)GetLastError());shutdown();return false;
        }
        published.store(bridge,std::memory_order_release);return true;
    }
    void shutdown() {
        published.store(nullptr,std::memory_order_release);
        if(bridge)bridge->quit=1;
        if(process.hProcess){WaitForSingleObject(process.hProcess,4000);CloseHandle(process.hThread);CloseHandle(process.hProcess);process={};}
        if(bridge){UnmapViewOfFile(bridge);bridge=nullptr;}
        if(mapping){CloseHandle(mapping);mapping=nullptr;}
        if(mutex){CloseHandle(mutex);mutex=nullptr;}
    }
    bool running()const{return process.hProcess&&WaitForSingleObject(process.hProcess,0)==WAIT_TIMEOUT;}
    bool command(uint32_t kind,const juce::String& url={}) {
        if(!bridge)return false;const auto lock=WaitForSingleObject(mutex,100);if(lock!=WAIT_OBJECT_0&&lock!=WAIT_ABANDONED)return false;
        const bool accepted=bridge->navigation.push(kind,url.toWideCharPointer());ReleaseMutex(mutex);return accepted;
    }
    juce::String address()const {
        if(!bridge)return{};const auto lock=WaitForSingleObject(mutex,0);if(lock!=WAIT_OBJECT_0&&lock!=WAIT_ABANDONED)return{};
        juce::String value(bridge->url);ReleaseMutex(mutex);return value;
    }
    void attach(HWND parent,int width,int height) {
        if(!bridge)return;
        HWND child=(HWND)(uintptr_t)bridge->window.load();
        if(!child||!IsWindow(child))return;
        if(GetParent(child)!=parent){ShowWindow(child,SW_HIDE);SetWindowLongPtr(child,GWL_STYLE,WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);SetParent(child,parent);}
        SetWindowPos(child,nullptr,0,0,width,height,SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
    }
    void detach() {
        if(!bridge)return;
        HWND child=(HWND)(uintptr_t)bridge->window.load();
        if(child&&IsWindow(child)){ShowWindow(child,SW_HIDE);SetParent(child,nullptr);SetWindowLongPtr(child,GWL_STYLE,WS_POPUP|WS_CLIPCHILDREN);}
    }
    void pull(juce::AudioBuffer<float>& output) {
        output.clear();auto* b=published.load(std::memory_order_acquire);if(!b)return;
        b->sampleRate.store((uint32_t)rate.load(),std::memory_order_relaxed);
        auto r=b->read.load(std::memory_order_relaxed),w=b->write.load(std::memory_order_acquire);
        auto ready=w-r;const auto sourceRate=b->audioRate.load();
        if(!sourceRate)return;
        if(ready>(uint32_t)(sourceRate*0.20)){b->read.store(w,std::memory_order_release);return;}
        if(sourceRate==(uint32_t)rate.load()){
            const auto count=juce::jmin((uint32_t)output.getNumSamples(),ready);
            for(uint32_t i=0;i<count;++i)for(int c=0;c<juce::jmin(2,output.getNumChannels());++c)
                output.setSample(c,(int)i,b->audio[((r+i)&(webcapture::capacity-1))*2+c]);
            b->read.store(r+count,std::memory_order_release);
        }else{
            const double ratio=sourceRate/rate.load();
            if(ready<(uint32_t)std::ceil(output.getNumSamples()*ratio)+2)return;
            for(uint32_t i=0;i<ready;++i)for(int c=0;c<2;++c)resampleInput.setSample(c,(int)i,b->audio[((r+i)&(webcapture::capacity-1))*2+c]);
            int used=0;
            for(int c=0;c<juce::jmin(2,output.getNumChannels());++c)used=converter[c].process(ratio,resampleInput.getReadPointer(c),output.getWritePointer(c),output.getNumSamples(),(int)ready,0);
            b->read.store(r+(uint32_t)used,std::memory_order_release);
        }
    }
    juce::String status() const {
        if(!error.isEmpty())return error;
        if(!bridge)return "Preparando navegador";
        if(process.hProcess&&WaitForSingleObject(process.hProcess,0)==WAIT_OBJECT_0){DWORD code=0;GetExitCodeProcess(process.hProcess,&code);return "El navegador se cerro ("+juce::String((int)code)+")";}
        if(bridge->fault)return "Error del navegador o del puente de audio";
        if(!bridge->ready)return "Iniciando navegador movil...";
        return "Salida exclusiva al DAW | "+juce::String(rate.load(),0)+" Hz";
    }
    DWORD pid() const{return process.dwProcessId;}
    webcapture::Bridge* data() const{return published.load(std::memory_order_acquire);}
    std::atomic<double> rate{48000};
private:
    webcapture::Bridge* bridge=nullptr;
    std::atomic<webcapture::Bridge*> published{nullptr};
    juce::AudioBuffer<float> resampleInput;
    juce::WindowedSincInterpolator converter[2];
    HANDLE mapping=nullptr,mutex=nullptr;
    PROCESS_INFORMATION process{};
    std::wstring name;
    juce::String error;
};
