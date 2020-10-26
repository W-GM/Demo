
#include <iostream>
#include "Config.h"

//#define SINGLE

using namespace gcfg;
using namespace std;

const char *ConfigFilePath = "./config/myconfig.json";

typedef struct ConfigData
{
    string Ver;
    string RtuName;

    int NetPort;
    string NetIp;
    string NetGateway;
    string NetMask;

    ZIGCFG ZigCfg;

    int DiIndex;
    int DiReg;
    vector<int>DiRegs;

    int DoIndex;
    int DoReg;
    vector<int>DoRegs;
    //TODO：设置DO的状态

    int AiIndex;
    int AiReg;
    int AiRng;
    vector<int>AiRegs;
    vector<int>AiRngs;

    int EthIndx;
    EthCFG EthCfg;
    vector<EthCFG>EthCfgs;

    WifiCFG WifiCfg;

    int SerialIndex;
    SerialCFG SerialCfg;
    vector<SerialCFG>SerialCfgs;

    int ManifoldIndex;
    ManiFold_S ManifoldCfg;
    vector<ManiFold_S> ManifoldCfgS;

    int WellportIndex;
    int WellportCfg;
    vector<int>WellportCfgs;

}Cdata;

int main(int argc, char const *argv[])
{
    int regs[] = {8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    int rngs[] = {6, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int wellports[] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    EthCFG EthCFG0, EthCFG1;
    SerialCFG SerialCFG0, SerialCFG1, SerialCFG2;
    ManiFold_S ManiFold_S0, ManiFold_S1;
    Cdata data;

    data.Ver = "1.1.1";
    data.RtuName = "281";

    data.NetPort = 502;
    data.NetIp = "11.11.11.22";
    data.NetGateway = "11.11.11.254";
    data.NetMask = "255.255.255.0";

    data.ZigCfg.ao = "1";
    data.ZigCfg.ce = "1";
    data.ZigCfg.id = "1010";
    data.ZigCfg.sc = "7fff";

    data.DiIndex = 0;
    data.DiReg = 255;
    for(int i = 0; i < 8; i++)
    {
        data.DiRegs.push_back(regs[i]);
    }

    data.DoIndex = 0;
    data.DoReg = 255;
    for(int i = 0; i < 8; i++)
    {
        data.DoRegs.push_back(regs[i]);
    }

    data.AiIndex = 0;
    data.AiReg = 255;
    data.AiRng = 6;
    for(int i = 0; i < 10; i++)
    {
        data.AiRegs.push_back(regs[i]);
        data.AiRngs.push_back(rngs[i]);
    }

    EthCFG0.ip = "11.11.11.2";
    EthCFG0.gateway = "11.11.11.254";
    EthCFG0.mask = "255.255.255.0";
    EthCFG1.ip = "11.11.11.22";
    EthCFG1.gateway = "11.11.11.254";
    EthCFG1.mask = "255.255.255.0";
    data.EthIndx = 0;
    data.EthCfg = EthCFG0;
    data.EthCfgs.push_back(EthCFG0);
    data.EthCfgs.push_back(EthCFG1);

    data.WifiCfg.ssid = "root";
    data.WifiCfg.passwd = "123456";

    SerialCFG0.baudrate = "115200";
    SerialCFG0.parity = "None"; /* None:无校验；Odd:奇校验；Even:偶校验 */ 
    SerialCFG0.databit = 8;
    SerialCFG0.stopbit = 1;
    SerialCFG1.baudrate = "9600";
    SerialCFG1.parity = "Odd"; /* None:无校验；Odd:奇校验；Even:偶校验 */ 
    SerialCFG1.databit = 8;
    SerialCFG1.stopbit = 1;
    SerialCFG2.baudrate = "9600";
    SerialCFG2.parity = "Even"; /* None:无校验；Odd:奇校验；Even:偶校验 */ 
    SerialCFG2.databit = 8;
    SerialCFG2.stopbit = 1;
    data.SerialIndex = 0;
    data.SerialCfg = SerialCFG0;
    data.SerialCfgs.push_back(SerialCFG0);
    data.SerialCfgs.push_back(SerialCFG1);
    data.SerialCfgs.push_back(SerialCFG2);

    ManiFold_S0.cfg = 1000;
    ManiFold_S0.rng = 6;
    ManiFold_S1.cfg = 2506;
    ManiFold_S1.rng = 0;
    data.ManifoldIndex = 0;
    data.ManifoldCfg = ManiFold_S0;
    data.ManifoldCfgS.push_back(ManiFold_S0);
    data.ManifoldCfgS.push_back(ManiFold_S1);

    data.WellportIndex = 0;
    data.WellportCfg = 1;
    for(int i = 0; i < 20; i++)
    {
        data.WellportCfgs.push_back(wellports[i]);
    }

    ConfigSingle * config = ConfigSingle::getInstance();

    if(config == nullptr)
    {
        cout << "create config error" << endl;
        return -1;
    }

    /* 打开config文件 */
    if(config->RefreshCfg(ConfigFilePath) == -1)
    {
        return -1;
    }

    config->SetVersion(data.Ver);
    config->SetRtuName(data.RtuName);
    cout << endl;
    cout << "version>> " << config->GetVersion().c_str() << endl;
    cout << "rtuname>> " << config->GetRtuName() << endl;

    config->SetNetPort(data.NetPort);
    config->SetNetIP(data.NetIp);
    config->SetNetGateway(data.NetGateway);
    config->SetNetMask(data.NetMask);
    cout << endl;
    cout << "netport>> " << config->GetNetPort() << endl;
    cout << "netip>> " << config->GetNetIP() << endl;
    cout << "netgateway>> " << config->GetNetGateway() << endl;
    cout << "macaddr>> " << config->GetMacAddr() << endl;
    cout << "netmask>> " << config->GetNetMask() << endl;
    
    config->SetZIGCFG(data.ZigCfg);
    cout << endl;
    cout << "zigbee>> " << endl;
    cout << "ao: " << config->GetZIGCFG().ao << endl;
    cout << "ce: " << config->GetZIGCFG().ce << endl;
    cout << "id: " << config->GetZIGCFG().id << endl;
    cout << "sc: " << config->GetZIGCFG().sc << endl;

#ifdef SINGLE
    config->SetDIReg(data.DiIndex, data.DiReg);
#else
    config->SetDIRegs(data.DiRegs);
#endif
    cout << endl;
    cout << "di>> ";
    for(size_t i = 0; i < config->GetDIRegs().size(); i++)
    {
        cout << config->GetDIRegs()[i] << " ";
    }
    cout << endl;
    cout << "di status>> " << config->GetDIStatus() << endl;

#ifdef SINGLE
    config->SetDOReg(data.DoIndex, data.DoReg);
#else
    config->SetDORegs(data.DoRegs);
#endif
    cout << endl;
    cout << "do>> ";
    for(size_t i = 0; i < config->GetDORegs().size(); i++)
    {
        cout << config->GetDORegs()[i] << " ";
    }
    cout << endl;
    cout << "do status>> " << config->GetDOStatus() << endl;

#ifdef SINGLE
    config->SetAIReg(data.AiIndex, data.AiReg);
    config->SetAIRng(data.AiIndex, data.AiRng);
#else
    config->SetAIRegs(data.AiRegs);
    config->SetAIRngs(data.AiRngs);
#endif
    cout << endl;
    cout << "ai>> ";
    for(size_t i = 0; i < config->GetAIRegs().size(); i++)
    {
        cout << config->GetAIRegs()[i] << " ";
    }
    cout << endl;
    cout << "ai-rng>> ";
    for(size_t i = 0; i < config->GetAIRngs().size(); i++)
    {
        cout << config->GetAIRngs()[i] << " ";
    }
    cout << endl;
    cout << "ai values>> ";
    for(size_t i = 0; i < config->GetAIValues().size(); i++)
    {
        cout << config->GetAIValues()[i] << " ";
    }
    cout << endl;

#ifdef SINGLE
    config->SetEth(data.EthIndx, data.EthCfg);
#else
    config->SetEths(data.EthCfgs);
#endif
    cout << endl;
    cout << "eth0>> " << endl;
    cout << "ip: " << config->GetEth(0).ip << endl;
    cout << "mask: " << config->GetEth(0).mask << endl;
    cout << "gateway: " << config->GetEth(0).gateway << endl;
    
    cout << "eth1>> " << endl;
    cout << "ip: " << config->GetEth(1).ip << endl;
    cout << "mask: " << config->GetEth(1).mask << endl;
    cout << "gateway: " << config->GetEth(1).gateway << endl;

    config->SetWifi(data.WifiCfg);
    cout << endl;
    cout << "wifi>> " << endl;
    cout << "ssid: " << config->GetWifi().ssid << endl;
    cout << "passwd: " << config->GetWifi().passwd << endl;

#ifdef SINGLE
    config->SetSerialCfg(data.SerialIndex, data.SerialCfg);
#else
    config->SetSerialCfgs(data.SerialCfgs);
#endif
    cout << endl;
    cout << "232>> " << endl;
    cout << "baudrate: " << config->GetSerialCfgs()[0].baudrate << endl;
    cout << "databit: " << config->GetSerialCfgs()[0].databit << endl;
    cout << "parity: " << config->GetSerialCfgs()[0].parity << endl;
    cout << "stopbit: " << config->GetSerialCfgs()[0].stopbit << endl;

    cout << endl;
    cout << "485-0>> " << endl;
    cout << "baudrate: " << config->GetSerialCfgs()[1].baudrate << endl;
    cout << "databit: " << config->GetSerialCfgs()[1].databit << endl;
    cout << "parity: " << config->GetSerialCfgs()[1].parity << endl;
    cout << "stopbit: " << config->GetSerialCfgs()[1].stopbit << endl;

    cout << endl;
    cout << "485-1>> " << endl;
    cout << "baudrate: " << config->GetSerialCfgs()[2].baudrate << endl;
    cout << "databit: " << config->GetSerialCfgs()[2].databit << endl;
    cout << "parity: " << config->GetSerialCfgs()[2].parity << endl;
    cout << "stopbit: " << config->GetSerialCfgs()[2].stopbit << endl;

#ifdef SINGLE
    config->SetManiFoldCfg(data.ManifoldIndex, data.ManifoldCfg);
#else
    config->SetManiFoldCfgs(data.ManifoldCfgS);
#endif
    cout << endl;
    cout << "Manifold-0>> " << endl;
    cout << "cfg: " << config->GetManiFoldCfgs()[0].cfg << endl;
    cout << "rng: " << config->GetManiFoldCfgs()[0].rng << endl;
    
    cout << endl;
    cout << "Manifold-1>> " << endl;
    cout << "cfg: " << config->GetManiFoldCfgs()[1].cfg << endl;
    cout << "rng: " << config->GetManiFoldCfgs()[1].rng << endl;

#ifdef SINGLE
    config->SetWellPortCfg(data.WellportIndex, data.WellportCfg);
#else
    config->SetWellPortCfgs(data.WellportCfgs);
#endif
    cout << "wellport>> ";
    for(size_t i = 0; i < config->GetWellPortCfgs().size(); i++)
    {
        cout << config->GetWellPortCfgs()[i] << " ";
    }
    cout << endl;

    config->SaveCfg(ConfigFilePath);

    return 0;
}
