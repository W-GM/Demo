#include "Config.h"
#include <iostream>

namespace gcfg {
/**
 * @brief Construct a new Config Single:: Config Single object
 * open the config file
 *
 */
#if 0
ConfigSingle::ConfigSingle()
{
    std::cout << "config class constructor" << std::endl;
}
#endif

/**
 * @brief get the version string
 *
 * @return const std::string
 * @note the return value is const
 */
const std::string ConfigSingle::GetVersion()
{
    std::string temp;

    oJson.Get("Version", temp);
    return temp;
}

/**
 * @brief set the version string
 *
 * @param ver the new version string
 */
void ConfigSingle::SetVersion(std::string ver)
{
    oJson.Replace("Version", ver);
}

/**
 * @brief get the rtu's name
 *
 * @return const std::string
 * @note the return value is const
 */
const std::string ConfigSingle::GetRtuName()
{
    std::string temp;

    oJson.Get("RtuName", temp);
    return temp;
}

/**
 * @brief set the rtu's name
 *
 * @param name
 */
void ConfigSingle::SetRtuName(std::string name)
{
    oJson.Replace("RtuName", name);
}

/**
 * @brief get the modbus server port
 *
 * @return unsigned int
 */
unsigned int ConfigSingle::GetNetPort()
{
    int temp;

    oJson.Get("NetPort", temp);
    return temp;
}

/**
 * @brief set the modubus server port
 *
 * @param port
 */
void ConfigSingle::SetNetPort(int port)
{
    oJson.Replace("NetPort", port);
}

/**
 * @brief
 *
 * @return const std::string
 */
const std::string ConfigSingle::GetNetIP()
{
    std::string temp;

    oJson.Get("NetIP", temp);
    return temp;
}

/**
 * @brief
 *
 * @param ip
 */
void ConfigSingle::SetNetIP(std::string ip)
{
    std::regex reg(
        "((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]).){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    bool ret = std::regex_match(ip, reg);

    assert(ret);
    oJson.Replace("NetIP", ip);
}

/**
 * @brief
 *
 * @return const std::string
 */
const std::string ConfigSingle::GetNetGateway()
{
    std::string temp;

    oJson.Get("NetGageway", temp);
    return temp;
}

/**
 * @brief
 *
 * @param gateway
 */
void ConfigSingle::SetNetGateway(std::string gateway)
{
    std::regex reg(
        "((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]).){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    bool ret = std::regex_match(gateway, reg);

    assert(ret);
    oJson.Replace("NetGageway", gateway);
}

/**
 * @brief
 *
 * @return const std::string
 */
const std::string ConfigSingle::GetMacAddr()
{
    std::string temp;

    oJson.Get("NetMac", temp);
    return temp;
}

/**
 * @brief
 *
 * @return const std::string
 */
const std::string ConfigSingle::GetNetMask()
{
    std::string temp;

    oJson.Get("NetMask", temp);
    return temp;
}

/**
 * @brief
 *
 * @param mask
 */
void ConfigSingle::SetNetMask(std::string mask)
{
    std::regex reg(
        "((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]).){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    bool ret = std::regex_match(mask, reg);

    assert(ret);
    oJson.Replace("NetMask", mask);
}

const ZIGCFG ConfigSingle::GetZIGCFG()
{
    ZIGCFG temp;

    oJson["ZIGBEE"].Get("XBEE_ID", temp.id);
    oJson["ZIGBEE"].Get("XBEE_SC", temp.sc);
    oJson["ZIGBEE"].Get("XBEE_AO", temp.ao);
    oJson["ZIGBEE"].Get("XBEE_CE", temp.ce);

    return temp;
}

void ConfigSingle::SetZIGCFG(ZIGCFG cfg)
{
    oJson["ZIGBEE"].Replace("XBEE_ID", cfg.id);
    oJson["ZIGBEE"].Replace("XBEE_SC", cfg.sc);
    oJson["ZIGBEE"].Replace("XBEE_AO", cfg.ao);
    oJson["ZIGBEE"].Replace("XBEE_CE", cfg.ce);
}

int ConfigSingle::GetDIReg(int index)
{
    int temp;

    oJson["DIREG"].Get(index, temp);
    return temp;
}

std::vector<int>ConfigSingle::GetDIRegs()
{
    int te;
    std::vector<int> temp;

    for (int i = 0; i < 8; i++)
    {
        oJson["DIREG"].Get(i, te);
        temp.push_back(te);
    }
    return temp;
}

void ConfigSingle::SetDIReg(int index, int reg)
{
    oJson["DIREG"].Replace(index, reg);
}

void ConfigSingle::SetDIRegs(std::vector<int>regs)
{
    assert(regs.size() == 8);

    for (int i = 0; i < 8; i++)
    {
        oJson["DIREG"].Replace(i, regs[i]);
    }
}

int ConfigSingle::GetDOReg(int index)
{
    int temp;

    oJson["DOREG"].Get(index, temp);
    return temp;
}

std::vector<int>ConfigSingle::GetDORegs()
{
    int te;
    std::vector<int> temp;

    for (int i = 0; i < 8; i++)
    {
        oJson["DOREG"].Get(i, te);
        temp.push_back(te);
    }
    return temp;
}

void ConfigSingle::SetDOReg(int index, int reg)
{
    oJson["DOREG"].Replace(index, reg);
}

void ConfigSingle::SetDORegs(std::vector<int>regs)
{
    assert(regs.size() == 8);

    for (int i = 0; i < 8; i++)
    {
        oJson["DOREG"].Replace(i, regs[i]);
    }
}

int ConfigSingle::GetAIReg(int index)
{
    int temp;

    oJson["AIREG"].Get(index, temp);
    return temp;
}

std::vector<int>ConfigSingle::GetAIRegs()
{
    int te;
    std::vector<int> temp;

    for (int i = 0; i < 10; i++)
    {
        oJson["AIREG"].Get(i, te);
        temp.push_back(te);
    }
    return temp;
}

void ConfigSingle::SetAIReg(int index, int reg)
{
    oJson["AIREG"].Replace(index, reg);
}

void ConfigSingle::SetAIRegs(std::vector<int>regs)
{
    assert(regs.size() == 10);

    for (int i = 0; i < 10; i++)
    {
        oJson["AIREG"].Replace(i, regs[i]);
    }
}

int ConfigSingle::GetAIRng(int index)
{
    int temp;

    oJson["AIRNG"].Get(index, temp);
    return temp;
}

std::vector<int>ConfigSingle::GetAIRngs()
{
    int te;
    std::vector<int> temp;

    for (int i = 0; i < 10; i++)
    {
        oJson["AIRNG"].Get(i, te);
        temp.push_back(te);
    }
    return temp;
}

void ConfigSingle::SetAIRng(int index, int reg)
{
    oJson["AIRNG"].Replace(index, reg);
}

void ConfigSingle::SetAIRngs(std::vector<int>regs)
{
    assert(regs.size() == 10);

    for (int i = 0; i < 10; i++)
    {
        oJson["AIRNG"].Replace(i, regs[i]);
    }
}

EthCFG ConfigSingle::GetEth(int index)
{
    EthCFG temp;

    oJson["Eth"][index].Get("ip", temp.ip);
    oJson["Eth"][index].Get("mask", temp.mask);
    oJson["Eth"][index].Get("gateway", temp.gateway);

    return temp;
}

std::vector<EthCFG>ConfigSingle::GetEths()
{
    std::vector<EthCFG> temp;
    std::string s;
    EthCFG t;

    for (int index = 0; index < 2; index++)
    {
        oJson["Eth"][index].Get("ip", s);
        t.ip = s;
        oJson["Eth"][index].Get("mask", s);
        t.mask = s;
        oJson["Eth"][index].Get("gateway", s);
        t.gateway = s;
        temp.push_back(t);
    }

    return temp;
}

void ConfigSingle::SetEth(int index, EthCFG cfg)
{
    std::regex reg(
        "((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]).){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    bool ret;

    ret = std::regex_match(cfg.ip, reg);
    assert(ret);
    ret = std::regex_match(cfg.mask, reg);
    assert(ret);
    ret = std::regex_match(cfg.gateway, reg);
    assert(ret);

    oJson["Eth"][index].Replace("ip", cfg.ip);
    oJson["Eth"][index].Replace("mask", cfg.mask);
    oJson["Eth"][index].Replace("gateway", cfg.gateway);
}

void ConfigSingle::SetEths(std::vector<EthCFG>cfgs)
{
    assert(cfgs.size() == 2);
    std::regex reg(
        "((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]).){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    bool ret;

    for (int i = 0; i < 2; i++)
    {
        ret = std::regex_match(cfgs[i].ip, reg);
        assert(ret);
        ret = std::regex_match(cfgs[i].mask, reg);
        assert(ret);
        ret = std::regex_match(cfgs[i].gateway, reg);
        assert(ret);

        oJson["Eth"][i].Replace("ip", cfgs[i].ip);
        oJson["Eth"][i].Replace("mask", cfgs[i].mask);
        oJson["Eth"][i].Replace("gateway", cfgs[i].gateway);
    }
}

WifiCFG ConfigSingle::GetWifi()
{
    WifiCFG temp;

    oJson["Wifi"].Get("ssid", temp.ssid);
    oJson["Wifi"].Get("passwd", temp.passwd);

    return temp;
}

void ConfigSingle::SetWifi(WifiCFG cfg)
{
    assert(cfg.passwd.size() >= 6);
    oJson["Wifi"].Replace("ssid", cfg.ssid);
    oJson["Wifi"].Replace("passwd", cfg.passwd);
}

/**
 * @brief
 *
 * @param index
 * @return SerialCFG
 */
SerialCFG ConfigSingle::GetSerialCfg(int index)
{
    SerialCFG temp;

    oJson["Serial"][index].Get("baudrate", temp.baudrate);
    oJson["Serial"][index].Get("parity", temp.parity);
    oJson["Serial"][index].Get("databit", temp.databit);
    oJson["Serial"][index].Get("stopbit", temp.stopbit);

    return temp;
}

std::vector<SerialCFG>ConfigSingle::GetSerialCfgs()
{
    std::vector<SerialCFG> temp;
    SerialCFG t;

    for (int index = 0; index < 3; index++)
    {
        oJson["Serial"][index].Get("baudrate", t.baudrate);
        oJson["Serial"][index].Get("parity", t.parity);
        oJson["Serial"][index].Get("databit", t.databit);
        oJson["Serial"][index].Get("stopbit", t.stopbit);
        temp.push_back(t);
    }
    return temp;
}

void ConfigSingle::SetSerialCfg(int index, SerialCFG cfg)
{
    oJson["Serial"][index].Replace("baudrate", cfg.baudrate);
    oJson["Serial"][index].Replace("parity", cfg.parity);
    oJson["Serial"][index].Replace("databit", cfg.databit);
    oJson["Serial"][index].Replace("stopbit", cfg.stopbit);
}

void ConfigSingle::SetSerialCfgs(std::vector<SerialCFG>cfgs)
{
    assert(cfgs.size() == 3);

    for (int index = 0; index < 3; index++)
    {
        oJson["Serial"][index].Replace("baudrate", cfgs[index].baudrate);
        oJson["Serial"][index].Replace("parity", cfgs[index].parity);
        oJson["Serial"][index].Replace("databit", cfgs[index].databit);
        oJson["Serial"][index].Replace("stopbit", cfgs[index].stopbit);
    }
}

char ConfigSingle::GetDIStatus()
{
    // TODO:需要从具体的芯片中读取
    return 0x5;
}

char ConfigSingle::GetDOStatus()
{
    // TODO:需要从具体的芯片中读取
    return 0x7;
}

void ConfigSingle::SetDOStatus(char status)
{
    // TODO:需要从具体的芯片中读取
}

std::vector<float>ConfigSingle::GetAIValues()
{
    // TODO:需要从具体的芯片中读取
    std::vector<float> temp;

    for (int i = 0; i < 10; i++)
    {
        temp.push_back((i + 1) * 0.56);
    }
    return temp;
}

MainFold_S ConfigSingle::GetMainFoldCfg(int index)
{
    MainFold_S temp;

    oJson["MainFold"][index].Get("MainFoldCfg", temp.cfg);
    oJson["MainFold"][index].Get("MainFoldRng", temp.rng);
    return temp;
}

std::vector<MainFold_S>ConfigSingle::GetMainFoldCfgs()
{
    std::vector<MainFold_S> temp;
    MainFold_S t;

    for (int i = 0; i < 2; i++)
    {
        oJson["MainFold"][i].Get("MainFoldCfg", t.cfg);
        oJson["MainFold"][i].Get("MainFoldRng", t.rng);
        temp.push_back(t);
    }
    return temp;
}

void ConfigSingle::SetMainFoldCfg(int index, MainFold_S p)
{
    oJson["MainFold"][index].Replace("MainFoldCfg", p.cfg);
    oJson["MainFold"][index].Replace("MainFoldRng", p.rng);
}

void ConfigSingle::SetMainFoldCfgs(std::vector<MainFold_S>cfgs)
{
    assert(cfgs.size() == 2);

    for (int i = 0; i < 2; i++)
    {
        oJson["MainFold"][i].Replace("MainFoldCfg", cfgs[i].cfg);
        oJson["MainFold"][i].Replace("MainFoldRng", cfgs[i].rng);
    }
}

int ConfigSingle::GetWellPortCfg(int index)
{
    int temp;

    oJson["WellPort"].Get(index, temp);
    return temp;
}

std::vector<int>ConfigSingle::GetWellPortCfgs()
{
    std::vector<int> temp;
    int t;

    for (int i = 0; i < 20; i++)
    {
        oJson["WellPort"].Get(i, t);
        temp.push_back(t);
    }
    return temp;
}

void ConfigSingle::SetWellPortCfg(int index, int cfg)
{
    oJson["WellPort"].Replace(index, cfg);
}

void ConfigSingle::SetWellPortCfgs(std::vector<int>cfgs)
{
    assert(cfgs.size() == 20);

    for (int i = 0; i < 20; i++)
    {
        oJson["WellPort"].Replace(i, cfgs[i]);
    }
}

/**
 * @brief 保存修改后的配置文件
 * 
 * @param path 配置文件路径及名称
 */
void ConfigSingle::SaveCfg(const char * path)
{
    std::ofstream out;

    out.open(path, std::ios::trunc);
    out.write(oJson.ToString().c_str(), oJson.ToString().size());
    out.flush();
    out.close();
}

/**
 * @brief 打开配置文件
 * 
 * @param path 配置文件路径及名称
 * @return int 成功返回0；失败返回-1
 */
int ConfigSingle::RefreshCfg(const char * path)
{
    std::ifstream in;

    in.open(path, std::ios::in);

    if (in.good())
    {
        std::cerr << "Open the config.json Success!" << std::endl;
        std::string str((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
        in.close();
        oJson.Parse(str);
        return 0;
    }
    else
    {
        std::cerr << "Open the config.json error!" << std::endl;
        return -1;
    }
}

#if 0
ConfigSingle::~ConfigSingle()
{
    std::cout << "config class deconstructor" << std::endl;
    std::ofstream out;

    out.open("myconfig.json", std::ios::trunc);
    out.write(oJson.ToString().c_str(), oJson.ToString().size());
    out.flush();
    out.close();
}
#endif
} // namespace gcfg
