#pragma once
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>

// Reads adapter and power state off the GUI/audio threads. No SSIDs, location
// access, network requests or system-setting changes are needed.
class SystemStatus final:private juce::Thread {
public:
    SystemStatus():Thread("PC status"){startThread();}
    ~SystemStatus()override{signalThreadShouldExit();notify();stopThread(-1);}
    std::atomic<int> network{-1},battery{-1},power{-1},charging{0};
    // network: -1 unknown, 0 offline, 1 ethernet, 2 Wi-Fi
    // battery: -2 no battery, -1 unknown, otherwise percentage
private:
    void run()override{
        while(!threadShouldExit()){
            SYSTEM_POWER_STATUS p{};
            if(GetSystemPowerStatus(&p)){
                power=p.ACLineStatus==255?-1:(int)p.ACLineStatus;
                battery=(p.BatteryFlag!=255&&(p.BatteryFlag&128))?-2:(p.BatteryLifePercent<=100?(int)p.BatteryLifePercent:-1);
                charging=p.BatteryFlag!=255&&(p.BatteryFlag&8);
            }
            PMIB_IF_TABLE2 table=nullptr;
            if(GetIfTable2(&table)==NO_ERROR){
                int state=0;
                for(ULONG i=0;i<table->NumEntries;++i){const auto& a=table->Table[i];
                    if(a.OperStatus!=IfOperStatusUp)continue;
                    if(a.Type==IF_TYPE_IEEE80211)state=2;
                    else if(a.Type==IF_TYPE_ETHERNET_CSMACD&&a.InterfaceAndOperStatusFlags.HardwareInterface&&state!=2)state=1;
                }
                network=state;FreeMibTable(table);
            }
            wait(2000);
        }
    }
};
