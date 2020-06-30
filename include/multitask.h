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

#include "config/tinyxml2.h"

#include <stdio.h>
#include <termios.h>
#include <linux/ioctl.h>
#include <linux/serial.h>
#include <asm-generic/ioctls.h> /* TIOCGRS485 + TIOCSRS485 ioctl definitions */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <sys/ioctl.h>

/**
 * @brief: set the properties of serial port
 * @Param: fd: file descriptor
 * @Param: nSpeed: Baud Rate
 * @Param: nBits: character size
 * @Param: nEvent: parity of serial port
 * @Param: nStop: stop bits
 */

typedef enum { DISABLE = 0, ENABLE } RS485_ENABLE_t;


using namespace tinyxml2;


#define I2C

/* 允许监听的最大个数 */
#define NB_CONNECTON 5

/* 用于接收 zigbee 数据的长度 */
#define XBEE_RTU_MAX_LENGTH 80

/**
 * @brief 状态机的状态表
 *
 */
enum state_machine
{
    PHASE_START,                     /* 起始阶段 */
    PHASE_BASIC1,                    /* 基础数据第一部分 */
    PHASE_BASIC2,                    /* 基础数据第二部分 */
    PHASE_BASIC3,                    /* 基础数据第三部分 */
    PHASE_BASIC4,                    /* 基础数据第四部分 */
    PHASE_BASIC5,                    /* 基础数据第五部分 */
    PHASE_INDICATOR_DIAGRAM_BASIC_1, /* 功图基础数据第一部分 */
    PHASE_INDICATOR_DIAGRAM_BASIC_2, /* 功图基础数据第二部分 */
    PHASE_INDICATOR_DIAGRAM,         /* 功图 */
    PHASE_POWER_DIAGRAM,             /* 功率图 */
    PHASE_WATER_WELL,                /* 水源井数据 */
    PHASE_VALVE_GROUP                /* 阀组间数据 */
};

/**
 * @brief 当前所存储的数据类型表
 *
 */
enum stroe_type
{
    TYPE_BASIC_DATA,                   /* 基础数据类型 */
    TYPE_INDICATOR_DIAGRAM_BASIC_DATA, /* 功图基础数据类型 */
    TYPE_INDICATOR_DIAGRAM,            /* 功图数据类型 */
    TYPE_WATER_WELL_DATA,              /* 水源井数据类型 */
    TYPE_VALVE_GROUP_DATA,             /* 阀组间数据类型 */
    TYPE_MANIFOLD_PRESSURE             /* 汇管压力(AI)数据类型 */
};

/**
 * @brief 用于保存状态机的当前状态信息
 *
 */
struct state_machine_current_info
{
    bool     is_add_id;            /* 是否对id进行++操作 */
    int      id;
    int      id_oil_well;          /* 当前油井编号(ID) */
    int      id_ind;
    int      id_indicator_diagram; /* 当前功图对应的油井编号(ID) */
    uint8_t  func_code;            /* 当前功能码 */
    uint16_t start_addr;           /* 当前寄存器起始地址 */
    uint16_t addr_len;             /* 当前地址长度 */
    bool     isGetTime;            /* 当前GetTime的状态 */
    uint16_t phase;                /* 当前状态机所处的阶段 */
    string   time;                 /* 当前时间 */
    int      store_type;           /* 当前所存储的数据类型 */
    bool     is_again;             /* 判断当前是否开始请求下一口井的数据 */
};

/**
 * @brief 用于存储一个站号的一组基础数据
 *
 */
struct data_block
{
    uint8_t              rtu_id;                /* 站号 */
    std::vector<uint16_t>basic_value;           /* 对应寄存器值的基础数据 */
    std::vector<uint16_t>ind_diagram_basic_val; /* 功图基础数据 */
    std::vector<uint16_t>ind_diagram;           /* 功图数据 */
    std::vector<uint16_t>water_well_data;       /* 水源井数据 */
    std::vector<uint16_t>valve_group_data;      /* 阀组间数据 */
    std::vector<uint16_t>manifold_pressure;     /* 汇管压力数据 */




    time_t               cur_time_diagram;      /* 存储功图的当前时间 */
};

/**
 * @brief 用于存储上位机发送来的数据
 *
 */
struct tcp_data
{
    uint8_t   frame_header[6]; /* 帧头 */
    uint8_t   rtu_id;          /* 站号 */
    uint8_t   func_code;       /* 功能码 */
    uint16_t  start_addr;      /* 寄存器起始地址 */
    uint16_t  len;             /* 地址/数组长度 */
    uint16_t *value;           /* 指向对应寄存器值的数组 */
};

struct rtu_info
{
    int      id;      /* 站号 */
    uint64_t id_addr; /* xbee64位地址 */
    int      type;    /* 油井，水井，阀组间 */
};

/**
 * @brief 井口信息
 *
 */
