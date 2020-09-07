/**
 * @file multitask.h
 * @author wgm (wangguomin@scaszh.com)
 * @brief This is the CQOF wellsite RTU multi-threaded head file
 * @version 1.1
 * @date 2020-07-03
 *
 * @copyright Copyright (c) 2020
 *
 */
#ifndef __MULTITASK_H__
#define __MULTITASK_H__

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "library/modbus.h"
#include "xbee_op.h"
#include "uart.h"
#include "sqlite_helper.h"
#include <vector>
#include <shared_mutex>
#include "i2c.h"
#include "spi.h"
#include <algorithm> /* 用于实现string中的某个字符替换成新的字符 */

#include "config/tinyxml2.h"

#include "modbus-private.h"
#include "rs485.h"
using namespace tinyxml2;

/* 允许监听的最大个数 */
#define NB_CONNECTON 5

#define ARMCQ

//#define ANTE_AI

/**
 * @brief 厂家名称
 * 
 */
enum manufactures
{
    ANKONG = 100,  /* 安控 */
    KAISHAN = 200, /* 凯山 */
    ANTE = 300,    /* 安特 */
    JINGSHI = 400  /* 金时 */
};

/**
 * @brief 通讯连接方式
 *
 */
enum communication_connect_type
{
    CON_NO,       /* 无连接 */
    CON_WIRED,    /* 有线连接 */
    CON_WIRELESS, /* 无线连接 */
};

/**
 * @brief 状态机的状态表
 *
 */
enum state_machine
{
    PHASE_START,             /* 起始阶段 */
    PHASE_OIL_WELL,          /* 油井数据 */
    PHASE_WATER_WELL,        /* 水源井数据 */
    PHASE_VALVE_GROUP,       /* 阀组间数据 */
    PHASE_MANIFOLD_PRESSURE, /* 汇管压力数据 */
    PHASE_WELLSITE_RTU       /* 井场RTU数据 */
};

/**
 * @brief 当前所存储的数据类型表
 *
 */
enum stroe_type
{
    TYPE_NO,                    /* 未配置类型 */
    TYPE_OIL_WELL_DATA,         /* 油井数据类型 */
    TYPE_WATER_WELL_DATA,       /* 水源井数据类型 */
    TYPE_VALVE_GROUP_DATA,      /* 阀组间数据类型 */
    TYPE_MANIFOLD_PRESSURE_1,   /* 汇管压力1(AI)数据类型 */
    TYPE_MANIFOLD_PRESSURE_2,   /* 汇管压力2(AI)数据类型 */
    TYPE_INDICATION_DIAGRAM,    /* 功图数据类型 */
    TYPE_POWER_DIAGRAM          /* 功率图数据类型 */
};

/**
 * @brief 初始化错误类型
 *
 */
enum error_mun
{
    ERROR_NO,     /* 未出现错误 */
    ERROR_CONFIG, /* 读取配置项失败 */
    ERROR_XBEE,   /* 初始化xbee失败 */
    ERROR_I2C,    /* 初始化i2c失败 */
    ERROR_SPI,    /* 初始化spi失败 */
    ERROR_485,    /* 初始化485失败 */
    ERROR_SQL,    /* 初始化数据库失败 */

    ERROR_XBEE_RECV_TIMEOUT, /* zigbee接收超时 */
    ERROR_XBEE_RECV_DATA     /* zigbee接收数据错误 */
};

/**
 * @brief 用于保存状态机的当前状态信息
 *
 */
struct state_machine_current_info
{
    bool     is_add_id;            /* 是否对id进行++操作 */
    int      id_addr;              /* 当前站号所在的地址，包含存储块和配置项 */
    int      id;                   /* 当前站号(ID) */
    int      id_ind;
    int      id_indicator_diagram; /* 当前功图对应的油井编号(ID) */
    uint8_t  func_code;            /* 当前功能码 */
    uint16_t start_addr;           /* 当前寄存器起始地址 */
    uint16_t addr_len;             /* 当前地址长度 */
    bool     isGetTime;            /* 当前GetTime的状态 */
    uint16_t phase;                /* 当前状态机所处的阶段 */
    //string   time;                 /* 当前使能￼时间 */
    int      store_type;           /* 当前所存储的数据类型 */
    bool     is_again;             /* 判断当前是否开始请求下一口井的数据 */
};

/**
 * @brief 用于存储一个站号的一组基础数据
 *
 */
