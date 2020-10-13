#pragma once
#include "CJsonObject.hpp"
#include <fstream>
#include <streambuf>
#include <vector>
#include <string>
#include <regex>
#include <assert.h>

typedef struct
{
    std::string id;
    std::string sc;
    std::string ao;
    std::string ce;
} ZIGCFG;

typedef struct
{
    bool         state;
    unsigned int type;
} PortCFG;

typedef struct
{
    std::string ip;
    std::string mask;
    std::string gateway;
} EthCFG;

typedef struct
{
    std::string ssid;
    std::string passwd;
} WifiCFG;


typedef struct
{
    std::string  baudrate;
    std::string  parity;
    unsigned int databit;
    unsigned int stopbit;
} SerialCFG;

typedef struct
{
    unsigned int cfg;
    unsigned int rng;
} MainFold_S;

namespace gcfg {
class ConfigSingle {
public:
    //ConfigSingle();

    static ConfigSingle* getInstance()
    {
        static ConfigSingle locla_s;

        return &locla_s;
    }

    void                   SaveCfg(const char * path);
    int                   RefreshCfg(const char * path);

    const std::string      GetVersion();
    void                   SetVersion(std::string ver);
    const std::string      GetRtuName();
    void                   SetRtuName(std::string name);
    unsigned int           GetNetPort();
    void                   SetNetPort(int port);
    const std::string      GetNetIP();
    void                   SetNetIP(std::string ip);
    const std::string      GetNetGateway();
    void                   SetNetGateway(std::string gateway);
    const std::string      GetMacAddr();
    const std::string      GetNetMask();
    void                   SetNetMask(std::string mask);
    const ZIGCFG           GetZIGCFG();
    void                   SetZIGCFG(ZIGCFG cfg);

    int                    GetDIReg(int index);
    std::vector<int>       GetDIRegs();
    void                   SetDIReg(int index,
                                    int reg);
    void                   SetDIRegs(std::vector<int>regs);

    int                    GetDOReg(int index);
    std::vector<int>       GetDORegs();
    void                   SetDOReg(int index,
                                    int reg);
    void                   SetDORegs(std::vector<int>regs);

    int                    GetAIReg(int index);
    std::vector<int>       GetAIRegs();
    void                   SetAIReg(int index,
                                    int reg);
    void                   SetAIRegs(std::vector<int>regs);

    int                    GetAIRng(int index);
    std::vector<int>       GetAIRngs();
    void                   SetAIRng(int index,
                                    int reg);
    void                   SetAIRngs(std::vector<int>regs);

    EthCFG                 GetEth(int index);
    std::vector<EthCFG>    GetEths();
    void                   SetEth(int    index,
                                  EthCFG cfg);
    void                   SetEths(std::vector<EthCFG>cfgs);

    WifiCFG                GetWifi();
    void                   SetWifi(WifiCFG cfg);

    SerialCFG              GetSerialCfg(int index);
    std::vector<SerialCFG> GetSerialCfgs();
    void                   SetSerialCfg(int       index,
                                        SerialCFG cfg);
    void                   SetSerialCfgs(std::vector<SerialCFG>cfgs);

    char                   GetDIStatus();
    char                   GetDOStatus();
    void                   SetDOStatus(char status);
    std::vector<float>     GetAIValues();

    MainFold_S             GetMainFoldCfg(int index);
    std::vector<MainFold_S>GetMainFoldCfgs();
    void                   SetMainFoldCfg(int        index,
                                          MainFold_S cfg);
    void                   SetMainFoldCfgs(std::vector<MainFold_S>cfgs);

    int                    GetWellPortCfg(int index);
    std::vector<int>       GetWellPortCfgs();
    void                   SetWellPortCfg(int index,
                                      int cfg);
    void                   SetWellPortCfgs(std::vector<int>cfgs);

private:

    neb::CJsonObject oJson;
    //~ConfigSingle();
};
} // namespace  gcfg
