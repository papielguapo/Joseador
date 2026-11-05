#include "Browser.h"
#include "SystemStatus.h"
class Processor;
class HomeButton final:public juce::Button {
public:
    HomeButton():Button("Atras"){setTooltip("Volver atras hasta Inicio");}
    void paintButton(juce::Graphics& g,bool over,bool down)override{
        if(over||down){g.setColour(juce::Colour(down?0xffd4d7dd:0xffedf0f4));g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(1),9);}
        const float w=(float)getWidth(),h=(float)getHeight(),left=4,right=w-4,top=h*.27f,bottom=h*.73f,tip=h*.5f;
        juce::Path outline;outline.startNewSubPath(left,tip);outline.lineTo(left+9,top);outline.lineTo(right-2,top);outline.quadraticTo(right,top,right,top+2);outline.lineTo(right,bottom-2);outline.quadraticTo(right,bottom,right-2,bottom);outline.lineTo(left+9,bottom);outline.closeSubPath();
        g.setColour(juce::Colour(0xff171a20));g.strokePath(outline,juce::PathStrokeType(1.8f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        const float cx=left+15,cy=h*.5f;g.drawLine(cx-3,cy-3,cx+3,cy+3,1.6f);g.drawLine(cx+3,cy-3,cx-3,cy+3,1.6f);
    }
};
class Editor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit Editor(Processor&);
    void resized()override;
    void paint(juce::Graphics& g)override;
private:
    Processor& proc;BrowserView browser;juce::TextEditor url;
    HomeButton home;
    juce::TextButton clear{juce::String::fromUTF8("\xc3\x97")};
    SystemStatus pc;
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
    void ensureBrowser(){if(!session.data()){session.start(runtime());session.command(1,lastUrl);}}
    const juce::String getName()const override{return "Joseador";}
    void prepareToPlay(double sr,int)override{session.rate=sr;}
    void releaseResources()override{}
    bool isBusesLayoutSupported(const BusesLayout& l)const override{return l.getMainInputChannelSet().isDisabled()&&l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
    void processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi)override{juce::ScopedNoDenormals scoped;session.pull(b);midi.clear();}
    bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}
    double getTailLengthSeconds()const override{return std::numeric_limits<double>::infinity();}bool hasEditor()const override{return true;}
    juce::AudioProcessorEditor* createEditor()override{ensureBrowser();return new Editor(*this);}
    int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}
    const juce::String getProgramName(int)override{return{};}void changeProgramName(int,const juce::String&)override{}
    void getStateInformation(juce::MemoryBlock& block)override{juce::XmlElement x("WebCaptureMobile");x.setAttribute("url",lastUrl);copyXmlToBinary(x,block);}
    void setStateInformation(const void* bytes,int n)override{if(auto x=getXmlFromBinary(bytes,n))if(x->hasTagName("WebCaptureMobile"))lastUrl=x->getStringAttribute("url",lastUrl);}
};
Editor::Editor(Processor& p):AudioProcessorEditor(p),proc(p),browser(p.session){
    setResizable(true,true);setResizeLimits(320,400,1200,1400);setSize(400,680);
    for(auto* c:std::initializer_list<juce::Component*>{&browser,&url,&home,&clear})addAndMakeVisible(c);
    home.setTitle("Atras / Inicio");
    clear.setTitle("Borrar busqueda");clear.setTooltip("Borrar");clear.onClick=[this]{url.clear();url.grabKeyboardFocus();};
    clear.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff96979b));
    clear.setColour(juce::TextButton::textColourOffId,juce::Colours::white);
    url.setText(proc.lastUrl=="webcapture://home"?juce::String():proc.lastUrl);url.setSelectAllWhenFocused(true);
    home.onClick=[this]{proc.session.command(2);};
    url.setTextToShowWhenEmpty("Buscar o escribir direccion",juce::Colours::grey);
    url.setFont(juce::Font(16));
    url.setBorder(juce::BorderSize<int>(0));
    url.setIndents(0,0);
    url.setJustification(juce::Justification::centredLeft);
    url.setColour(juce::TextEditor::backgroundColourId,juce::Colours::transparentBlack);
    url.setColour(juce::TextEditor::textColourId,juce::Colour(0xff171719));
    url.setColour(juce::TextEditor::outlineColourId,juce::Colours::transparentBlack);
    url.setColour(juce::TextEditor::focusedOutlineColourId,juce::Colours::transparentBlack);
    url.onReturnKey=[this]{auto address=url.getText().trim();if(address.isEmpty())return;
        if(!address.startsWithIgnoreCase("http://")&&!address.startsWithIgnoreCase("https://")){
            if(address.containsAnyOf(" \t")||!address.containsChar('.'))address="https://www.google.com/search?q="+juce::URL::addEscapeChars(address,true);
            else address="https://"+address;
        }
        proc.lastUrl=address;proc.session.command(1,address);url.setText(address,false);};
    startTimerHz(2);
}
void Editor::resized(){
    browser.setBounds(4,47,getWidth()-8,getHeight()-116);
    home.setBounds(14,getHeight()-57,30,36);
    url.setBounds(56,getHeight()-55,getWidth()-100,32);
    clear.setBounds(getWidth()-36,getHeight()-49,20,20);
}
void Editor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff10151e));g.setColour(juce::Colour(0xff515866));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1),24,2);
    g.setColour(juce::Colours::black);g.fillRoundedRectangle((float)getWidth()/2-44,8,88,26,13);
    g.setColour(juce::Colour(0xff172733));g.fillEllipse((float)getWidth()/2+25,16,9,9);
    g.setColour(juce::Colours::white);g.setFont(13);g.drawText(juce::Time::getCurrentTime().formatted("%H:%M"),17,10,55,23,juce::Justification::centredLeft);
    const float x=(float)getWidth()-91;const int net=pc.network;
    if(net==2){
        for(float r:{7.0f,12.0f}){juce::Path arc;arc.addCentredArc(x+9,29,r,r,0,-0.8f,0.8f,true);g.strokePath(arc,juce::PathStrokeType(1.8f));}g.fillEllipse(x+7,27,4,4);
    }else{g.setFont(11);g.drawText(net==1?"LAN":(net==0?"OFF":"?"),(int)x-2,12,28,20,juce::Justification::centred);}
    const int bat=pc.battery;
    if(bat==-2){g.setFont(12);g.drawText(pc.power==1?"AC":"--",getWidth()-57,12,43,20,juce::Justification::centred);}
    else{
        g.drawRoundedRectangle((float)getWidth()-59,15,35,15,3,1);g.fillRect((float)getWidth()-23,19.0f,2.0f,7.0f);
        if(bat>=0){g.setColour(pc.charging?juce::Colour(0xff86e7bb):(bat<20?juce::Colours::salmon:juce::Colours::white));g.fillRoundedRectangle((float)getWidth()-57,17,31.0f*bat/100,11,1);g.setColour(bat>55?juce::Colours::black:juce::Colours::white);}
        g.setFont(10);g.drawText(bat>=0?juce::String(bat):"?",getWidth()-57,16,31,12,juce::Justification::centred);
    }
    g.setColour(juce::Colour(0xffaeb4bf));g.fillRoundedRectangle((float)getWidth()/2-44,(float)getHeight()-9,88,3,2);
    g.setColour(juce::Colour(0xfffdfdfe));g.fillRoundedRectangle(8,(float)getHeight()-60,(float)getWidth()-16,42,16);
}
void Editor::timerCallback(){const auto current=proc.session.address();if(current.isNotEmpty()&&current!="about:blank"){proc.lastUrl=current;if(!url.hasKeyboardFocus(true))url.setText(current=="webcapture://home"?juce::String():current,false);}auto text=proc.session.status();text+="\n";text+=pc.network==2?"Wi-Fi conectado":(pc.network==1?"Ethernet conectado":(pc.network==0?"Sin conexion de red":"Red no disponible"));text+=" | ";text+=pc.battery==-2?"PC sin bateria":(pc.battery>=0?"Bateria: "+juce::String(pc.battery.load())+"%":"Bateria no disponible");url.setTooltip(text);repaint();}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Processor();}

