#pragma once
#include "Session.h"
class BrowserView final : public juce::Component,private juce::Timer {
public:
    explicit BrowserView(BrowserSession& s):session(s){startTimerHz(15);}
    ~BrowserView()override{stopTimer();session.detach();if(container)DestroyWindow(container);}
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& wheel)override{if(wheel.deltaY!=0)session.command(wheel.deltaY<0?4:5);}
private:
    BrowserSession& session;
    HWND container=nullptr;
    void timerCallback()override {
        if(!getPeer())return;
        if(!container)container=CreateWindowExW(0,L"STATIC",L"Joseador",WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,
            0,0,10,10,(HWND)getPeer()->getNativeHandle(),nullptr,GetModuleHandle(nullptr),nullptr);
        if(!container)return;
        const auto logical=getPeer()->getComponent().getLocalArea(this,getLocalBounds());
        const auto physical=(logical.toDouble()*getPeer()->getPlatformScaleFactor()).toNearestInt();
        SetWindowPos(container,nullptr,physical.getX(),physical.getY(),physical.getWidth(),physical.getHeight(),SWP_NOZORDER|SWP_NOACTIVATE);
        if(isShowing()){ShowWindow(container,SW_SHOWNA);session.attach(container,physical.getWidth(),physical.getHeight());}
        else{session.detach();ShowWindow(container,SW_HIDE);}
    }
};