struct well_info
{
    int      id;   /* 站号 */
    int      type; /* 类型  0:<油井>  1:<水井>  2:<阀组> */
    uint64_t addr; /* 64位设备地址 */
};

/**
 * @brief 配置信息
 *
 */
struct config_info
{
    int    port;                    /* 端口号 */
    string ip;                      /* ip地址 */
    string gateway;                 /* 网关 */
    string mac;                     /* mac地址 */
    string mask;                    /* 子网掩码 */

    int valve_group;                /* 阀组连接方式 0:<通过ZigBee连接>  1:<通过RS485连接> */
    int valve_SEL;                  /* 第一位到第四位分别为125~128  0:<配置>  1:<未配置> */


    int manifold_pressure;          /* 汇管压力连接方式，同上 */
    int manifold_pressure_id;       /* 汇管压力连接到第几个站号上 */

    string xbee_id;                 /* 16位的PAN ID */
    string xbee_sc;                 /* 7fff */
    int    xbee_ao;                 /* 0:<不接收ack>  1:<接收ack> */
    int    xbee_ce;                 /* 0:<路由器>  1:<协调器> */

    int               well_max_num; /* 配置的井口最大个数 */
    struct well_info *w_info;       /* 井口的配置信息 */
};

// TODO : 这里以后可以把指针更换成智能指针
class MultiTask {
private:

    /* modbus handler */
    modbus_t *ctx = nullptr;

    /* 用于保存在构造函数中，是否出现错误 */
    bool is_error = false;

    /* 用于监听的 socket */
    int server_socket;

    /* 用于保存状态机的当前状态信息 */
    struct state_machine_current_info current_info;

    /* 用于配置DI AI DO */
    struct DI_AI_DO
    {
        /* true表示在井场使用了，false表示未使用 */
        bool    DI;
        bool    DO;
        bool    AI;
        uint8_t di_val[2];
        uint8_t do_val;
        uint8_t ai_val;
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

    /* 用于存储上位机发送来数据 */
    struct tcp_data tcp_data;

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

    /* 用于保存站号的类别 */
    struct classify_id
    {
        int oil_well_id[16];
        int water_well_id[16];
        int valve_group_id[16];
    } class_id;

    /* 用于指向存储站号的两组数据区 */
    struct data_block *data_block = nullptr;
    struct data_block *data_block_2 = nullptr;

    /* 用于指向分配的两个data_block数据区 */
    struct to_data_block
    {
        struct data_block *wr_db;
        struct data_block *rd_db;
    }to_db;

    /* xbee handler */
    XBee *xbee_handler = nullptr;

    /* xbee serial handler */
    uart *xbee_ser = nullptr;

    /* 用于指示上位机数据是否到来 */
    bool is_data_come = false;

    /* sqlite helper handler */
    sqlControl *sql_handler = nullptr;

    /* i2c handler */
    i2c_handler *i2c_D = nullptr;

    /* spi handler */
    struct spi_handler spi_ai;


    /* ----------------------配置项---------------------- */
    /* configure */
    XMLDocument config;                  /* 定义配置对象 */
    XMLElement *config_label = nullptr;  /* 指向配置文件中的标签 */
    XMLElement *config_option = nullptr; /* 指向标签中的配置 */

    /* 用于保存从配置项文件中得到的配置信息 */
    struct config_info c_info;

    /* 用于存储配置的井口信息*/
    struct well_info well_info;


    /*---------------------互斥量和条件变量---------------*/

    /* 用于控制 zigbee 和 上位机共享数组的互斥量 */
    std::mutex m_mutex_xbee_data;

    /* 用于通知上位机数据已经准备好的条件变量 */
    std::condition_variable m_cond_datacom;

    /* 用于管理 xbee_handler 的互斥量 */
    std::mutex m_mutex_xbee;

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

    /* 用于判断接收的井口基础数据是否准备好 */
    std::condition_variable cv_basic_data;

    /* 用于判断接收的井口功图数据是否准备好 */
    std::condition_variable cv_diagram_data;

    /* 用于判断当前是否开始请求下一口井的数据的条件变量 */
    std::condition_variable cv_cur_info;

public:

    explicit MultiTask();

    virtual ~MultiTask();

    void initThread();

    void getWellPortInfoThread();

    void hostRequestProcThread();

    void updateThread();

    void forwardHostMsgThread();

    void sqlMemoryThread();

    bool get_error()
    {
        return is_error;
    }

    int config_manage();

    int rs485(struct state_machine_current_info *curr_info,
              struct data_block                 *data_block);

    int to_xbee(struct tcp_data *tcp_data);

    int sel_data_to_tcp(struct tcp_data *tcp_data);

    int state_machine_operation(struct state_machine_current_info *curr_info,
                                struct data_block                 *data_block);
};

#endif // ifndef __MULTITASK_H__
