#include "Browser.h"
#include "SystemStatus.h"
#include "NavigationPolicy.h"
#include "BinaryData.h"
#include "ConnectivityState.h"
class Processor;
class NavigationButton final:public juce::Button {
public:
    enum Kind { back, forward, refresh, start };
    explicit NavigationButton(Kind k):Button(k==back?"Atras / Inicio":k==forward?"Adelante":k==start?"Inicio":"Actualizar pagina"),kind(k){setTooltip(getName());}
    void paintButton(juce::Graphics& g,bool over,bool down)override{
        const auto bounds=getLocalBounds().toFloat();
        if(over||down){g.setColour(juce::Colour(down?0xff383b40:0xff2d3035));g.fillRoundedRectangle(bounds.reduced(1),8);}
        g.setColour(juce::Colour(0xffc5c7ca).withMultipliedAlpha(isEnabled()?1.0f:0.35f));
        const float cx=bounds.getCentreX(),cy=bounds.getCentreY();
        juce::Path icon;
        if(kind==start){
            for(float offset:{-6.0f,0.0f,6.0f}){icon.startNewSubPath(cx-8,cy+offset);icon.lineTo(cx+8,cy+offset);}
        }else if(kind==refresh){
            icon.addCentredArc(cx,cy,8,8,0,0.7f,6.1f,true);
            icon.startNewSubPath(cx+5,cy-9);icon.lineTo(cx+5,cy-3);icon.lineTo(cx+10,cy-5);
        }else{
            const float direction=kind==back?-1.0f:1.0f;
            icon.startNewSubPath(cx-8*direction,cy);icon.lineTo(cx+8*direction,cy);
            icon.startNewSubPath(cx+1*direction,cy-7);icon.lineTo(cx+8*direction,cy);icon.lineTo(cx+1*direction,cy+7);
        }
        g.strokePath(icon,juce::PathStrokeType(1.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    }
private:
    Kind kind;
};
class Editor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit Editor(Processor&);
    void resized()override;
    void paint(juce::Graphics& g)override;
private:
    Processor& proc;BrowserView browser;juce::TextEditor url;
    NavigationButton home{NavigationButton::back},forward{NavigationButton::forward};
    NavigationButton reload{NavigationButton::refresh},start{NavigationButton::start};
    SystemStatus pc;
    ConnectivityState connection;
    juce::Image offlineImage=juce::ImageCache::getFromMemory(JoseadorAssets::offline_png,JoseadorAssets::offline_pngSize);
    juce::TooltipWindow tips{this,500};
    void timerCallback()override;
};
class Processor final:public juce::AudioProcessor {
public:
    Processor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)){}
    BrowserSession session;juce::String lastUrl="webcapture://home";
    juce::File runtime()const {
        auto module=juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        // In VST3 currentExecutableFile is the plug-in DLL; in Standalone it is the exe.
        return module.getParentDirectory().getChildFile("BrowserRuntime");
    }
    void ensureBrowser(){if(session.data()&&!session.running())session.shutdown();if(!session.data()&&session.start(runtime()))session.command(1,lastUrl);}
    const juce::String getName()const override{return "JOSEADOR";}
    void prepareToPlay(double sr,int)override{session.rate=sr;}
    void releaseResources()override{}
    bool isBusesLayoutSupported(const BusesLayout& l)const override{return l.getMainInputChannelSet().isDisabled()&&l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
    void processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi)override{juce::ScopedNoDenormals scoped;session.pull(b);midi.clear();}
    bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}
    double getTailLengthSeconds()const override{return std::numeric_limits<double>::infinity();}bool hasEditor()const override{return true;}
    juce::AudioProcessorEditor* createEditor()override{ensureBrowser();return new Editor(*this);}
    int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}
    const juce::String getProgramName(int)override{return{};}void changeProgramName(int,const juce::String&)override{}
    void getStateInformation(juce::MemoryBlock& block)override{juce::XmlElement x("WebCaptureMobile");const auto current=session.address();const auto address=current.isNotEmpty()?current:lastUrl;x.setAttribute("url",juce::String(navigation::projectAddress(std::wstring(address.toWideCharPointer())).c_str()));copyXmlToBinary(x,block);}
    void setStateInformation(const void* bytes,int n)override{if(auto x=getXmlFromBinary(bytes,n))if(x->hasTagName("WebCaptureMobile")){const auto saved=x->getStringAttribute("url",lastUrl);lastUrl=juce::String(navigation::projectAddress(std::wstring(saved.toWideCharPointer())).c_str());if(session.data())session.command(1,lastUrl);}}
};
Editor::Editor(Processor& p):AudioProcessorEditor(p),proc(p),browser(p.session){
    setResizable(true,true);setResizeLimits(320,400,1200,1400);setSize(400,680);
    for(auto* c:std::initializer_list<juce::Component*>{&browser,&url,&home,&forward,&reload,&start})addAndMakeVisible(c);
    home.setTitle("Atras / Inicio");
    start.onClick=[this]{proc.lastUrl="webcapture://home";if(!proc.session.running())proc.ensureBrowser();else proc.session.command(1,proc.lastUrl);url.setText({},false);};
    forward.onClick=[this]{if(proc.session.running())proc.session.command(3);};
    reload.onClick=[this]{if(!proc.session.running())proc.ensureBrowser();else proc.session.command(6);};
    url.setText(proc.lastUrl=="webcapture://home"?juce::String():proc.lastUrl);url.setSelectAllWhenFocused(true);
    url.setInputRestrictions(2047);
    home.onClick=[this]{if(!proc.session.running()){proc.lastUrl="webcapture://home";proc.ensureBrowser();}else proc.session.command(2);};
    url.setTextToShowWhenEmpty("Buscar o escribir direccion",juce::Colours::grey);
    url.setFont(juce::Font(16));
    url.setBorder(juce::BorderSize<int>(0));
    url.setIndents(0,0);
    url.setJustification(juce::Justification::centredLeft);
    url.setColour(juce::TextEditor::backgroundColourId,juce::Colours::transparentBlack);
    url.setColour(juce::TextEditor::textColourId,juce::Colour(0xff171719));
    url.setColour(juce::TextEditor::outlineColourId,juce::Colours::transparentBlack);
    url.setColour(juce::TextEditor::focusedOutlineColourId,juce::Colours::transparentBlack);
    url.onReturnKey=[this]{auto address=url.getText().trim();
        if(address.isEmpty()||address.equalsIgnoreCase("inicio")){proc.lastUrl="webcapture://home";if(!proc.session.running())proc.ensureBrowser();else proc.session.command(1,proc.lastUrl);url.setText({},false);return;}
        if(!address.startsWithIgnoreCase("http://")&&!address.startsWithIgnoreCase("https://")){
            if(address.containsAnyOf(" \t")||!address.containsChar('.'))address="https://www.google.com/search?q="+juce::URL::addEscapeChars(address,true);
            else address="https://"+address;
        }
        proc.lastUrl=address;if(!proc.session.running())proc.ensureBrowser();else proc.session.command(1,address);url.setText(address,false);};
    startTimerHz(2);
}
void Editor::resized(){
    browser.setBounds(4,47,getWidth()-8,getHeight()-116);
    const int bottom=getHeight()-60;
    home.setBounds(10,bottom+3,32,36);
    forward.setBounds(44,bottom+3,32,36);
    reload.setBounds(78,bottom+3,32,36);
    url.setBounds(126,bottom+5,getWidth()-182,32);
    start.setBounds(getWidth()-42,bottom+3,32,36);
}
void Editor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff10151e));g.setColour(juce::Colour(0xff515866));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1),24,2);
    g.setColour(juce::Colours::black);g.fillRoundedRectangle((float)getWidth()/2-44,8,88,26,13);
    g.setColour(juce::Colour(0xff172733));g.fillEllipse((float)getWidth()/2+25,16,9,9);
    g.setColour(juce::Colours::white);g.setFont(13);g.drawText(juce::Time::getCurrentTime().formatted("%H:%M"),17,10,55,23,juce::Justification::centredLeft);
    const float x=(float)getWidth()-91;const int net=pc.internet==0?0:pc.network.load();
    if(net==2||net==0){
        for(float r:{7.0f,12.0f}){juce::Path arc;arc.addCentredArc(x+9,29,r,r,0,-0.8f,0.8f,true);g.strokePath(arc,juce::PathStrokeType(1.8f));}g.fillEllipse(x+7,27,4,4);
        if(net==0){g.setColour(juce::Colour(0xff10151e));g.fillEllipse(x+15,22,13,13);g.setColour(juce::Colours::white);g.drawLine(x+18,25,x+24,31,1.7f);g.drawLine(x+24,25,x+18,31,1.7f);}
    }else{g.setFont(11);g.drawText(net==1?"LAN":(net==0?"OFF":"?"),(int)x-2,12,28,20,juce::Justification::centred);}
    const int bat=pc.battery;
    if(bat==-2){g.setFont(12);g.drawText(pc.power==1?"AC":"--",getWidth()-57,12,43,20,juce::Justification::centred);}
    else{
        g.drawRoundedRectangle((float)getWidth()-59,15,35,15,3,1);g.fillRect((float)getWidth()-23,19.0f,2.0f,7.0f);
        if(bat>=0){g.setColour(pc.charging?juce::Colour(0xff86e7bb):(bat<20?juce::Colours::salmon:juce::Colours::white));g.fillRoundedRectangle((float)getWidth()-57,17,31.0f*bat/100,11,1);g.setColour(bat>55?juce::Colours::black:juce::Colours::white);}
        g.setFont(10);g.drawText(bat>=0?juce::String(bat):"?",getWidth()-57,16,31,12,juce::Justification::centred);
    }
    if(connection.offline){
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        if(offlineImage.isValid())g.drawImageWithin(offlineImage,20,60,getWidth()-40,getHeight()-175,juce::RectanglePlacement::centred|juce::RectanglePlacement::onlyReduceInSize);
        g.setColour(juce::Colours::white);g.setFont(17);
        g.drawFittedText(juce::String::fromUTF8("Se requiere conexión a internet para usar este VST"),20,getHeight()-103,getWidth()-40,72,juce::Justification::centred,3);
        return;
    }
    g.setColour(juce::Colour(0xffaeb4bf));g.fillRoundedRectangle((float)getWidth()/2-44,(float)getHeight()-9,88,3,2);
    g.setColour(juce::Colour(0xfffdfdfe));g.fillRoundedRectangle(114,(float)getHeight()-60,(float)getWidth()-160,42,16);
}
void Editor::timerCallback(){
    const auto change=connection.update(pc.network.load(),pc.internet.load());
    if(change!=ConnectivityState::none){
        const bool show=!connection.offline;
        browser.setVisible(show);url.setVisible(show);home.setVisible(show);forward.setVisible(show);reload.setVisible(show);start.setVisible(show);
        if(change==ConnectivityState::restored){proc.lastUrl="webcapture://home";if(!proc.session.running())proc.ensureBrowser();else proc.session.command(1,proc.lastUrl);url.setText({},false);}
    }const auto current=proc.session.address();if(change!=ConnectivityState::restored&&current.isNotEmpty()&&current!="about:blank"){proc.lastUrl=current;if(!url.hasKeyboardFocus(true))url.setText(current=="webcapture://home"?juce::String():current,false);}auto text=proc.session.status();text+="\n";text+=pc.network==2?"Wi-Fi conectado":(pc.network==1?"Ethernet conectado":(pc.network==0?"Sin conexion de red":"Red no disponible"));text+=" | ";text+=pc.battery==-2?"PC sin bateria":(pc.battery>=0?"Bateria: "+juce::String(pc.battery.load())+"%":"Bateria no disponible");url.setTooltip(text);repaint();}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Processor();}