struct data_block
{
    uint8_t              id;                 /* 站号 */
    std::vector<uint16_t>oil_basic_data;     /* 油井基础数据 */
    std::vector<uint16_t>ind_diagram;        /* 功图数据(包含40201~40210) */
    std::vector<uint16_t>power_diagram;      /* 功率图数据(包含41411~41420) */
    std::vector<uint16_t>water_well_data;    /* 水源井数据 */
    std::vector<uint16_t>valve_group_data;   /* 阀组间原始数据 */
    std::vector<uint16_t>wellsite_rtu;       /* 井场RTU的数据 */
    uint8_t              injection_well_num; /* 配置的注水井个数 */

    time_t cur_time_diagram;                 /* 存储功图的当前时间 */
};

/**
 * @brief 用于存储上位机发送来的数据
 *
 */
struct tcp_data
{
    uint8_t  frame_header[6]; /* 帧头 */
    uint8_t  id;              /* 站号 */
    uint8_t  func_code;       /* 功能码 */
    uint8_t byte_count;       /* 字节个数（功能码0x10, 0x0f） */
    //uint8_t coils_data[20];   /* 线圈输出值 (功能码0x0f) */
    uint16_t start_addr;      /* 寄存器起始地址 */
    uint16_t len;             /* 寄存器的长度/个数 */
    uint16_t set_val;         /* 向寄存器中写入的值（功能码0x06） */
    uint16_t value[20];       /* 向多个寄存器中写入的值（功能码0x10, 0x0f时只当做单字节使用） */
};

/**
 * @brief 配置信息
 *
 */
struct config_info
{
    string version;  /* 固件版本 */
    string rtu_name; /* rtu名称 */

    uint16_t port;     /* 端口号 */
    string ip;       /* ip地址 */
    string gateway;  /* 网关 */
    string mac;      /* mac地址 */
    string mask;     /* 子网掩码 */

    /* 汇管的配置信息 */
    struct manifold_pressure
    {
        uint8_t type;   /* 0:未配置 1:AI配置 2:ZigBee配置 */
        uint8_t add;    /* 1～6对应连接到AI的第几个引脚上 */
        uint8_t id;     /* 配置在第几个站号上 */
        uint16_t range; /* 量程 */
    } manifold_1, manifold_2;

    string   xbee_id;   /* 16位的PAN ID */
    string   xbee_sc;   /* 7fff */
    uint16_t xbee_ao;   /* 0:<不接收ack>  1:<接收ack> */
    uint16_t xbee_ce;   /* 0:<路由器>  1:<协调器> */

    /* 井口的配置信息 */
    struct well_info
    {
        uint8_t  id;   /* 已配置的站号 */
        uint8_t  type; /* 类型 0:<未配置> 1:<油井> 2:<水井> 0x31:<阀组(485)>
                          0x32<阀组(zigbee)> */
        uint64_t addr; /* 64位设备地址 */
    } well_info[20];

    /* 配置的井口和阀组的个数 */
    int well_max_num;

    /* 自定义的与上位机间通信的配置寄存器信息，对应站号为222 */
    uint16_t reg_value[100];
};


// TODO : 这里以后可以把指针更换成智能指针
class MultiTask {
private:

    /* modbus handler */
    modbus_t *ctx = nullptr;

    /* 用于保存在构造函数中，是否出现错误 */
    int is_error = ERROR_NO;

    /* 用于监听的 socket */
    int server_socket;

    /* 用于保存状态机的当前状态信息 */
    struct state_machine_current_info current_info;

    /* 用于配置DI AI DO */
    struct DI_AI_DO
    {
        /* true表示在井场使用了，false表示未使用 */
        bool    en_di;
        bool    en_do;
        bool    en_ai;
        uint8_t val_di[2];
        uint8_t val_do;
        uint8_t val_ai;
    } DI_AI_DO;

    struct rs485_info
    {
        int            fd;
        struct termios oldtio;
    } rs485_info;

    /* 数据库表名 */
    struct sql_tab_name
    {
        char tab_basic[10];                   /* 油井基础数据表名 */
        char tab_indicator_diagram_basic[22]; /* 功图基础数据表名 */
        char tab_indicator_diagram[17];       /* 功图数据表名 */
        char tab_water_well[10];              /* 水源井数据表名 */
        char tab_valve_group[11];             /* 阀组间数据表名 */
    } sql_tab_name;

    /**
     * @brief 井场RTU信息
     *
     */
    struct wellsite_info
    {
        std::vector<uint16_t>wellsite_info;           /* 井场信息(暂未定) */
        std::vector<uint16_t>manufacturer;            /* 厂家信息(暂未定) */
        std::vector<uint16_t>rtu_version;             /* 设备型号(暂未定) */
        uint16_t             manifold_pressure[2];    /* 汇管压力数据 */
        uint16_t             fault_info[5];           /* 故障信息 */
        uint16_t             infrared_alarm[2];       /* 红外报警 */
    } wellsite_info;


    /* 用于存储上位机发送来数据 */
    struct tcp_data tcp_data;

    /* 保存配置在某一口井上的汇管压力对应的ID再对应的存储块的ID */
    // int to_id_manifold_1 = 0;
    // int to_id_manifold_2 = 0;

    /* 发送给 zigbee 的数据帧(modbus RTU) */
    struct to_zigbee
    {
        uint8_t data[8];
        uint8_t len;
    } to_zigbee;

    /* 从ZigBee接收到的数据 */
    struct from_zigbee
    {
        uint8_t data[MODBUS_MAX_ADU_LENGTH];
        uint8_t len;
    } from_zigbee;

    /* 服务器的IP地址和端口号 */
    struct server_info
    {
        char ip_addr[16];
        int  port;
    } server_info;

    /* 用于指向存储站号的两组数据区 */
    struct data_block *data_block = nullptr;
    struct data_block *data_block_2 = nullptr;

    /* 用于指向分配的两个data_block数据区 */
    struct to_data_block
    {
        struct data_block *wr_db; /* 指向写基础数据指针 */
        struct data_block *rd_db; /* 指向写基础数据指针 */
        struct data_block *wr_db_ind; /* 指向写功图数据指针 */
        struct data_block *rd_db_ind; /* 指向读功图数据指针 */
    } to_db;

    /* xbee handler */
    XBee *xbee_handler = nullptr;

    /* xbee serial handler */
    uart *xbee_ser = nullptr;

    /* sqlite helper handler */
    sqlControl *sql_handler = nullptr;

    /* i2c handler */
    i2c_handler *i2c_D = nullptr;

    /* spi handler */
    struct spi_handler spi_ai;

    /* 用于在电脑上运行rs485 */
    uart *uart_485 = nullptr;


    /* ----------------------配置项---------------------- */
    /* configure */
    XMLDocument config;                  /* 定义配置对象 */
    XMLElement *config_label = nullptr;  /* 指向配置文件中的标签 */
    XMLElement *config_option = nullptr; /* 指向标签中的配置 */

    /* 用于保存从配置项文件中得到的配置信息 */
    struct config_info config_info;

    /*---------------------互斥量和条件变量---------------*/

    /* 用于管理xbee Tx 与 Rx 的互斥量 */
    std::mutex m_tx_rx;

    /* 目前用于给data_block上锁 */
    std::mutex m_data_block;

    /* 用于给井口基础数据上锁 */
    std::mutex m_basic_data;

    /* 用于给功图数据上锁 */
    std::mutex m_diagram_data;

    /* 用于给读数据区指针上锁 */
    std::mutex m_rd_db;

    /* 用于给井场RTU信息上锁 */
    std::mutex m_wellsite;

    /* 用于判断接收的井口基础数据是否准备好 */
    std::condition_variable cv_basic_data;

    /* 用于判断接收的井口功图数据是否准备好 */
    std::condition_variable cv_diagram_data;

    /* 用于判断当前是否开始请求下一口井的数据的条件变量 */
    std::condition_variable cv_cur_info;



public:

    explicit MultiTask();

    virtual ~MultiTask();

    /* 初始化线程 */
    void thread_init();

    /* 获取井口数据线程 */
    void thread_get_wellport_info();

    /* 处理上位机请求线程 */
    void thread_host_request();

    /* 升级线程 */
    void thread_update();

    /* 数据库存储线程 */
    void thread_sql_memory();

    /* 看门狗控制线程 */
    void thread_watchdogctl();

    int  get_error()
    {
        return is_error;
    }

    struct config_info get_config() const
    {
        return config_info;
    }

    int get_tcp_config(const char * value, char * get_value, int addr, int time);
    


    int                config_manage();

    int                rs485(struct state_machine_current_info *curr_info,
                             struct data_block                 *data_block);

    int                to_xbee(struct tcp_data *tcp_data);

    int                sel_data_to_tcp(struct tcp_data *tcp_data);

    int                state_machine_operation(
        struct state_machine_current_info *curr_info,
        struct data_block                 *data_block);
};

#endif // ifndef __MULTITASK_H__
