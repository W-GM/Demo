/**
 * @file multitask.cpp
 * @author wgm (wangguomin@scaszh.com)
 * @brief This is the CQOF wellsite RTU multi-threaded program
 * @version 1.1
 * @date 2020-07-03
 *
 * @copyright Copyright (c) 2020
 *
 */
#include "multitask.h"
#include "constants.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "helper.h"

/**
 * @brief Construct a new Multi Task:: Multi Task object
 *
 */
MultiTask::MultiTask()
{
    /* ---------读取配置文件中的配置项--------- */
    if (get_jconfig_info() < 0)
    {
        is_error = ERROR_CONFIG;
        return;
    }

    /* ------------ 初始化xbee ------------ */
    xbee_handler = new XBee();

    if (xbee_handler == nullptr)
    {
        // fprintf(stderr, "xbee_hanler new error\n");
        is_error = ERROR_XBEE;
        return;
    }

    /* ------初始化xbee对应的uart ------- */
    xbee_ser = new uart();

    if (xbee_ser == nullptr)
    {
        // fprintf(stderr, "xbee_ser new error\n");
        is_error = ERROR_XBEE;
        return;
    }

    xbee_handler->setSerial(xbee_ser);

    /* 打开串口 */
#ifdef ARMCQ

    if (xbee_ser->Open("/dev/ttymxc6", 9600) < 0)
#else  // ifdef ARMCQ

    if (xbee_ser->Open("/dev/ttyUSB1", 9600) < 0)
#endif // ifdef ARMCQ
    {
        is_error = ERROR_XBEE;
        return;
    }

    /* -------------初始化DI，DO，AI的配置--------------- */
#ifdef ARMCQ
    /* DI,DO(I2C)初始化 */
    i2c_D = i2c_init("/dev/i2c-2", 0x74, 0xff, 0x03);
    if (i2c_D == nullptr)
    {
        is_error = ERROR_I2C;
        return;
    }
    else
    {
        debug("%s\n", "DI,DO初始化成功");
    }

    /* AI(SPI)初始化 */
    spi_ai.spi = spi_init("/dev/spidev0.0");
    if (spi_ai.spi == nullptr)
    {
        is_error = ERROR_SPI;
        return;
    }
    else
    {
        debug("%s\n", "AI初始化成功");
    }

    memset(spi_ai.val, 0, 2);
    memset(spi_ai.buf_rx, 0, 2);
    memset(spi_ai.val, 0, 16);
#endif

    /* ------------------ rs232配置 ------------------ */
#ifdef ARMCQ
    serial_parity_t rs232_parity;
    if (get_cfg().serial_cfg[0].parity == "None")
    {
        rs232_parity = PARITY_NONE;
    }
    else if (get_cfg().serial_cfg[0].parity == "Odd")
    {
        rs232_parity = PARITY_ODD;
    }
    else if (get_cfg().serial_cfg[0].parity == "Even")
    {
        rs232_parity = PARITY_EVEN;
    }

    if (uart_232->Open("/dev/ttymxc6",
                       atoi(get_cfg().serial_cfg[0].baudrate.c_str()),
                       rs232_parity,
                       get_cfg().serial_cfg[0].databit,
                       get_cfg().serial_cfg[0].stopbit) < 0)
    {
        is_error = ERROR_RS232;
        return;
    }
#endif

    /* ------------------ rs485 0口初始化 ------------------ */
#ifdef ARMCQ
    uint16_t rs485_0_parity;
    if (get_cfg().serial_cfg[1].parity == "None")
    {
        rs485_0_parity = 1;
    }
    else if (get_cfg().serial_cfg[1].parity == "Odd")
    {
        rs485_0_parity = 2;
    }
    else if (get_cfg().serial_cfg[1].parity == "Even")
    {
        rs485_0_parity = 3;
    }
    rs485_info[0].fd =
        usart_init(0,
                   atoi(get_cfg().serial_cfg[1].baudrate.c_str()),
                   get_cfg().serial_cfg[1].databit,
                   get_cfg().serial_cfg[1].stopbit,
                   rs485_0_parity);

    if (rs485_info[0].fd < 0)
    {
        is_error = ERROR_485_0;
        return;
    }
#else  // ifdef ARMCQ
    // uart_485 = new uart();
    // if (uart_485 == nullptr)
    // {
    //     is_error = ERROR_485_0;
    //     return;
    // }
    // /* 打开串口 */
    // if (uart_485->Open("/dev/ttyUSB1", 9600) < 0)
    // {
    //     is_error = ERROR_485_0;
    //     return;
    // }
#endif // ifdef ARM_CQ

    /* ------------------ rs485 1口初始化 ------------------ */
#ifdef ARMCQ
    uint16_t rs485_1_parity;
    if (get_cfg().serial_cfg[2].parity == "None")
    {
        rs485_1_parity = 1;
    }
    else if (get_cfg().serial_cfg[2].parity == "Odd")
    {
        rs485_1_parity = 2;
    }
    else if (get_cfg().serial_cfg[2].parity == "Even")
    {
        rs485_1_parity = 3;
    }
    rs485_info[1].fd =
        usart_init(0,
                   atoi(get_cfg().serial_cfg[2].baudrate.c_str()),
                   get_cfg().serial_cfg[2].databit,
                   get_cfg().serial_cfg[2].stopbit,
                   rs485_0_parity);

    if (rs485_info[1].fd < 0)
    {
        is_error = ERROR_485_1;
        return;
    }
#endif

    /* ------------------ 网口配置 ------------------ */
#ifdef ARMCQ
    /* eth0配置 */
    /* 设置IP和子网掩码 */
    std::string ip;
    ip += "ifconfig eth0 ";
    ip += get_cfg().eth_cfg[0].ip;
    ip += " netmask ";
    ip += get_cfg().eth_cfg[0].mask;
    system(ip.c_str());

    /* 设置网关 */
    std::string gw;
    gw += "route add default gw ";
    gw += get_cfg().eth_cfg[0].gateway;
    gw += " dev eth0";
    system(gw.c_str());

    /* eth1配置 */
    /* 设置Mac地址 */
    std::string mac;
    mac += "ifconfig eth1 hw ether 00.c9.";
    mac += get_cfg().eth_cfg[1].ip;
    replace(mac.begin(), mac.end(), '.', ':');
    system(mac.c_str());

    /* 启动网卡1 */
    system("ifconfig eth1 up");

    /* 设置IP和子网掩码 */
    std::string ip;
    ip += "ifconfig eth1 ";
    ip += get_cfg().eth_cfg[1].ip;
    ip += " netmask ";
    ip += get_cfg().eth_cfg[1].mask;
    system(ip.c_str());

    /* 设置网关 */
    std::string gw;
    gw += "route add default gw ";
    gw += get_cfg().eth_cfg[1].gateway;
    gw += " dev eth1";
    system(gw.c_str());
#endif // ifdef ARMCQ

    /* ------------------ wifi配置 ------------------ */
    //TODO：wifi配置待完善

    /* ------------------ xbee配置 ------------------ */
    int xbee_ret = 0;
    const char *xbee_error_type[] = {"未成功设置SC",
                                     "未成功设置为AP模式",
                                     "未成功设置CE",
                                     "未成功设置AO",
                                     "未成功设置ID",
                                     "未成功保存设置",
                                     "未成功退出命令行模式"};

    /* 接收到的AT命令ACK */
    uint8_t recv_ack[10] = {0};

    /* 初始化的AT命令 */
    uint8_t uint_at_command[10][20];

    for (int i = 0; i < 10; i++)
    {
        memset(uint_at_command[i], 0, 20);
    }
    string atsc = "ATSC" + get_cfg().xbee_sc + "\r";
    string atce = "ATCE" + get_cfg().xbee_ce + "\r";
    string atao = "ATAO" + get_cfg().xbee_ao + "\r";
    string atid = "ATID" + get_cfg().xbee_id + "\r";
    strcpy((char *)uint_at_command[0], "+++");
    strcpy((char *)uint_at_command[1], atsc.c_str());
    strcpy((char *)uint_at_command[2], "ATAP1\r");
    strcpy((char *)uint_at_command[3], atce.c_str());
    strcpy((char *)uint_at_command[4], atao.c_str());
    strcpy((char *)uint_at_command[5], atid.c_str());
    strcpy((char *)uint_at_command[6], "ATC810\r");
    strcpy((char *)uint_at_command[7], "ATNJFF\r");
    strcpy((char *)uint_at_command[8], "ATWR\r");
    strcpy((char *)uint_at_command[9], "ATCN\r");

    /* 如果发送失败,重发 */
    for (int i = 0; i < 3; i++)
    {
        memset(recv_ack, 0, 10);

        for (int j = 0; j < 2; j++)
        {
            /* 进入AT命令模式 */
            xbee_ser->Write(uint_at_command[0], 3);

            if (xbee_ser->Read(recv_ack, 10, 5000) <= 0)
            {
                debug("%s\n", "AT command read error or timeout");
                continue;
            }
            else
            {
                break;
            }
        }

        /* 判断是否成功进入AT命令模式 */
        if (strncmp((char *)recv_ack, "OK", 2) == 0)
        {
            /* 循环发送AT命令 */
            for (int j = 1; j < 10; j++)
            {
                for (int m = 0; m < 2; m++)
                {
                    xbee_ser->Write(uint_at_command[j],
                                    strlen((char *)uint_at_command[j]));

                    if (xbee_ser->Read(recv_ack, 10, 2000) <= 0)
                    {
                        debug("%s\n", "uart read error or timeout");
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }

                if (strncmp((char *)recv_ack, "OK", 2) != 0)
                {
                    debug("%s\n", xbee_error_type[j]);
                    xbee_ret = -1;
                    break;
                }
                else
                {
                    continue;
                    xbee_ret = 0;
                }
            }
            if (xbee_ret < 0)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            debug("%s\n", "未成功进入AT 命令行模式 ");
            xbee_ret = -1;
        }
    }

    if (xbee_ret < 0)
    {
        debug("%s\n", "设置ZigBee失败 ");
    }
    else
    {
        debug("%s\n", "设置ZigBee成功 ");
    }

    /* ------------ 初始化sqlite3 ------------ */
#ifdef SQL
    sql_handler = new sqlControl();

    char path[] = "./wellsiteRTUData.db";

    /* 打开数据库，没有则创建 */
    if (sql_handler->sqlOpen(path) == -1)
    {
        debug("%s\n", "sql open error");
        is_error = ERROR_SQL;
        return;
    }

    /*开启 高速，写同步 */
    if (sql_handler->writeSync() < 0)
    {
        is_error = ERROR_SQL;
        return;
    }

    /* 创建数据表 */
    strcpy(sql_tab_name.tab_basic, "basicInfo");
    strcpy(sql_tab_name.tab_indicator_diagram_basic, "indicatorDiagramBasic");
    strcpy(sql_tab_name.tab_indicator_diagram, "indicatorDiagram");
    strcpy(sql_tab_name.tab_water_well, "waterWell");
    strcpy(sql_tab_name.tab_valve_group, "valveGroup");

    char basic_info_field[] =
        "_saveTime text, _wellNum int, register int, basicData int";
    char indicator_diagram_basic_field[] =
        "_saveTime text, _wellNum int, register int, basicData int";
    char indicator_diagram_field[] =
        "_saveTime text, _wellNum int, register int, basicData int";
    char water_well_field[] =
        "_saveTime text, _wellNum int, register int, basicData int";
    char valve_group_field[] =
        "_saveTime text, _wellNum int, register int, basicData int";

    /* 创建基础数据表 */
    sql_handler->sqlCreateTable(sql_tab_name.tab_basic,
                                basic_info_field);

    /* 创建功图和功率图基础数据表 */
    sql_handler->sqlCreateTable(sql_tab_name.tab_indicator_diagram_basic,
                                indicator_diagram_basic_field);

    /* 创建功图数据表 */
    sql_handler->sqlCreateTable(sql_tab_name.tab_indicator_diagram,
                                indicator_diagram_field);

    /* 创建水源井数据表 */
    sql_handler->sqlCreateTable(sql_tab_name.tab_water_well,
                                water_well_field);

    /* 创建阀组间数据表 */
    sql_handler->sqlCreateTable(sql_tab_name.tab_valve_group,
                                valve_group_field);
#endif // ifdef SQL
}

/**
 * @brief Destroy the Multi Task:: Multi Task object
 *
 */
MultiTask::~MultiTask()
{
    /* restore the old configuration */

#ifdef ARMCQ
    tcsetattr(rs485_info[0].fd, TCSANOW, &rs485_info[0].oldtio);
    close(rs485_info[0].fd);

    tcsetattr(rs485_info[1].fd, TCSANOW, &rs485_info[1].oldtio);
    close(rs485_info[1].fd);

    uart_232->Close();

    spi_close(spi_ai.spi);
    spi_free(spi_ai.spi);

    i2c_close(i2c_D->i2c);
    i2c_free(i2c_D->i2c);
    free(i2c_D);
    i2c_D = NULL;
#endif
#ifdef test
    modbus_free(ctx);
    ctx = nullptr;
#endif //test
    delete[] data_block;
    data_block = nullptr;
    delete[] data_block_2;
    data_block_2 = nullptr;

#ifdef SQL
    delete sql_handler;
    sql_handler = nullptr;
#endif

    delete xbee_ser;
    xbee_ser = nullptr;

    delete xbee_handler;
    xbee_handler = nullptr;
}

/**
 * @brief 初始化线程
 *
 */
void MultiTask::thread_init()
{
    debug("%s\n", "This is init_thread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /* ------------初始化modbus TCP服务器---------------- */
    /* IP地址 */
    memset(server_info.ip_addr, 0, 16);
    strcpy(server_info.ip_addr, get_cfg().ip.c_str());

    /* 端口号 */
    server_info.port = get_cfg().port;

    /* --------------初始化上位机发送来的数据------------ */
    tcp_data.id = 0;
    tcp_data.func_code = 0x03;
    tcp_data.start_addr = 0;
    tcp_data.len = 0;
    tcp_data.set_val = 0;
    tcp_data.byte_count = 0;
    memset(tcp_data.value, 0, sizeof(tcp_data.value));

    /* --------------初始化状态机状态信息---------------- */
    current_info.is_add_id = false;
    current_info.id_addr = 0;
    current_info.id = 0;
    current_info.id_ind = 0;
    current_info.id_indicator_diagram = 0;
    current_info.func_code = 0x03;
    current_info.start_addr = 0;
    current_info.addr_len = 0;
    current_info.isGetTime = true;
    current_info.phase = PHASE_START;
    current_info.store_type = TYPE_NO;
    current_info.is_again = false;

    /* --------------初始化井场RTU信息---------------- */
    memset(wellsite_info.manifold_pressure, 0, 4);
    memset(wellsite_info.fault_info, 0, 10);
    memset(wellsite_info.infrared_alarm, 0, 4);

    /* -------给data_block在堆区分配内存，并初始化------- */

    /* 在堆区分配一段内存用于存储所有配置的井口基础数据 */
    data_block = new struct data_block[get_cfg().well_max_num];
    data_block_2 = new struct data_block[get_cfg().well_max_num];

    if ((data_block == nullptr) || (data_block_2 == nullptr))
    {
        debug("%s\n", "new data_block error");
        return;
    }

    //TODO：数据块部分的初始化后续根据实际情况再做修改

    /* 初始化 */
    for (int i = 0; i < get_cfg().well_max_num; i++)
    {

        data_block[i].id = get_cfg().well_info[i].id;
        data_block_2[i].id = get_cfg().well_info[i].id;

        data_block[i].injection_well_num = 0;
        data_block_2[i].injection_well_num = 0;

        switch (get_cfg().well_info[i].type)
        {
        case 0x01: /* 油井 */
            data_block[i].cur_time_diagram = 0;
            data_block[i].oil_basic_data.resize(200);
            data_block[i].ind_diagram.resize(810);
            data_block[i].power_diagram.resize(410);

            data_block_2[i].cur_time_diagram = 0;
            data_block_2[i].oil_basic_data.resize(200);
            data_block_2[i].ind_diagram.resize(810);
            data_block_2[i].power_diagram.resize(410);
            break;
        case 0x02: /* 水井 */
            data_block[i].water_well_data.resize(70);
            data_block_2[i].water_well_data.resize(70);
            break;
        case 0x31: /* 阀组 */
        case 0x32:
            data_block[i].valve_group_data.resize(100);
            data_block[i].wellsite_rtu.resize(200);

            data_block_2[i].valve_group_data.resize(100);
            data_block_2[i].wellsite_rtu.resize(200);
            break;
        default: /*  */
            break;
        }
    }

    /* 未配置阀组的井场128站号 */
    if (get_cfg().well_info[get_cfg().well_max_num - 1].type == 0)
    {
        data_block[get_cfg().well_max_num - 1].wellsite_rtu.resize(100);
        data_block_2[get_cfg().well_max_num - 1].wellsite_rtu.resize(100);
    }

    /* 初始化写指针指向data_block,读指针指向data_block_2 */
    to_db.wr_db = data_block;
    to_db.rd_db = data_block_2;
    to_db.wr_db_ind = data_block;
    to_db.rd_db_ind = data_block_2;
}

/**
 * @brief 获取井口数据线程
 *
 */
void MultiTask::thread_get_wellport_info()
{
    debug("%s\n", "This is get_wellport_info_thread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /* 用于存入数据库的 string */
    std::string sql = "";

    /* 返回值 */
    int ret = 0;

    /* 是否开始采集功图 */
    bool is_collect_diagram = false;

    /* 是否开始纪录起始时间 */
    bool is_record_time = true;

    /* 是否可以重新纪录时间 */
    bool is_re_record_time = false;

    /* 纪录功图采集的起始时间 */
    time_t start_time = 0;

    /* 用于保存设备故障信息的临时变量 */
    struct device_fault_info dev_fault;

    /* 用于保存当前站号是否产生了故障 */
    bool is_dev_fault = false;

    /* 用于保存2的指数结果，用于与按位保存的该站号故障类型进行“位与”操作，从而得到具体的故障类型 */
    int pow_value = 0;

    /* 保存每次读取油井基础数据的地址长度 */
    uint16_t oil_basic_addr_len[] =
        {0x00, 0x1e, 0x1f, 0x20, 0x21, 0x20, 0x2a};

    /* 保存每次读取功图数据的地址长度 */
    uint16_t ind_diagram_addr_len[] =
        {
            0x00, 0x26, 0x27, 0x28, 0x29, 0x2a, /* 起始地址 40410 */
            0x26, 0x27, 0x28, 0x29, 0x2a,       /* 起始地址 40610 */
            0x26, 0x27, 0x28, 0x29, 0x2a,       /* 起始地址 40810 */
            0x26, 0x27, 0x28, 0x29, 0x2a,       /* 起始地址 41010 */
        };

    uint16_t power_diagram_addr_len[] =
        {
            0x00, 0x26, 0x27, 0x28, 0x29, 0x2a, /* 起始地址 41210 */
            0x26, 0x27, 0x28, 0x29, 0x2a,       /* 起始地址 41410 */
        };

    while (1)
    {
        switch (current_info.phase)
        {
        case PHASE_START:
        {
            if (current_info.is_add_id)
            {
                current_info.id_addr++;
            }

            /* 是否开始纪录起始时间 */
            if (is_record_time)
            {
                start_time = time(nullptr);

                is_record_time = false;
            }

            /* 当最后一个站号读取完数据后开始重新获取起始时间 */
            if ((time(nullptr) - start_time > 100) &&
                (is_re_record_time == true) &&
                (current_info.id_addr == (get_cfg().well_max_num - 1)))
            {
                is_record_time = true;
                is_collect_diagram = false;
                is_re_record_time = false;
            }

            /* 用于切换1到最后一口配置井口从机地址 */
            if (current_info.id_addr < get_cfg().well_max_num)
            {
                current_info.id =
                    get_cfg().well_info[current_info.id_addr].id;
                current_info.is_add_id = true;
            }
            else
            {
                usleep(100000);
                current_info.id_addr = 0;
                current_info.id =
                    get_cfg().well_info[current_info.id_addr].id;

                /* ############# 上锁 ############# */
                m_rd_db.lock();

                if (to_db.rd_db == data_block)
                {
                    to_db.rd_db = data_block_2;
                    to_db.wr_db = data_block;
                }
                else if (to_db.rd_db == data_block_2)
                {
                    to_db.rd_db = data_block;
                    to_db.wr_db = data_block_2;
                }

                /* ############# 解锁 ############# */
                m_rd_db.unlock();
            }

            /* 100s后并且从第一个站号开始读取数据时，开始读取功图数据 */
            if ((time(nullptr) - start_time > 100) &&
                (current_info.id_addr == 0) &&
                (to_db.wr_db != to_db.wr_db_ind))
            {
                is_re_record_time = true;
                is_collect_diagram = true;
                to_db.wr_db_ind = to_db.wr_db;
                to_db.rd_db_ind = to_db.rd_db;
                //ind_diagram_store_place = to_db.wr_db;
            }

            /* 判断当前的站号的类型，并执行对应的程序 */
            if (get_cfg().well_info[current_info.id_addr].type ==
                TYPE_OIL_WELL_DATA)
            {
                current_info.phase = PHASE_OIL_WELL;
            }
            else if (get_cfg().well_info[current_info.id_addr].type ==
                     TYPE_WATER_WELL_DATA)
            {
                current_info.phase = PHASE_WATER_WELL;
            }
            else if ((get_cfg().well_info[current_info.id_addr].type >> 4) ==
                     TYPE_VALVE_GROUP_DATA)
            {
                current_info.phase = PHASE_VALVE_GROUP;
            }
            else if ((get_cfg().well_info[current_info.id_addr].id == 128) &&
                     (get_cfg().well_info[current_info.id_addr].type == 0))
            {
                current_info.phase = PHASE_WELLSITE_RTU;
            }
            else
            {
                current_info.phase = PHASE_START;
            }

            break;
        }

        case PHASE_OIL_WELL:
        {
            to_db.wr_db[current_info.id_addr].oil_basic_data.clear();

            /* 设置当前阶段的状态信息 */
            current_info.func_code = 0x03;
            current_info.start_addr = 0;
            current_info.store_type = TYPE_OIL_WELL_DATA;

            for (int i = 0; i < 6; i++)
            {
                current_info.start_addr += oil_basic_addr_len[i];
                current_info.addr_len = oil_basic_addr_len[i + 1];

                ret = state_machine_operation(&current_info, to_db.wr_db);

                if (ret == 0)
                {
                    break;
                }
            }

            /* 如果超时，则重新开始读下一个站号数据 */
            if (ret == 0)
            {
                current_info.phase = PHASE_START;
                break;
            }

            /* 判断当前站号是否有故障,并记录故障信息到井场RTU上 */
            // if(to_db.wr_db[current_info.id_addr].oil_basic_data[52] == 1)
            // {
            //     /* 故障设备代码 */
            //     wellsite_info.fault_info[0] = 100 + current_info.id;

            //     /* 故障部位代码 */
            //     wellsite_info.fault_info[1] =
            //         to_db.wr_db[current_info.id_addr].oil_basic_data[53];

            //     /* 故障时间 */
            //     // wellsite_info.fault_info[2] =
            //     //     ((get_current_time().year - 2000) << 8) |
            //     //     (get_current_time().month & 0xff);
            //     // wellsite_info.fault_info[3] =
            //     //     (get_current_time().day << 8) |
            //     //     (get_current_time().hour & 0xff);
            //     // wellsite_info.fault_info[4] =
            //     //     (get_current_time().minute << 8) |
            //     //     (get_current_time().second & 0xff);
            //     wellsite_info.fault_info[2] =
            //         to_db.wr_db[current_info.id_addr].oil_basic_data[54];
            //     wellsite_info.fault_info[3] =
            //         to_db.wr_db[current_info.id_addr].oil_basic_data[55];
            //     wellsite_info.fault_info[4] =
            //         to_db.wr_db[current_info.id_addr].oil_basic_data[56];
            // }

            /* ############## 更新故障处理机制 wgm-2020-10-21 ############### */
            dev_fault.device_type = 100 + current_info.id;

            if (to_db.wr_db[current_info.id_addr].oil_basic_data[52] == 1)
            {
                is_dev_fault = true;
                dev_fault.fault_type.clear();
                for (int i = 0; i < 16; i++)
                {
                    pow_value = pow(2, i);
                    if (to_db.wr_db[current_info.id_addr].oil_basic_data[53] & pow_value)
                    {
                        dev_fault.fault_type.push_back(100 + i);
                    }
                }
                dev_fault.time[0] = to_db.wr_db[current_info.id_addr].oil_basic_data[54];
                dev_fault.time[0] = to_db.wr_db[current_info.id_addr].oil_basic_data[55];
                dev_fault.time[0] = to_db.wr_db[current_info.id_addr].oil_basic_data[56];
            }
            else
            {
                is_dev_fault = false;
            }

            /* ###### 上锁 ###### */
            m_dev_fault.lock();

            manage_fault_device(is_dev_fault, dev_fault);

            /* ###### 解锁 ###### */
            m_dev_fault.unlock();

            /* ############## END wgm-2020-10-21 ############### */

            /* 当前油井是否可以采集功图数据 */
            if (is_collect_diagram)
            {
                debug("%s\n", "开始采集功图数据########");
                /* 采集功图基础数据 */
                current_info.store_type = TYPE_INDICATION_DIAGRAM;
                to_db.wr_db[current_info.id_addr].ind_diagram.clear();

                /* 功图基础数据存储在功图数据的第0～9位 */
                current_info.start_addr = 0xc8;
                current_info.addr_len = 0xa;
                ret = state_machine_operation(&current_info, to_db.wr_db_ind);

                /* 只有当采集的功图基础数据无误时，才开始采功图数据 */
                if (ret > 0)
                {
                    /* 采集功图数据 */
                    current_info.start_addr = 0xd2;

                    for (int i = 0; i < 20; i++)
                    {
                        current_info.start_addr += ind_diagram_addr_len[i];

                        current_info.addr_len = ind_diagram_addr_len[i + 1];
                        ret = state_machine_operation(&current_info, to_db.wr_db_ind);
                        if ((ret == -2) || (ret == -3))
                        {
                            /* 功图前200个点或后200个点采集失败，直接跳出 */
                            break;
                        }
                        else if ((ret == 0) || (ret == -1))
                        {
                            /* 超时或接收数据出错或功率图200个点采集失败 */
                            break;
                        }
                    }
                }

                debug("%s\n", "开始采集功率图数据########");
                /* 采集功率图基础数据 */
                current_info.store_type = TYPE_POWER_DIAGRAM;
                to_db.wr_db[current_info.id_addr].power_diagram.clear();

                /* 功率图基础数据存储在功率图数据的第0～9位 */
                current_info.start_addr = 0x0582;
                current_info.addr_len = 0xa;
                ret = state_machine_operation(&current_info, to_db.wr_db_ind);

                /* 只有当采集的功率图基础数据无误时，才开始采功率图数据 */
                if (ret > 0)
                {
                    /* 采集功率图数据 */
                    current_info.start_addr = 0x3f2;

                    for (int i = 0; i < 10; i++)
                    {
                        current_info.start_addr += power_diagram_addr_len[i];

                        current_info.addr_len = power_diagram_addr_len[i + 1];
                        ret = state_machine_operation(&current_info, to_db.wr_db_ind);
                        if ((ret == -4) || (ret == -5))
                        {
                            /* 功率图前200个点或后200个点采集失败，直接跳出 */
                            break;
                        }
                        else if ((ret == 0) || (ret == -1))
                        {
                            /* 超时或接收数据出错或功率图200个点采集失败 */
                            break;
                        }
                    }
                    // if (ret == 0)
                    // {
                    //     current_info.phase = PHASE_START;
                    //     break;
                    // }
                }
            }

            /* 判断当前站号是否配置了汇管 */
            if (((get_cfg().manifold_1.type == CON_WIRELESS) &&
                 (get_cfg().manifold_1.id == current_info.id)) ||
                ((get_cfg().manifold_2.type == CON_WIRELESS) &&
                 (get_cfg().manifold_2.id == current_info.id)))
            {
                current_info.phase = PHASE_MANIFOLD_PRESSURE;
            }
            else
            {
                current_info.phase = PHASE_START;
            }

            break;
        }

        case PHASE_WATER_WELL:
        {
            to_db.wr_db[current_info.id_addr].water_well_data.clear();

            /* 设置当前阶段的状态信息 */
            current_info.func_code = 0x03;
            current_info.start_addr = 0x00;
            current_info.addr_len = 0x1e;
            current_info.store_type = TYPE_WATER_WELL_DATA;

            ret = state_machine_operation(&current_info, to_db.wr_db);

            if (ret < 0)
            {
                current_info.phase = PHASE_START;
                break;
            }

            /* ############## 更新故障处理机制 wgm-2020-10-22 ############### */
            dev_fault.device_type = 200 + current_info.id;

            if ((to_db.wr_db[current_info.id_addr].water_well_data[11] > 3) &&
                (to_db.wr_db[current_info.id_addr].water_well_data[11] < 15))
            {
                is_dev_fault = true;
                dev_fault.fault_type.clear();
                dev_fault.fault_type.push_back(200 + to_db.wr_db[current_info.id_addr].water_well_data[11]);

                dev_fault.time[0] = 0;
                dev_fault.time[0] = 0;
                dev_fault.time[0] = 0;
            }
            else
            {
                is_dev_fault = false;
            }

            /* ###### 上锁 ###### */
            m_dev_fault.lock();

            manage_fault_device(is_dev_fault, dev_fault);

            /* ###### 解锁 ###### */
            m_dev_fault.unlock();

            /* ############## END wgm-2020-10-22 ############### */

            /* 判断当前站号时候配置了汇管 */
            if (((get_cfg().manifold_1.type == CON_WIRELESS) &&
                 (get_cfg().manifold_1.id == current_info.id)) ||
                ((get_cfg().manifold_2.type == CON_WIRELESS) &&
                 (get_cfg().manifold_2.id == current_info.id)))
            {
                current_info.phase = PHASE_MANIFOLD_PRESSURE;
            }
            else
            {
                current_info.phase = PHASE_START;
            }
            break;
        }

        case PHASE_VALVE_GROUP:
        {
            /* 初始化，清零 */
            to_db.wr_db[current_info.id_addr].valve_group_data.clear();

            /* 读取配置的注水井的个数和注水井总的汇管压力 */
            current_info.func_code = 0x04;
            current_info.start_addr = 0x06;
            current_info.addr_len = 0x08;
            current_info.store_type = TYPE_VALVE_GROUP_DATA;

            if ((get_cfg().well_info[current_info.id_addr].type & 0xf) ==
                CON_WIRED)
            {
                ret = rs485(&current_info, to_db.wr_db);
            }
            else if ((get_cfg().well_info[current_info.id_addr].type & 0xf) ==
                     CON_WIRELESS)
            {
                ret = state_machine_operation(&current_info, to_db.wr_db);
            }

            if (ret < 0)
            {
                current_info.phase = PHASE_WELLSITE_RTU;
                break;
            }

            /* 读取配置的注水井个数 */
            to_db.wr_db[current_info.id_addr].injection_well_num =
                to_db.wr_db[current_info.id_addr].valve_group_data[2];

            /* 各注水井的基本数据 */
            for (int i = 0;
                 i < to_db.wr_db[current_info.id_addr].injection_well_num;
                 i++)
            {
                /* 设置当前阶段的状态信息 */
                current_info.start_addr = 0x0e + i * 0xa;
                current_info.addr_len = 0x8;

                if ((get_cfg().well_info[current_info.id_addr].type & 0xf) ==
                    CON_WIRED)
                {
                    ret = rs485(&current_info, to_db.wr_db);
                }
                else if ((get_cfg().well_info[current_info.id_addr].type & 0xf) ==
                         CON_WIRELESS)
                {
                    ret = state_machine_operation(&current_info, to_db.wr_db);
                }

                if (ret < 0)
                {
                    break;
                }
            }

            if (ret < 0)
            {
                current_info.phase = PHASE_WELLSITE_RTU;
                break;
            }

            /* ############## 更新故障处理机制 wgm-2020-10-22 ############### */
            /* 轮询查找每一口注水井是否有故障 */
            for (int i = 0; i < to_db.wr_db[current_info.id_addr].injection_well_num; i++)
            {
                /* 当前注水井是否有故障 */
                if ((to_db.wr_db[current_info.id_addr].valve_group_data[14 + i * 10] & 0x8) > 0)
                {
                    dev_fault.device_type = (current_info.id - 120) * 100 + i;

                    is_dev_fault = true;
                    dev_fault.fault_type.clear();

                    /* 当前注水井电池是否正常 */
                    if ((to_db.wr_db[current_info.id_addr].valve_group_data[14 + i * 10] & 0x8) > 0)
                    {
                        dev_fault.fault_type.push_back((current_info.id - 120) * 100 + 4);
                    }

                    /* 当前注水井流量计是否正常 */
                    if ((to_db.wr_db[current_info.id_addr].valve_group_data[14 + i * 10] & 0x10) > 0)
                    {
                        dev_fault.fault_type.push_back((current_info.id - 120) * 100 + 5);
                    }

                    /* 协议转换箱与配注仪通讯正常 */
                    if ((to_db.wr_db[current_info.id_addr].valve_group_data[14 + i * 10] & 0x8000) > 0)
                    {
                        dev_fault.fault_type.push_back((current_info.id - 120) * 100 + 16);
                    }

                    dev_fault.time[0] = 0;
                    dev_fault.time[0] = 0;
                    dev_fault.time[0] = 0;
                }
                else
                {
                    is_dev_fault = false;
                }

                /* ###### 上锁 ###### */
                m_dev_fault.lock();

                manage_fault_device(is_dev_fault, dev_fault);

                /* ###### 解锁 ###### */
                m_dev_fault.unlock();
            }

            /* ############## END wgm-2020-10-22 ############### */

            current_info.phase = PHASE_WELLSITE_RTU;
            break;
        }

        case PHASE_MANIFOLD_PRESSURE:
        {
            /* 汇管1配置 */
            if ((get_cfg().manifold_1.type == CON_WIRELESS) &&
                (get_cfg().manifold_1.id == current_info.id))
            {
                if (to_db.wr_db[current_info.id_addr].oil_basic_data[15] == ANKONG)
                {
                    //TODO:当前配置在安控井口的汇管,默认直接去读取井口配置的AI
                    current_info.store_type = TYPE_MANIFOLD_PRESSURE_1;
                    current_info.start_addr = 0;
                    current_info.addr_len = 0x06;
                    current_info.func_code = 0x04;

                    debug("%s\n", "开始采集汇管1数据");

                    ret = state_machine_operation(&current_info, to_db.wr_db);

                    if (ret < 0)
                    {
                        break;
                    }
                }
                else if (to_db.wr_db[current_info.id_addr].oil_basic_data[15] == ANTE)
                {
                    //TODO:当前配置在安特井口的汇管,默认从40051寄存器中读取,默认为处理过的数据,后根据现场实际情况进行修改
                    //安特不同版本间，对于汇管的处理方式也存在不同
#ifdef ANTE_AI
                    current_info.store_type = TYPE_MANIFOLD_PRESSURE_1;
                    current_info.start_addr = 0;
                    current_info.addr_len = 0x06;
                    current_info.func_code = 0x04;

                    ret = state_machine_operation(&current_info, to_db.wr_db);

                    if (ret < 0)
                    {
                        break;
                    }
#else
                    wellsite_info.manifold_pressure[0] =
                        to_db.wr_db[current_info.id_addr].oil_basic_data[50];
#endif
                }
                else if (to_db.wr_db[current_info.id_addr].oil_basic_data[15] == OTHER)
                {
                    wellsite_info.manifold_pressure[0] =
                        to_db.wr_db[current_info.id_addr].oil_basic_data[59];
                }
            }
            else if (get_cfg().manifold_1.type == CON_WIRED)
            {
                // for (int i = 0; i < 2; i++)
                // {
                //     if (spi_transfer(spi_ai.spi,
                //             spi_ai.buf_tx[get_cfg().manifold_1.add],
                //             spi_ai.buf_rx,
                //             sizeof(spi_ai.buf_rx)) < 0)
                //     {
                //         fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi_ai.spi));
                //         exit(1);
                //     }
                // }

                // //当前汇管通过AI口得到的电流值在4~20mA之间，
                // //再根据所给量程，得到汇管的真实值（单位MPa）
                // //再对数据扩充100倍，得到要发送给上位机的汇管数据
                // ai_current =
                //     (((spi_ai.buf_rx[0] << 8) | (spi_ai.buf_rx[1] & 0xff)) & 0xfff) * 0.005055147;
                // wellsite_info.manifold_pressure[0] =
                //     (ai_current - 4) / 16 * 100 * get_cfg().manifold_1.range;

                wellsite_info.manifold_pressure[0] =
                    get_ai_value(spi_ai, get_cfg().manifold_1.add, get_cfg().manifold_1.range);

                debug("读取汇管1的数据 >> %d\n", wellsite_info.manifold_pressure[0]);
            }

            /* 汇管2配置 */
            if ((get_cfg().manifold_2.type == CON_WIRELESS) &&
                (get_cfg().manifold_2.id == current_info.id))
            {
                if (to_db.wr_db[current_info.id_addr].oil_basic_data[15] == ANKONG)
                {
                    //TODO:当前配置在安特井口的汇管,默认直接去读取井口配置的AI
                    current_info.store_type = TYPE_MANIFOLD_PRESSURE_2;
                    current_info.start_addr = 0;
                    current_info.addr_len = 0x06;
                    current_info.func_code = 0x04;

                    debug("%s\n", "开始采集汇管2数据");

                    ret = state_machine_operation(&current_info, to_db.wr_db);

                    if (ret < 0)
                    {
                        break;
                    }
                }
                else if (to_db.wr_db[current_info.id_addr].oil_basic_data[15] == ANTE)
                {
                    //TODO:当前配置在安特井口的汇管2,默认从40060寄存器中读取,默认为处理过的数据,后根据现场实际情况进行修改
#ifdef ANTE_AI
                    current_info.store_type = TYPE_MANIFOLD_PRESSURE_2;
                    current_info.start_addr = get_cfg().manifold_2.add - 1;
                    current_info.addr_len = 0x06;
                    current_info.func_code = 0x04;

                    ret = state_machine_operation(&current_info, to_db.wr_db);

                    if (ret < 0)
                    {
                        break;
                    }
#else
                    wellsite_info.manifold_pressure[1] =
                        to_db.wr_db[current_info.id_addr].oil_basic_data[59];
#endif
                }
                else if (to_db.wr_db[current_info.id_addr].oil_basic_data[15] == OTHER)
                {
                    wellsite_info.manifold_pressure[1] =
                        to_db.wr_db[current_info.id_addr].oil_basic_data[59];
                }
            }
            else if (get_cfg().manifold_1.type == CON_WIRED)
            {
                // for (int i = 0; i < 2; i++)
                // {
                //     if (spi_transfer(spi_ai.spi,
                //             spi_ai.buf_tx[get_cfg().manifold_2.add],
                //             spi_ai.buf_rx,
                //             sizeof(spi_ai.buf_rx)) < 0)
                //     {
                //         fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi_ai.spi));
                //         exit(1);
                //     }
                // }

                // //当前汇管通过AI口得到的电流值在4~20mA之间，
                // //再根据所给量程，得到汇管的真实值（单位MPa）
                // //再对数据扩充100倍，得到要发送给上位机的汇管数据
                // ai_current =
                //     (((spi_ai.buf_rx[0] << 8) | (spi_ai.buf_rx[1] & 0xff)) & 0xfff) * 0.005055147;
                // wellsite_info.manifold_pressure[1] =
                //     (ai_current - 4) / 16 * 100 * get_cfg().manifold_2.range;

                wellsite_info.manifold_pressure[1] =
                    get_ai_value(spi_ai, get_cfg().manifold_2.add, get_cfg().manifold_2.range);

                debug("读取汇管2的数据 >> %d\n", wellsite_info.manifold_pressure[1]);
            }

            current_info.phase = PHASE_START;
            break;
        }

        case PHASE_WELLSITE_RTU:
        {
            /* 初始化，清零 */
            //to_db.wr_db[current_info.id_addr].wellsite_rtu.clear();
            if (current_info.id == 128)
            {
                //TODO:井场及厂家信息,汇管,故障
                for (int i = 0; i < 50; i++)
                {
                    //TODO:前50为井场信息,暂未定
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[i];
                }

                /* ############# 上锁 ############# */
                m_wellsite.lock();

                /* 井场汇管压力 */
                to_db.wr_db[current_info.id_addr].wellsite_rtu[50] =
                    wellsite_info.manifold_pressure[0];
                to_db.wr_db[current_info.id_addr].wellsite_rtu[59] =
                    wellsite_info.manifold_pressure[1];

                debug("将汇管压力数据[%d, %d]保存到井场主RTU寄存器表中\n",
                      wellsite_info.manifold_pressure[0],
                      wellsite_info.manifold_pressure[1]);

                /* 注水井总的汇管压力 */
                to_db.wr_db[current_info.id_addr].wellsite_rtu[51] = 0;

                /* 井场故障信息 */
                for (int i = 0; i < 5; i++)
                {
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[52 + i] =
                        wellsite_info.fault_info[i];
                }

                /* 井场红外报警 */
                to_db.wr_db[current_info.id_addr].wellsite_rtu[57] =
                    wellsite_info.infrared_alarm[0];
                to_db.wr_db[current_info.id_addr].wellsite_rtu[58] =
                    wellsite_info.infrared_alarm[1];

                m_wellsite.unlock();
                /* ############# 解锁 ############# */
            }
            else
            {
                for (int i = 0; i < 60; i++)
                {
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[i];
                }
            }

            if ((get_cfg().well_info[current_info.id_addr].type >> 4) ==
                TYPE_VALVE_GROUP_DATA)
            {
                /* 注水井总的汇管压力 */
                to_db.wr_db[current_info.id_addr].wellsite_rtu[51] =
                    to_db.wr_db[current_info.id_addr].valve_group_data[4] +
                    to_db.wr_db[current_info.id_addr].valve_group_data[5] +
                    to_db.wr_db[current_info.id_addr].valve_group_data[6] +
                    to_db.wr_db[current_info.id_addr].valve_group_data[7];

                /* 各注水井信息,保存在wellsite_rtu中,从60开始存,每个注水井占6个寄存器 */
                for (int i = 0;
                     i < to_db.wr_db[current_info.id_addr].injection_well_num;
                     i++)
                {
                    /* 注水井注水量设定读  */
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[i * 6 + 60] =
                        to_db.wr_db[current_info.id_addr].valve_group_data[i * 8 + 9];
                    /* 注水井注水量设置值 */
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[i * 6 + 61] =
                        to_db.wr_db[current_info.id_addr].valve_group_data[i * 8 + 9];
                    /* 各分水井注水压力 */
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[i * 6 + 62] =
                        to_db.wr_db[current_info.id_addr].valve_group_data[i * 8 + 15];
                    /* 各分水井注水瞬时流量 */
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[i * 6 + 63] =
                        to_db.wr_db[current_info.id_addr].valve_group_data[i * 8 + 10];
                    /* 各分水井注水累计流量 */
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[i * 6 + 64] =
                        to_db.wr_db[current_info.id_addr].valve_group_data[i * 8 + 11];
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[i * 6 + 65] =
                        to_db.wr_db[current_info.id_addr].valve_group_data[i * 8 + 12];
                }
            }

            /* 汇管有线连接 */
            if ((get_cfg().manifold_1.type == CON_WIRED) ||
                (get_cfg().manifold_2.type == CON_WIRED))
            {
                current_info.phase = PHASE_MANIFOLD_PRESSURE;
            }
            else
            {
                current_info.phase = PHASE_START;
            }

            break;
        }
        default:
        {
            current_info.phase = PHASE_START;
            break;
        }
        }
    }
}

/**
 * @brief 处理上位机请求线程
 *
 */
void MultiTask::thread_host_request()
{
    debug("%s\n", "This is host_request_thread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /* 请求/响应数据 */
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

    /* 用于连接的 socket */
    int master_socket;

    /* modbus recv 成功接收的响应的数据长度 */
    int rc;

    /* 读事件表 */
    fd_set readfset;

    /* 临时的读事件表 */
    fd_set tempfset;

    /* 最大文件描述符数量 */
    int fdmax;

    /* 用于组合功图功率图信息的表的数据 */
    std::string sql = "";

    // TODO:配置项
    ctx = modbus_new_tcp(server_info.ip_addr, server_info.port);

    modbus_set_debug(ctx, ON);

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTON);

    if (server_socket == -1)
    {
        fprintf(stderr, "Unable to listen TCP connection\n");
        modbus_free(ctx);
        return;
    }

    /* 清空读事件表 */
    FD_ZERO(&readfset);

    /* 将 server_socket 加入事件表 */
    FD_SET(server_socket, &readfset);

    /* 最大文件描述符个数 */
    fdmax = server_socket;

    while (1)
    {
        tempfset = readfset;

        if (select(fdmax + 1, &tempfset, NULL, NULL, NULL) == -1)
        {
            perror("Server select() failure.");
            return;
        }

        /* 检查已存在的连接，看是否有数据读入 */
        for (master_socket = 0; master_socket <= fdmax; master_socket++)
        {
            if (!FD_ISSET(master_socket, &tempfset))
            {
                continue;
            }

            if (master_socket == server_socket)
            {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = accept(server_socket,
                               (struct sockaddr *)&clientaddr,
                               &addrlen);

                if (newfd == -1)
                {
                    perror("Server accept() error");
                }
                else
                {
                    FD_SET(newfd, &readfset);

                    if (newfd > fdmax)
                    {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(clientaddr.sin_addr),
                           clientaddr.sin_port,
                           newfd);
                }
            }
            else
            {
                modbus_set_socket(ctx, master_socket);

                debug("%s\n", "------------------------------------");

                memset(query, 0, sizeof(query));

                /* 读取上位机发送来的数据 */
                rc = modbus_receive(ctx, query);

                if (rc > 0)
                {
                    debug("--- %s ---\n", "above is modbus receive frame");

                    /* 将接收到的数据存到共享数据区 */
                    memset(tcp_data.frame_header, 0,
                           sizeof(tcp_data.frame_header));

                    for (int i = 0; i < 6; i++)
                    {
                        tcp_data.frame_header[i] = query[i];
                    }
                    tcp_data.id = query[6];
                    tcp_data.func_code = query[7];
                    tcp_data.start_addr = (query[8] << 8) | (query[9] & 0xff);

                    /* 读保持寄存器或输入寄存器 */
                    if ((tcp_data.func_code == 0x01) ||
                        (tcp_data.func_code == 0x02) ||
                        (tcp_data.func_code == 0x03) ||
                        (tcp_data.func_code == 0x04))
                    {
                        tcp_data.len =
                            (query[10] << 8) | (query[11] & 0xff);
                    }

                    /* 写单个线圈或单个寄存器 */
                    else if ((tcp_data.func_code == 0x05) ||
                             (tcp_data.func_code == 0x06))
                    {
                        tcp_data.set_val =
                            (query[10] << 8) | (query[11] & 0xff);
                    }

                    /* 写多个线圈 */
                    else if (tcp_data.func_code == 0x0f)
                    {
                        tcp_data.len =
                            (query[10] << 8) | (query[11] & 0xff);
                        tcp_data.byte_count = query[12];
                        memset(tcp_data.value, 0, sizeof(tcp_data.value));

                        for (int i = 0; i < tcp_data.byte_count; i++)
                        {
                            /* 将当前的单字节线圈输出值保存为双字节 */
                            tcp_data.value[i] = query[13 + i];
                        }
                    }

                    /* 写多个寄存器 */
                    else if (tcp_data.func_code == 0x10)
                    {
                        tcp_data.len =
                            (query[10] << 8) | (query[11] & 0xff);
                        tcp_data.byte_count = query[12];

                        memset(tcp_data.value, 0, sizeof(tcp_data.value));

                        for (int i = 0; i < tcp_data.len; i++)
                        {
                            tcp_data.value[i] =
                                (query[13 + i] << 8) | (query[14 + i] & 0xff);
                        }
                    }
                    else
                    {
                        continue;
                    }

                    /* 与上位机进行数据交互 */
                    sel_data_to_tcp(&tcp_data);
                }
                else if (rc == -1)
                {
                    /* This example server in ended on connection closing or
                     * any errors. */
                    debug("Connection closed on socket %d\n",
                          master_socket);
                    close(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &readfset);

                    if (master_socket == fdmax)
                    {
                        fdmax--;
                    }
                }
            }
        }
    }
}

#ifdef test
#ifdef SQL

/**
 * @brief 数据库存储线程
 *
 */
void MultiTask::thread_sql_memory()
{
    debug("%s\n", "This is sql_memory_thread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /* 用于组装sql语句 */
    std::string sql = "";

    int err_num = 0;

    /* 获取当前时间 */
    time_t c_time = 0;

    /* 保存要删除的语句 */
    std::string del_statement = "";

    /* 用于保存当前井口编号 */
    int id_well = 0;

    int id = 0;

    /* 用于保存当前功图对应的油井编号(ID) */
    int id_ind_diagrom = 0;

    /* 用于保存获取数据的当前时间 */
    string cur_time = "";

    /* 用于保存当前的数据类型 */
    int cur_store_type = 0;

    /* 用于保存每种数据的数据长度 */
    int cycles[3] = {0};

    /* 实际要存储的数据类型个数 */
    int cycles_n = 0;

    /* 判断当前是否需要重新获取时间,用于删除语句中*/
    bool del_is_get_time = true;

    /* 保存获取的时间，作为是否需要执行删除语句的起始时间 */
    time_t del_start_time = 0;

    char *tab_name = nullptr;

    while (1)
    {
        /* 阻塞等待，避免存取数据库太快，导致xbee还未请求到下一口数据 */

        /* ################VVV 上锁 VVV################# */
        std::unique_lock<std::mutex> db_lock(m_rd_db);

        /* 阻塞等待当前是否开始请求下一组站号数据 */
        while (!current_info.is_again)
        {
            cv_cur_info.wait(db_lock);
        }

        /* 保存当前的站号 */
        id_well = current_info.id;

        id = current_info.id_addr;

        /* ################AAA 解锁 AAA################# */
        db_lock.unlock();

        /* ################VVV 上锁 VVV################# */
        std::unique_lock<std::mutex> lck(m_basic_data);

        /* ###########AAA 解锁 AAA############ */
        lck.unlock();

        /* 获取数据准备好后的时间语句 */
        cur_time.clear();
        cur_time += current_info.time;

        cycles_n = 2;
        memset(cycles, 0, sizeof(cycles));

        /* ############VVV 上锁 VVV############ */
        m_data_block.lock();

        cycles[0] = data_block[id].oil_basic_data.size() - 1;
        cycles[1] = data_block[id].ind_diagram_basic_val.size() - 1;

        cur_store_type = current_info.store_type;

        // id_ind_diagrom = current_info.id_indicator_diagram;
        // TODO：测试
        id_ind_diagrom = id_well;

        /* ############AAA 解锁 AAA############# */
        m_data_block.unlock();

        if (cur_store_type == TYPE_INDICATOR_DIAGRAM)
        {
            /* ################VVV 上锁 VVV################# */
            std::unique_lock<std::mutex> dia_lock(m_diagram_data);

            /* 阻塞等待当前基础数据是否准备好 */
            while (!data_block[current_info.id_ind].is_ready_diagram)
            {
                cv_diagram_data.wait(dia_lock); /* 如果标志位不为 true, 则阻塞等待 */
            }

            /* ###########AAA 解锁 AAA############ */
            dia_lock.unlock();

            cycles_n = 3;
            cycles[2] = data_block[current_info.id_ind].ind_diagram.size() -
                        1;
        }

        /* [j=0]:基础数据；[j=1]:功图基础数据；[j=2]:功图数据 */
        for (int j = 0; j < cycles_n; j++)
        {
            debug("*******j = %d\n", j);

            for (int i = 0; i < cycles[j]; i++)
            {
                /* 组装sql语句 */
                sql.clear();
                sql += "\"" + cur_time + "\"";
                sql += ",";

                if (j == 2)
                {
                    sql += std::to_string(id_ind_diagrom);
                }
                else
                {
                    sql += std::to_string(id_well);
                }
                sql += ",";

                if (j == 0)
                {
                    sql += std::to_string(1 + i);
                }
                else if (j == 1)
                {
                    if (i < 7)
                    {
                        sql += std::to_string(201 + i);
                    }
                    else
                    {
                        sql += std::to_string(1411 + i);
                    }
                }
                else if (j == 2)
                {
                    sql += std::to_string(211 + i);
                }

                sql += ",";

                /* #############VVV 上锁 VVV############## */
                m_data_block.lock();

                if (j == 0)
                {
                    sql += std::to_string(data_block[id].oil_basic_data[i]);
                    tab_name = sql_tab_name.tab_basic;
                }
                else if (j == 1)
                {
                    sql += std::to_string(
                        data_block[id].ind_diagram_basic_val[i]);
                    tab_name = sql_tab_name.tab_indicator_diagram_basic;
                }
                else if (j == 2)
                {
                    sql += std::to_string(
                        data_block[current_info.id_ind].ind_diagram[i]);
                    tab_name = sql_tab_name.tab_indicator_diagram;
                }

                /* #############AAA 解锁 AAA############## */
                m_data_block.unlock();

                while (err_num <= 2)
                {
                    if (err_num == 2)
                    {
                        sql.clear();
                        sql += "\"" + std::to_string(0) + "\"" + "," +
                               std::to_string(0) + "," + std::to_string(0) +
                               "," +
                               std::to_string(0);
                    }

                    // 写入基础数据库
                    if (sql_handler->sqlInsertData(tab_name,
                                                   sql.c_str()) == -1)
                    {
                        debug("%s\n", "insert sqlite error");
                        err_num++;
                        continue;
                    }
                    break;
                }
                err_num = 0;
            }
        }

        /* 判断是否需要获取要执行删除语句的起始时间 */
        if (del_is_get_time)
        {
            time(&del_start_time);
            del_is_get_time = false;
        }

        /* 判断是否经过了一天时间后，再进行删除语句操作 */
        time(&c_time);

        if ((c_time - del_start_time) > 86400)
        {
            /* 删除五天之前的数据，保证数据库中只保存当前五天的数据 */

            c_time -= 432000;

            del_statement.clear();
            del_statement += "_saveTime<='";

            for (int i = 0; i < 11; i++)
            {
                del_statement += getCurTime(&c_time).c_str()[i];
            }
            del_statement += "00:00:00";

            // for (int i = 0; i < 17; i++)                         /*
            // 测试用,精度到分钟
            // */
            // {
            //     del_statement += getCurTime(&c_time).c_str()[i]; /* 测试用
            // */
            // }
            // del_statement += "00'";                              /* 测试用
            // */

            sql_handler->sqlDeleteData(sql_tab_name.tab_basic,
                                       del_statement.c_str());
            sql_handler->sqlDeleteData(
                sql_tab_name.tab_indicator_diagram_basic,
                del_statement.c_str());
            sql_handler->sqlDeleteData(sql_tab_name.tab_indicator_diagram,
                                       del_statement.c_str());

            del_is_get_time = true;
        }
    }
}

#endif // ifdef SQL

/**
 * @brief 升级线程
 *
 */
void MultiTask::thread_update()
{
    debug("%s\n", "This is update_thread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/**
 * @brief 看门狗控制线程
 * 
 */
void MultiTask::thread_watchdogctl()
{
    char buf[] = {1};
    int fd = open("/dev/watchdogctl", O_RDWR);
    if (fd == -1)
    {
        perror("open watchdogctl error");
        close(fd);
        return;
    }
    while (1)
    {
        if (write(fd, buf, sizeof(buf)) < 0)
        {
            break;
        }
        sleep(10);
        buf[0] = buf[0] ? 0 : 1;

        debug("watchdog 输出值 >> %d\n", buf[0]);
    }

    close(fd);
}

#endif // test


/**
 * @brief 故障上报线程
 * 
 */
void MultiTask::thread_fault_info_upload()
{

    while(1)
    {
        /* ######## 上锁 ######## */
        std::unique_lock<std::mutex> db_lock(m_dev_fault_platform);

        /* 阻塞等待是否有故障信息要上传 */
        while(!is_upload_fault_info_to_platform)
        {
            cv_dev_fault_platform.wait(db_lock);
        }

         
        
    }
}

/**
 * @brief 用于在与井口通讯线程中状态机的交互过程
 *
 * @param curr_info 用于指向状态机的当前状态信息
 * @param data_block 用于指向存储站号的一组数据
 * @return int 成功：1；超时：0；出错：-1
 */
int MultiTask::state_machine_operation(
    struct state_machine_current_info *curr_info,
    struct data_block *data_block)
{
    /* 保存CRC校验结果的临时变量 */
    int crcCode = 0;

    int ret = 0;

    int j = 0;

    /* 保存允许出错的次数 */
    int err_num = 0;

    /* 保存接收到的64位从机地址 */
    uint64_t slave_addr64 = 0;

    /* 用于组装发送给ZigBee的数据 */
    uint8_t to_xbee_data[8] = {0};

    /* 8 to 16 */
    uint16_t uint8To16 = 0;

    /* 用于接收井口 RTU 发送的数据 */
    uint8_t xbee_rtu_data[MODBUS_RTU_MAX_ADU_LENGTH] = {0};

    /* 接收到的 RTU 数据长度 */
    int xbeeRtulen = 0;

    /* 用于保存错误类型 */
    int err_type = 0;

    XBeeAddress64 addr64;

    /* ------ 开始组装RTU数据 ------ */

    /* 要发送的站号 */
    if (current_info.store_type == TYPE_VALVE_GROUP_DATA)
    {
        switch (curr_info->id)
        {
        case 128:
            to_xbee_data[0] = 16;
            break;
        case 127:
            to_xbee_data[0] = 15;
            break;
        case 126:
            to_xbee_data[0] = 14;
            break;
        case 125:
            to_xbee_data[0] = 13;
            break;
        default:
            break;
        }
    }
    else
    {
        to_xbee_data[0] = curr_info->id;
    }

    /* 要发送的64位地址 */

    addr64 = XBeeAddress64(
        get_cfg().well_info[curr_info->id_addr].addr >> 32,
        get_cfg().well_info[curr_info->id_addr].addr & 0xffffffff);

    to_xbee_data[1] = curr_info->func_code;
    to_xbee_data[2] = curr_info->start_addr >> 8;
    to_xbee_data[3] = curr_info->start_addr & 0xff;
    to_xbee_data[4] = curr_info->addr_len >> 8;
    to_xbee_data[5] = curr_info->addr_len & 0xff;
    crcCode = crc16(to_xbee_data, 6);
    to_xbee_data[6] = crcCode >> 8;
    to_xbee_data[7] = crcCode & 0xFF;

    debug("%s\n", "发送的数据帧>>");

    /* 出现错误时，重新发送并接收一遍数据，当错误超过两次时，便放弃当前站号数据，采下一组站号数据 */
    while (err_num < 2)
    {
        /* ############ 上锁 ############ */
        m_tx_rx.lock();

        /* 发送数据 */
        xbeeTx(*xbee_handler, to_xbee_data, 8, addr64, NO_RESPONSE_FRAME_ID);

        debug("%s\n", "");
        debug("%s\n", "接收的数据帧>>");

        /* 等待接收数据 */
        ret = xbeeRx(*xbee_handler, xbee_rtu_data, &xbeeRtulen, &slave_addr64);

        /* ############ 解锁 ########### */
        m_tx_rx.unlock();

        debug("tx len >> %02x    rx len >> %02x\n", curr_info->addr_len * 2,
              (uint16_t)xbee_rtu_data[2]);

        if (ret == 0)
        {
            err_type = ERROR_XBEE_RECV_TIMEOUT;
            err_num++;
            continue;
        }
        else if ((ret < 0) ||
                 ((curr_info->addr_len * 2) != (uint16_t)xbee_rtu_data[2]))
        {
            err_type = ERROR_XBEE_RECV_DATA;
            err_num++;
            continue;
        }

        break;
    }

    if (err_num >= 2)
    {
        if (err_type == ERROR_XBEE_RECV_TIMEOUT)
        {
            debug("站号(%d)>>地址(%d)-长度(%d)数据接收超时-------------->>\n",
                  curr_info->id, curr_info->start_addr + 1, curr_info->addr_len);

            //curr_info->phase = PHASE_START;
            //curr_info->isGetTime = false;
            ret = 0;
            //return ret;
        }
        else if (err_type == ERROR_XBEE_RECV_DATA)
        {
            debug("站号(%d)>>地址(%d)-长度(%d)数据接收出错-------------->>\n",
                  curr_info->id, curr_info->start_addr + 1, curr_info->addr_len);

            ret = -1;
        }

        if (curr_info->store_type == TYPE_OIL_WELL_DATA)
        {
            if ((curr_info->start_addr >= 0) &&
                (curr_info->start_addr < 200))
            {
                for (int i = curr_info->start_addr;
                     i < curr_info->start_addr + curr_info->addr_len;
                     i++)
                {
                    data_block[curr_info->id_addr].oil_basic_data[i] = 0;
                }
            }
        }
        else if (curr_info->store_type == TYPE_WATER_WELL_DATA)
        {
            if ((curr_info->start_addr >= 0) &&
                (curr_info->start_addr < 100))
            {
                for (int i = curr_info->start_addr;
                     i < curr_info->start_addr + curr_info->addr_len;
                     i++)
                {
                    data_block[curr_info->id_addr].water_well_data[i] = 0;
                }
            }
        }
        else if (curr_info->store_type == TYPE_MANIFOLD_PRESSURE_1)
        {
            wellsite_info.manifold_pressure[0] = 0;
        }
        else if (curr_info->store_type == TYPE_MANIFOLD_PRESSURE_2)
        {
            wellsite_info.manifold_pressure[1] = 0;
        }
        else if (curr_info->store_type == TYPE_VALVE_GROUP_DATA)
        {
            if ((curr_info->start_addr >= 0) &&
                (curr_info->start_addr < 10))
            {
                for (int i = curr_info->start_addr;
                     i < curr_info->start_addr + curr_info->addr_len;
                     i++)
                {
                    data_block[curr_info->id_addr].valve_group_data[i] = 0;
                }
            }
            for (int i = 0;
                 i < to_db.wr_db[curr_info->id_addr].injection_well_num;
                 i++)
            {
                if (curr_info->start_addr == 0x0e + i * 0xa)
                {
                    for (int i = curr_info->start_addr - 8 * i;
                         i < curr_info->start_addr + curr_info->addr_len;
                         i++)
                    {
                        data_block[curr_info->id_addr].valve_group_data[i] = 0;
                    }
                }
            }
        }
        else if (curr_info->store_type == TYPE_INDICATION_DIAGRAM)
        {
            if ((curr_info->start_addr >= 210) &&
                (curr_info->start_addr < 610))
            {
                for (int j = 0; j < 800; j++)
                {
                    data_block[curr_info->id_addr].ind_diagram[j] = 0;
                }
                ret = -2;
            }
            else if ((curr_info->start_addr >= 610) &&
                     (curr_info->start_addr < 1010))
            {
                for (int j = 0; j < 400; j++)
                {
                    data_block[curr_info->id_addr].ind_diagram[j + 400] = 0;
                }
                ret = -3;
            }
        }
        else if (curr_info->store_type == TYPE_POWER_DIAGRAM)
        {
            if ((curr_info->start_addr >= 1010) &&
                (curr_info->start_addr < 1210))
            {
                for (int j = 0; j < 400; j++)
                {
                    data_block[curr_info->id_addr].ind_diagram[j] = 0;
                }
                ret = -4;
            }
            else if ((curr_info->start_addr >= 1210) &&
                     (curr_info->start_addr < 1410))
            {
                for (int j = 0; j < 200; j++)
                {
                    data_block[curr_info->id_addr].ind_diagram[j + 200] = 0;
                }
                ret = -5;
            }
        }

        memset(xbee_rtu_data, 0, sizeof(xbee_rtu_data));
    }
    else
    {
        /* 保存目标的64位地址 */
        if (get_cfg().well_info[curr_info->id_addr].addr == 0xffff)
        {
            config_info.well_info[curr_info->id_addr].addr = slave_addr64;
        }

        /* 将数据存入临时缓存 */
        for (int i = 0; i < xbeeRtulen - 5; i += 2)
        {
            uint8To16 = (xbee_rtu_data[i + 3] << 8) |
                        (xbee_rtu_data[i + 4] & 0xff);

            if (curr_info->store_type == TYPE_OIL_WELL_DATA)
            {
                /* 当前基础数据块存放寄存器1~210以及41411~41420的数据 */
                /* 顺序上是连续的，共包含220个寄存器数据 */
                /* 偏移量为-1,基础数据在0~199位 */
                /* 功图基础数据第二部分在第200~209位，功图基础数据第一部分在210~219位 */
                data_block[curr_info->id_addr].oil_basic_data.push_back(uint8To16);
            }

            else if (curr_info->store_type == TYPE_WATER_WELL_DATA)
            {
                data_block[curr_info->id_addr].water_well_data.push_back(uint8To16);
            }
            else if (curr_info->store_type == TYPE_MANIFOLD_PRESSURE_1)
            {
                if (get_cfg().manifold_1.add == j)
                {
                    wellsite_info.manifold_pressure[0] = (uint8To16 * 0.0145 - 141.12) / 1;
                    debug("保存汇管1数据[%d]地址[%d]\n", wellsite_info.manifold_pressure[0], i);
                }
                j++;
            }
            else if (curr_info->store_type == TYPE_MANIFOLD_PRESSURE_2)
            {
                if (get_cfg().manifold_1.add == j)
                {
                    wellsite_info.manifold_pressure[1] = (uint8To16 * 0.0145 - 141.12) / 1;
                    debug("保存汇管2数据[%d]地址[%d]\n", wellsite_info.manifold_pressure[1], i);
                }
                j++;
            }
            else if (curr_info->store_type == TYPE_VALVE_GROUP_DATA)
            {
                data_block[curr_info->id_addr].valve_group_data.push_back(uint8To16);
            }
            else if (curr_info->store_type == TYPE_INDICATION_DIAGRAM)
            {
                data_block[curr_info->id_addr].ind_diagram.push_back(uint8To16);
            }
            else if (curr_info->store_type == TYPE_POWER_DIAGRAM)
            {
                data_block[curr_info->id_addr].power_diagram.push_back(uint8To16);
            }
        }

        ret = 1;
    }

    return ret;
}

/**
 * @brief 将临时缓存区中的数据返给上位机
 *
 * @param tcp_data 指向保存上位机数据的结构体
 * @return int 成功：0；失败：-1
 */
int MultiTask::sel_data_to_tcp(struct tcp_data *tcp_data)
{
    int ret = 0;

    /* 站号对应的位置 */
    int id_addr = 0;

    /* 用于判断是否是从井场40051(汇管压力)开始读取多位寄存器值 */
    //int st = 0;

    /* 用于保存阀组间寄存器起始地址与100的整除值 */
    //int ch = 0;

    /* 用于保存超出所在区间的个数 */
    int dif = 0;

    /* 用于保存实际要读取所在区间的寄存器的个数*/
    int dif_len = 0;

    /* 将最新故障信息列表保存到当前故障信息列表 */
    static vector<device_fault_info> current_dev_fault_info;

    /* 用于组合发送给上位机的数据帧 */
    std::vector<uint8_t> to_tcp_frame;

    /* 用于保存在读取错误信息列表数据时是否出现错误 */
    int is_read_dev_fault_err = 0;

    /* 用于保存要进行写操作的通信方式 */
    int write_comm_type = 0;

    /* 判断站号是否在当前配置项里，没有则退出当前函数 */
    while (id_addr < get_cfg().well_max_num)
    {
        if (tcp_data->id == 222)
        {
            break;
        }

        if (tcp_data->id != get_cfg().well_info[id_addr].id)
        {
            id_addr++;
            continue;
        }
        break;
    }

    if (id_addr >= get_cfg().well_max_num)
    {
        return -1;
    }

    /* 用于组装发送给上位机的modbus TCP数据帧 */
    /* 组装帧头 */
    to_tcp_frame.push_back(tcp_data->frame_header[0]);
    to_tcp_frame.push_back(tcp_data->frame_header[1]);
    to_tcp_frame.push_back(tcp_data->frame_header[2]);
    to_tcp_frame.push_back(tcp_data->frame_header[3]);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) >> 8);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) & 0xff);

    /* 组装站号，功能码 */
    to_tcp_frame.push_back(tcp_data->id);
    to_tcp_frame.push_back(tcp_data->func_code);

    /* 请求读寄存器 */
    if (tcp_data->func_code == 0x03)
    {
        /* 加寄存器长度 */
        to_tcp_frame.push_back((tcp_data->len * 2) & 0xff);

        /* 上位机读取配置信息 */
        if (tcp_data->id == 222)
        {
            if ((tcp_data->start_addr >= 0) &&
                (tcp_data->start_addr <= 69))
            {
                dif = tcp_data->start_addr + tcp_data->len - 70;

                if (dif > 0)
                {
                    dif_len = tcp_data->len - dif;
                }
                else
                {
                    dif_len = tcp_data->len;
                }

                /* 基础数据已经准备好，可以读取并发给上位机 */
                for (uint16_t i = tcp_data->start_addr;
                     i < (tcp_data->start_addr + dif_len); i++)
                {
                    to_tcp_frame.push_back(get_cfg().reg_value[i] >> 8);
                    to_tcp_frame.push_back(get_cfg().reg_value[i] & 0xff);
                }

                if (dif > 0)
                {
                    for (int i = 0; i < dif; i++)
                    {
                        to_tcp_frame.push_back(0);
                        to_tcp_frame.push_back(0);
                    }
                }
            }
        }

        /* 油井数据类型 */
        else if (get_cfg().well_info[id_addr].type == TYPE_OIL_WELL_DATA)
        {
            if ((tcp_data->start_addr >= 0) &&
                (tcp_data->start_addr < 200))
            {
                dif = tcp_data->start_addr + tcp_data->len - 200;

                if (dif > 0)
                {
                    dif_len = tcp_data->len - dif;
                }
                else
                {
                    dif_len = tcp_data->len;
                }

                /* ###### 上锁 ###### */
                m_rd_db.lock();

                /* 基础数据已经准备好，可以读取并发给上位机 */
                for (uint16_t i = tcp_data->start_addr;
                     i < (tcp_data->start_addr + dif_len); i++)
                {
                    to_tcp_frame.push_back(
                        to_db.rd_db[id_addr].oil_basic_data[i] >> 8);
                    to_tcp_frame.push_back(
                        to_db.rd_db[id_addr].oil_basic_data[i] & 0xff);
                }

                if ((dif > 0) && (dif <= 200))
                {
                    /* 功图数据 */
                    for (uint16_t i = 0; i < (0 + dif); i++)
                    {
                        to_tcp_frame.push_back(
                            to_db.rd_db_ind[id_addr].ind_diagram[i] >> 8);
                        to_tcp_frame.push_back(
                            to_db.rd_db_ind[id_addr].ind_diagram[i] & 0xff);
                    }
                }

                /* ###### 解锁 ###### */
                m_rd_db.unlock();
            }
            else if ((tcp_data->start_addr >= 200) &&
                     (tcp_data->start_addr < 1010))
            {
                dif = tcp_data->start_addr + tcp_data->len - 1010;

                if (dif > 0)
                {
                    dif_len = tcp_data->len - dif;
                }
                else
                {
                    dif_len = tcp_data->len;
                }

                /* ###### 上锁 ###### */
                m_rd_db.lock();

                /* 功图已经准备好，可以读取并发给上位机 */
                for (uint16_t i = tcp_data->start_addr - 200;
                     i < (tcp_data->start_addr - 200 + dif_len); i++)
                {
                    to_tcp_frame.push_back(
                        to_db.rd_db_ind[id_addr].ind_diagram[i] >> 8);
                    to_tcp_frame.push_back(
                        to_db.rd_db_ind[id_addr].ind_diagram[i] & 0xff);
                }

                if ((dif > 0) && (dif < 200))
                {
                    /* 功率图数据 */
                    for (uint16_t i = 10; i < (10 + dif); i++)
                    {
                        to_tcp_frame.push_back(
                            to_db.rd_db_ind[id_addr].power_diagram[i] >> 8);
                        to_tcp_frame.push_back(
                            to_db.rd_db_ind[id_addr].power_diagram[i] & 0xff);
                    }
                }

                /* ###### 解锁 ###### */
                m_rd_db.unlock();
            }
            else if ((tcp_data->start_addr >= 1010) &&
                     (tcp_data->start_addr < 1410))
            {
                dif = tcp_data->start_addr + tcp_data->len - 1410;

                if (dif > 0)
                {
                    dif_len = tcp_data->len - dif;
                }
                else
                {
                    dif_len = tcp_data->len;
                }

                /* ###### 上锁 ###### */
                m_rd_db.lock();

                /* 功率图数据第已经准备好，可以读取并发给上位机 */
                for (uint16_t i = tcp_data->start_addr - 1000;
                     i < (tcp_data->start_addr - 1000 + dif_len); i++)
                {
                    to_tcp_frame.push_back(
                        to_db.rd_db_ind[id_addr].power_diagram[i] >> 8);
                    to_tcp_frame.push_back(
                        to_db.rd_db_ind[id_addr].power_diagram[i] & 0xff);
                }

                if ((dif > 0) && (dif <= 10))
                {
                    /* 功率图基础数据 */
                    for (uint16_t i = 0; i < (0 + dif); i++)
                    {
                        to_tcp_frame.push_back(
                            to_db.rd_db_ind[id_addr].power_diagram[i] >> 8);
                        to_tcp_frame.push_back(
                            to_db.rd_db_ind[id_addr].power_diagram[i] & 0xff);
                    }
                }
                else if (dif > 10)
                {
                    /* 补零 */
                    for (uint16_t i = 0; i < (dif - 10); i++)
                    {
                        to_tcp_frame.push_back(0);
                        to_tcp_frame.push_back(0);
                    }
                }

                /* ###### 解锁 ###### */
                m_rd_db.unlock();
            }
            else if ((tcp_data->start_addr >= 1410) &&
                     (tcp_data->start_addr < 1420))
            {
                dif = tcp_data->start_addr + tcp_data->len - 1420;

                if (dif > 0)
                {
                    dif_len = tcp_data->len - dif;
                }
                else
                {
                    dif_len = tcp_data->len;
                }

                /* #######上锁 ###### */
                m_rd_db.lock();

                /* 功图基础数据第二部分已经准备好，可以读取并发给上位机 */
                for (uint16_t i = tcp_data->start_addr - 1410;
                     i < (tcp_data->start_addr - 1410 + dif_len); i++)
                {
                    to_tcp_frame.push_back(
                        to_db.rd_db_ind[id_addr].power_diagram[i] >> 8);
                    to_tcp_frame.push_back(
                        to_db.rd_db_ind[id_addr].power_diagram[i] & 0xff);
                }

                /* ###### 解锁 ###### */
                m_rd_db.unlock();

                if (dif > 0)
                {
                    for (int i = 0; i < dif; i++)
                    {
                        to_tcp_frame.push_back(0);
                        to_tcp_frame.push_back(0);
                    }
                }
            }
            else
            {
                /* 请求读取的数据不在任意对应区间的，一律返回零 */
                for (int i = 0; i < tcp_data->len; i++)
                {
                    to_tcp_frame.push_back(0);
                    to_tcp_frame.push_back(0);
                }
            }
        }

        /* 水源井数据类型 */
        else if (get_cfg().well_info[id_addr].type == TYPE_WATER_WELL_DATA)
        {
            /* 判断查询的数据为哪一部分 */
            if ((tcp_data->start_addr >= 0) &&
                (tcp_data->start_addr <= 29))
            {
                dif = tcp_data->start_addr + tcp_data->len - 30;
                if (dif > 0)
                {
                    dif_len = tcp_data->len - dif;
                }
                else
                {
                    dif_len = tcp_data->len;
                }

                /* ###### 上锁 ###### */
                m_rd_db.lock();

                /* 水源井数据已经准备好，可以读取并发给上位机 */
                for (uint16_t i = tcp_data->start_addr;
                     i < (tcp_data->start_addr + dif_len); i++)
                {
                    to_tcp_frame.push_back(
                        to_db.rd_db[id_addr].water_well_data[i] >> 8);
                    to_tcp_frame.push_back(
                        to_db.rd_db[id_addr].water_well_data[i] & 0xff);
                }

                /* ###### 解锁 ###### */
                m_rd_db.unlock();

                if (dif > 0)
                {
                    for (int i = 0; i < dif; i++)
                    {
                        to_tcp_frame.push_back(0);
                        to_tcp_frame.push_back(0);
                    }
                }
            }
            else
            {
                /* 请求读取的数据不在任意对应区间的，一律返回零 */
                for (int i = 0; i < tcp_data->len; i++)
                {
                    to_tcp_frame.push_back(0);
                    to_tcp_frame.push_back(0);
                }
            }
        }

        /* 阀组间数据以及井场RTU数据类型 */
        else if (((get_cfg().well_info[id_addr].type >> 4) == TYPE_VALVE_GROUP_DATA) ||
                 ((get_cfg().well_info[id_addr].id == 128) &&
                  (get_cfg().well_info[id_addr].type == 0)))
        {
            /* ######## 新增设备故障处理机制 wgm-2020-10-21 ######## */
            if ((get_cfg().well_info[id_addr].id == 128) &&
                (tcp_data->start_addr >= 0) &&
                (tcp_data->start_addr <= 52) &&
                ((tcp_data->start_addr + tcp_data->len) >= 56))
            {
                /* 当前组故障信息列表是否已发送完 */
                if (current_fault_type_num == 0)
                {
                    /* ###### 上锁 ###### */
                    m_dev_fault.lock();

                    /* 最新组故障信息列表中是否有故障信息 */
                    if (dev_fault_info.size() > 0)
                    {
                        /* 拷贝最新组故障信息列表到当前组故障信息列表 */
                        current_dev_fault_info.assign(dev_fault_info.begin(),
                                                      dev_fault_info.end());
                        /* 重置当前组故障信息列表数量 */
                        current_dev_type_num = current_dev_fault_info.size();
                        /* 重置当前故障设备中的故障数量 */
                        if (current_dev_type_num > 0)
                        {
                            current_fault_type_num =
                                current_dev_fault_info[current_dev_type_num - 1].fault_type.size();
                            if (current_fault_type_num > 0)
                            {
                                /* 将当前组故障信息依次放到井场主RTU寄存器 40053~40057中 */

                                /* 故障设备代码 */
                                wellsite_info.fault_info[0] =
                                    current_dev_fault_info[current_dev_type_num - 1].device_type;

                                /* 故障部位代码 */
                                wellsite_info.fault_info[1] =
                                    current_dev_fault_info[current_dev_type_num - 1].fault_type[current_fault_type_num - 1];

                                /* 故障时间 */
                                wellsite_info.fault_info[2] =
                                    current_dev_fault_info[current_dev_type_num - 1].time[0];
                                wellsite_info.fault_info[3] =
                                    current_dev_fault_info[current_dev_type_num - 1].time[1];
                                wellsite_info.fault_info[4] =
                                    current_dev_fault_info[current_dev_type_num - 1].time[2];

                                if (--current_fault_type_num == 0)
                                {
                                    if (--current_dev_type_num > 0)
                                    {
                                        current_fault_type_num =
                                            current_dev_fault_info[current_dev_type_num - 1].fault_type.size();
                                    }
                                }
                            }
                            else
                            {
                                is_read_dev_fault_err = -1;
                            }
                        }
                        else
                        {
                            is_read_dev_fault_err = -1;
                        }
                    }
                    else
                    {
                        is_read_dev_fault_err = 0;
                    }

                    /* ###### 解锁 ###### */
                    m_dev_fault.unlock();
                }
                else if (current_fault_type_num > 0)
                {
                    /* 判断当前设备类型中的故障是否发送完 */
                    if (current_fault_type_num == 0)
                    {
                        /* 重置当前故障设备中的故障数量 */
                        if (--current_dev_type_num > 0)
                        {
                            current_fault_type_num =
                                current_dev_fault_info[current_dev_type_num - 1].fault_type.size();
                        }
                    }
                    if (current_fault_type_num > 0)
                    {
                        /* 故障设备代码 */
                        wellsite_info.fault_info[0] =
                            current_dev_fault_info[current_dev_type_num - 1].device_type;

                        /* 故障部位代码 */
                        wellsite_info.fault_info[1] =
                            current_dev_fault_info[current_dev_type_num - 1].fault_type[current_fault_type_num - 1];

                        /* 故障时间 */
                        wellsite_info.fault_info[2] =
                            current_dev_fault_info[current_dev_type_num - 1].time[0];
                        wellsite_info.fault_info[3] =
                            current_dev_fault_info[current_dev_type_num - 1].time[1];
                        wellsite_info.fault_info[4] =
                            current_dev_fault_info[current_dev_type_num - 1].time[2];

                        current_fault_type_num--;
                        if (current_fault_type_num == 0)
                        {
                            current_dev_type_num--;
                            /* 重置当前故障设备中的故障数量 */
                            current_fault_type_num =
                                current_dev_fault_info[current_dev_type_num - 1].fault_type.size();
                        }
                    }
                    else
                    {
                        is_read_dev_fault_err = -1;
                    }
                }
                else
                {
                    is_read_dev_fault_err = -1;
                }

                if (is_read_dev_fault_err <= 0)
                {
                    /* 故障设备代码 */
                    wellsite_info.fault_info[0] = 0;

                    /* 故障部位代码 */
                    wellsite_info.fault_info[1] = 0;

                    /* 故障时间 */
                    wellsite_info.fault_info[2] = 0;
                    wellsite_info.fault_info[3] = 0;
                    wellsite_info.fault_info[4] = 0;
                }
            }
            /* ######## END wgm-2020-10-21 ######## */

            /* ###### 上锁 ###### */
            m_rd_db.lock();

            if ((tcp_data->start_addr >= 0) &&
                (tcp_data->start_addr <= 59))
            {
                dif = tcp_data->start_addr + tcp_data->len - 60;
                if (dif > 0)
                {
                    dif_len = tcp_data->len - dif;
                }
                else
                {
                    dif_len = tcp_data->len;
                }

                for (uint16_t i = tcp_data->start_addr;
                     i < (tcp_data->start_addr + dif_len); i++)
                {
                    to_tcp_frame.push_back(
                        to_db.rd_db[id_addr].wellsite_rtu[i] >> 8);
                    to_tcp_frame.push_back(
                        to_db.rd_db[id_addr].wellsite_rtu[i] & 0xff);
                }

                if ((dif > 0) && (dif <= 40))
                {
                    for (int i = 0; i < dif; i++)
                    {
                        to_tcp_frame.push_back(0);
                        to_tcp_frame.push_back(0);
                    }
                }
                else if ((dif > 40) && (dif <= 46))
                {
                    for (int i = 0; i < 40; i++)
                    {
                        to_tcp_frame.push_back(0);
                        to_tcp_frame.push_back(0);
                    }

                    for (uint16_t i = 60;
                         i < (60 + dif - 40); i++)
                    {
                        to_tcp_frame.push_back(
                            to_db.rd_db[id_addr].wellsite_rtu[i] >> 8);
                        to_tcp_frame.push_back(
                            to_db.rd_db[id_addr].wellsite_rtu[i] & 0xff);
                    }
                }
                else if (dif > 46)
                {
                    for (int i = 0; i < 40; i++)
                    {
                        to_tcp_frame.push_back(0);
                        to_tcp_frame.push_back(0);
                    }

                    for (uint16_t i = 60;
                         i < (60 + 6); i++)
                    {
                        to_tcp_frame.push_back(
                            to_db.rd_db[id_addr].wellsite_rtu[i] >> 8);
                        to_tcp_frame.push_back(
                            to_db.rd_db[id_addr].wellsite_rtu[i] & 0xff);
                    }

                    for (int i = 0; i < (dif - 46); i++)
                    {
                        to_tcp_frame.push_back(0);
                        to_tcp_frame.push_back(0);
                    }
                }
            }
            else if (tcp_data->start_addr >= 100)
            {
                for (uint8_t i = 1; i <= to_db.rd_db[id_addr].injection_well_num; i++)
                {
                    if ((tcp_data->start_addr >= (100 * i)) &&
                        (tcp_data->start_addr <= (100 * i + 6)))
                    {
                        dif = tcp_data->start_addr + tcp_data->len - (100 * i + 7);
                        if (dif > 0)
                        {
                            dif_len = tcp_data->len - dif;
                        }
                        else
                        {
                            dif_len = tcp_data->len;
                        }

                        for (uint16_t j = tcp_data->start_addr;
                             j < (tcp_data->start_addr + dif_len); j++)
                        {
                            to_tcp_frame.push_back(
                                to_db.rd_db[id_addr].wellsite_rtu[(i - 1) * 6 + 60 + (j & 0xf)] >> 8);
                            to_tcp_frame.push_back(
                                to_db.rd_db[id_addr].wellsite_rtu[(i - 1) * 6 + 60 + (j & 0xf)] & 0xff);
                        }

                        if (dif > 0)
                        {
                            for (int m = 0; m < dif; m++)
                            {
                                to_tcp_frame.push_back(0);
                                to_tcp_frame.push_back(0);
                            }
                        }

                        break;
                    }
                }
            }
            else
            {
                /* 请求读取的数据不在任意对应区间的，一律返回零 */
                for (int i = 0; i < tcp_data->len; i++)
                {
                    to_tcp_frame.push_back(0);
                    to_tcp_frame.push_back(0);
                }
            }

            /* ###### 解锁 ###### */
            m_rd_db.unlock();
        }
    }
    /* 请求写单个线圈/单个寄存器/多个线圈/多个寄存器 */
    else if ((tcp_data->func_code == 0x05) ||
             (tcp_data->func_code == 0x06) ||
             (tcp_data->func_code == 0x0f) ||
             (tcp_data->func_code == 0x10))
    {
        if ((get_cfg().well_info[id_addr].type == CON_WIRELESS) ||
            (get_cfg().well_info[id_addr].type == CON_VALVE_ZIGBEE))
        {
            write_comm_type = COMM_XBEE;
        }
        else if ((get_cfg().well_info[id_addr].type == CON_WIRED) ||
                 (get_cfg().well_info[id_addr].type == CON_VALVE_485))
        {
            write_comm_type = COMM_485;
        }
        else if (tcp_data->id == 222)
        {
            //TODO：根据商定的协议进行代码编写
            write_comm_type = COMM_RTU;
        }

        if (write_comm_type > 0)
        {
            if (modbus_write(write_comm_type, tcp_data) < 0)
            {
                return -1;
            }
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    /* 数据异常，发送0 */
    if (ret == -1)
    {
        uint8_t n = 0;

        for (int i = 0; i < tcp_data->len * 2; i++)
        {
            to_tcp_frame.push_back(n);
        }
    }

    debug("%s\n", "send tcp--------------------");

    for (int i = 0; i < int(to_tcp_frame.size()); i++)
    {
        debug("[%.2x]", to_tcp_frame[i]);
    }
    debug("%s\n", "");

    string to_tcp(to_tcp_frame.begin(), to_tcp_frame.end());

    /* 发送给上位机 */
    if (send(ctx->s, to_tcp.c_str(), to_tcp_frame.size(),
             MSG_NOSIGNAL) < 0)
    {
        debug("%s\n",
              " modbus TCP send error");
        return -1;
    }
    debug("--- %s ---\n",
          " Modbus TCP has been successfully sent");

    return 0;
}

/**
 * @brief 获取JSON格式的配置信息
 * 
 * @return int 成功：0；失败：-1
 */
int MultiTask::get_jconfig_info()
{
    using namespace gcfg;

#ifdef ARMCQ
    const char *jconfig_path = "/home/config/myconfig.json";
#else
    const char *jconfig_path = "./config/myconfig.json";
#endif

    int wellport = 0;
    int wellport_num = 0;

    /* 创建json对象 */
    jconfig = ConfigSingle::getInstance();

    if (jconfig == nullptr)
    {
        cout << "create config error" << endl;
        return -1;
    }

    if (jconfig->RefreshCfg(jconfig_path) == -1)
    {
        cout << "open config file error" << endl;
        return -1;
    }

    /* 固件版本 */
    config_info.version = jconfig->GetVersion();

    /* rtu名称 */
    config_info.rtu_name = jconfig->GetRtuName();

    /* 端口号 */
    config_info.port = jconfig->GetNetPort();

    /* ip地址 */
    config_info.ip = jconfig->GetNetIP();

    /* 网关 */
    config_info.gateway = jconfig->GetNetGateway();

    /* mac地址 */
    //TODO：后期如无用处，可删
    config_info.mac = jconfig->GetMacAddr();

    /* 子网掩码 */
    config_info.mask = jconfig->GetNetMask();

    /* 汇管压力配置1 */
    config_info.manifold_1.type = jconfig->GetManiFoldCfg(0).cfg >> 12;
    config_info.manifold_1.add = (jconfig->GetManiFoldCfg(0).cfg >> 8) & 0xf;
    config_info.manifold_1.id = jconfig->GetManiFoldCfg(0).cfg & 0xff;
    config_info.manifold_1.range = jconfig->GetManiFoldCfg(0).rng;

    /* 汇管压力配置2 */
    config_info.manifold_2.type = jconfig->GetManiFoldCfg(1).cfg >> 12;
    config_info.manifold_2.add = (jconfig->GetManiFoldCfg(1).cfg >> 8) & 0xf;
    config_info.manifold_2.id = jconfig->GetManiFoldCfg(1).cfg & 0xff;
    config_info.manifold_2.range = jconfig->GetManiFoldCfg(1).rng;

    /* zigbe配置 */
    config_info.xbee_id = jconfig->GetZIGCFG().id;
    config_info.xbee_ce = jconfig->GetZIGCFG().ce;
    config_info.xbee_ao = jconfig->GetZIGCFG().ao;
    config_info.xbee_sc = jconfig->GetZIGCFG().sc;

    /* DI寄存器配置 */
    for (int i = 0; i < 8; i++)
    {
        config_info.di_reg[i] = jconfig->GetDIReg(i);
    }

    /* DO寄存器配置 */
    for (int i = 0; i < 8; i++)
    {
        config_info.do_reg[i] = jconfig->GetDOReg(i);
    }

    /* AI寄存器配置 */
    for (int i = 0; i < 10; i++)
    {
        config_info.ai_cfg[i].reg = jconfig->GetAIReg(i);
        config_info.ai_cfg[i].rng = jconfig->GetAIRng(i);
    }

    /* 网口配置 */
    for (int i = 0; i < 2; i++)
    {
        config_info.eth_cfg[i].ip = jconfig->GetEth(i).ip;
        config_info.eth_cfg[i].mask = jconfig->GetEth(i).mask;
        config_info.eth_cfg[i].gateway = jconfig->GetEth(i).gateway;
    }

    /* wifi配置 */
    config_info.wifi_cfg.ssid = jconfig->GetWifi().ssid;
    config_info.wifi_cfg.password = jconfig->GetWifi().passwd;

    /* 串口(232,485)配置  0->232;1->485;2->485 */
    for (int i = 0; i < 3; i++)
    {
        config_info.serial_cfg[i].baudrate = jconfig->GetSerialCfg(i).baudrate;
        config_info.serial_cfg[i].parity = jconfig->GetSerialCfg(i).parity;
        config_info.serial_cfg[i].databit = jconfig->GetSerialCfg(i).databit;
        config_info.serial_cfg[i].stopbit = jconfig->GetSerialCfg(i).stopbit;
    }

    memset(config_info.well_info, 0, sizeof(config_info.well_info));

    /* 油井，水井，阀组配置 */
    for (int i = 0; i < 16; i++)
    {
        wellport = jconfig->GetWellPortCfgs()[i];

        if ((wellport == TYPE_OIL_WELL_DATA) || (wellport == TYPE_WATER_WELL_DATA)) /* 油井或水水井 */
        {
            config_info.well_info[wellport_num].id = i + 1;
        }
        else if ((i >= 12) && ((wellport >> 4) == TYPE_VALVE_GROUP_DATA)) /* 阀组 */
        {
            config_info.well_info[wellport_num].id = i - 12 + 125;
        }

        if (wellport > 0)
        {
            config_info.well_info[wellport_num].type = wellport;
            config_info.well_info[wellport_num].addr = 0xffff;

            wellport_num++;
        }
    }

    /* 添加站号为128的井场主RTU */
    if (config_info.well_info[wellport_num - 1].id != 128)
    {
        config_info.well_info[wellport_num].id = 128;
        config_info.well_info[wellport_num].type = 0;
        config_info.well_max_num = wellport_num + 1;
    }
    else
    {
        /* 添加配置的井口和阀组的总个数，包含未配置阀组的128站号 */
        config_info.well_max_num = wellport_num;
    }

    return 0;
}

/**
 * @brief 用于通过RS485进行modbus 读数据操作
 * 
 * @param curr_info 当前状态信息
 * @param data_block 用于保存接收到的数据的存储数据区
 * @return int 成功：0；失败：-1
 */
int MultiTask::rs485(struct state_machine_current_info *curr_info,
                     struct data_block *data_block)
{
    int crcCode = 0;

    int ret = 0;

    bool stat = false;

    /* 保存允许出错的次数 */
    int err_num = 0;

    /* 用于组装发送给rs485的数据 */
    uint8_t to_485_data[8] = {0};

    /* 8 to 16 */
    uint16_t uint8To16 = 0;

    /* 用于接收rs485发送的数据 */
    uint8_t from_485_data[MODBUS_RTU_MAX_ADU_LENGTH] = {0};

    XBeeAddress64 addr64;

    /* 开始组装RTU数据 */
    if (curr_info->store_type == TYPE_VALVE_GROUP_DATA)
    {
        // to_485_data[0] = get_cfg().well_info[curr_info->id_addr].id;
        if (get_cfg().well_info[curr_info->id_addr].id == 128)
        {
            to_485_data[0] = 16;
        }
        else if (get_cfg().well_info[curr_info->id_addr].id == 127)
        {
            to_485_data[0] = 15;
        }
        else if (get_cfg().well_info[curr_info->id_addr].id == 126)
        {
            to_485_data[0] = 14;
        }
        else if (get_cfg().well_info[curr_info->id_addr].id == 125)
        {
            to_485_data[0] = 13;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    to_485_data[1] = curr_info->func_code;
    if ((curr_info->func_code == 0x03) || (curr_info->func_code == 0x04))
    {
        to_485_data[2] = curr_info->start_addr >> 8;
        to_485_data[3] = curr_info->start_addr & 0xff;
        to_485_data[4] = curr_info->addr_len >> 8;
        to_485_data[5] = curr_info->addr_len & 0xff;
        crcCode = crc16(to_485_data, 6);
        to_485_data[6] = crcCode >> 8;
        to_485_data[7] = crcCode & 0xFF;
    }

    debug("%s\n", "发送的rs485数据帧>>");

    for (int i = 0; i < 8; i++)
    {
        debug("[%x]", to_485_data[i]);
    }
    debug("%s\n", "");

    /* 出现错误时，重新发送并接收一遍数据，当错误超过两次时，便放弃当前站号数据，采下一组数据 */
    while (err_num < 2)
    {
        /* 发送数据 */
#ifdef ARMCQ
        usart_send_data(rs485_info[0].fd, to_485_data, 8);
#else  // ifdef ARMCQ
        uart_485->Write(to_485_data, 8);
#endif // ifdef ARMCQ

        usleep(20000); // 40ms

        debug("%s\n", "");
        debug("%s\n", "接收的rs485数据帧>>");

        /* 等待接收数据 */

#ifdef ARMCQ
        ret = usart_rev_data(rs485_info[0].fd, from_485_data);
#else  // ifdef ARMCQ
        ret = uart_485->Read(from_485_data, 256, 1000);

        for (int i = 0; i < curr_info->addr_len * 2 + 5; i++)
        {
            debug("%.2x ", from_485_data[i]);
        }
        debug("%s\n", "");
#endif // ifdef ARMCQ

        if (ret <= 0)
        {
            err_num++;
            continue;
        }

        // for (int i = 0; i < ret; i++)
        // {
        //     debug("[%x]", from_485_data[i]);
        // }
        // debug("%s\n", "");
        break;
    }

    if (err_num >= 2)
    {
        debug("%s\n", "rs485 rx error");

        curr_info->phase = PHASE_START;
        curr_info->isGetTime = false;
        stat = false;
    }

    /* 判断接收到的rs485数据是否正确 */
    if ((from_485_data[0] != to_485_data[0]) ||
        (from_485_data[1] != to_485_data[1]) ||
        (from_485_data[2] != curr_info->addr_len * 2))
    {
        debug("%s\n", "recv rs485 data error");
        stat = false;
    }
    else
    {
        stat = true;
    }
    // if (from_485_data[0] != to_485_data[0])
    // {
    //     debug("%s\n", "recv rs485 data error");
    //     return -1;
    // }
    // else
    // {
    //     if (from_485_data[1] != to_485_data[1])
    //     {
    //         debug("%s\n", "recv rs485 data error");
    //         return -1;
    //     }
    //     else
    //     {
    //         if (from_485_data[2] != curr_info->addr_len * 2)
    //         {
    //             debug("%s\n", "recv rs485 data error");
    //             return -1;
    //         }
    //     }
    // }

    if (stat)
    {
        // 将数据存入临时缓存
        for (int i = 0; i < ret - 5; i += 2)
        {
            uint8To16 = (from_485_data[i + 3] << 8) |
                        (from_485_data[i + 4] & 0xff);

            if (curr_info->store_type == TYPE_VALVE_GROUP_DATA)
            {
                data_block[curr_info->id_addr].valve_group_data.push_back(
                    uint8To16);
            }
        }
    }
    else
    {
        // 将0存入临时缓存
        for (int i = 0; i < ret - 5; i += 2)
        {
            uint8To16 = 0;

            if (curr_info->store_type == TYPE_VALVE_GROUP_DATA)
            {
                data_block[curr_info->id_addr].valve_group_data.push_back(
                    uint8To16);
            }
        }
    }

    return 0;
}

/**
 * @brief 获取转换后的AI值
 * 
 * @param spi_ai 保存ai信息的spi结构体对象
 * @param addr 要读取的位置 0~9
 * @param range 量程
 * @return uint16_t 转换后的AI值
 */
uint16_t MultiTask::get_ai_value(struct spi_handler spi_ai, uint8_t addr, uint16_t range)
{
    /* 用于保存从AI口得到的电流值 */
    double ai_current = 0;

    /* 用于保存AI的转换值 */
    uint16_t ai_change = 0;

    for (int i = 0; i < 2; i++)
    {
        if (spi_transfer(spi_ai.spi,
                         spi_ai.buf_tx[addr],
                         spi_ai.buf_rx,
                         sizeof(spi_ai.buf_rx)) < 0)
        {
            fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi_ai.spi));
            return -1;
        }
    }

    ai_current =
        (((spi_ai.buf_rx[0] << 8) | (spi_ai.buf_rx[1] & 0xff)) & 0xfff) * 0.005055147;

    //如果读取的为汇管数据，则通过AI口得到的电流值在4~20mA之间，
    //再根据所给量程（扩大100倍后），得到要发送给上位机的汇管数据（单位MPa）
    ai_change = (ai_current - 4) / 16 * range;

    return ai_change;
}

/**
 * @brief 故障设备处理
 * 
 * @param is_fault 当前设备是否有故障
 * @param dev_fault 保存当前故障信息
 */
void MultiTask::manage_fault_device(bool is_fault, struct device_fault_info dev_fault)
{

    /* 用来保存故障信息中出现故障的数量 */
    int fault_device_num = 0;

    /* 用于保存当前设备是否在故障设备信息中 */
    bool is_find_dev = false;
    /* 用于保存当前设备在故障设备信息中的位置 */
    int dev_path = 0;

    fault_device_num = dev_fault_info.size();

    /* 判断当前设备是否有故障 */
    if (is_fault)
    {
        /* 判断故障设备信息列表中是否存有故障信息 */
        if (fault_device_num > 0)
        {
            /* 轮询查找该设备是否在故障信息列表里 */
            for (int i = 0; i < fault_device_num; i++)
            {
                if (dev_fault_info[i].device_type == dev_fault.device_type)
                {
                    dev_path = i;
                    is_find_dev = true;
                    break;
                }
            }

            /* 判断当前设备是否已经在故障设备信息中 */
            if (is_find_dev)
            {
                /* 删除原有的故障类型 */
                dev_fault_info[dev_path].fault_type.clear();

                /* 更新新的故障类型信息 */
                dev_fault_info[dev_path].fault_type.assign(dev_fault.fault_type.begin(),
                                                           dev_fault.fault_type.end());
                dev_fault_info[dev_path].time[0] = dev_fault.time[0];
                dev_fault_info[dev_path].time[1] = dev_fault.time[1];
                dev_fault_info[dev_path].time[2] = dev_fault.time[2];
            }
            else
            {
                /* 新增故障信息 */
                dev_fault_info.push_back(dev_fault);
            }
        }
        else
        {
            /* 新增故障信息，并将故障信息加入故障信息列表中 */
            dev_fault_info.push_back(dev_fault);
        }
    }
    else
    {
        /* 判断故障设备信息列表中是否存有故障信息 */
        if (fault_device_num > 0)
        {
            /* 轮询查找该设备是否在故障信息列表里 */
            for (int i = 0; i < fault_device_num; i++)
            {
                if (dev_fault_info[i].device_type == dev_fault.device_type)
                {
                    dev_path = i;
                    is_find_dev = true;
                    break;
                }
            }

            /* 判断当前设备是否已经在故障设备信息中 */
            if (is_find_dev)
            {
                /* 从故障信息列表中删除当前油井故障信息 */
                dev_fault_info.erase(dev_fault_info.begin() + dev_path);
            }
        }
    }
}

/**
 * @brief 发送modbus写指令(0x05,0x06,0x0f,0x10),并将结果返回给上位机
 * 
 * @param comm_type 进行数据通讯的方式
 * @param tcp_data TCP要写入的数据
 * @return int 成功：0；失败：-1
 */
int MultiTask::modbus_write(int comm_type, struct tcp_data *tcp_data)
{
    /* 用于保存要发送的modbus数据帧 */
    uint8_t send_frame[20];
    /* 用于保存接收到的数据帧 */
    uint8_t recv_frame[15];
    /* 用于保存crc校验码 */
    uint16_t crc_code = 0;
    /* 用于保存要发送的modbus帧长度 */
    int send_frame_len = 0;
    /* 用于保存读取返回值 */
    int ret = 0;
    /* 用于保存接收到的数据长度 */
    int recv_len = 0;
    /* 用于保存接收到的xbee64位地址 */
    uint64_t recv_addr64 = 0;
    /* 用于保存允许出错的数量 */
    int err_num = 0;

    /* 组装站号 */
    switch (tcp_data->id)
    {
    case 128:
        send_frame[0] = 16;
        break;
    case 127:
        send_frame[0] = 15;
        break;
    case 126:
        send_frame[0] = 14;
        break;
    case 125:
        send_frame[0] = 13;
        break;
    default:
        send_frame[0] = tcp_data->id;
    }
    /* 组装功能码 */
    send_frame[1] = tcp_data->func_code;
    /* 组装起始地址 */
    if ((tcp_data->id >= 125) && (tcp_data->id <= 128))
    {
        send_frame[2] = ((tcp_data->start_addr - 2) / 100) >> 8;
        send_frame[3] = ((tcp_data->start_addr - 2) / 100) & 0xff;
    }
    else
    {
        send_frame[2] = tcp_data->start_addr >> 8;
        send_frame[3] = tcp_data->start_addr & 0xff;
    }

    /* 写单个线圈或单个寄存器 */
    if ((tcp_data->func_code == 0x05) ||
        (tcp_data->func_code == 0x06))
    {
        /* 组装要写入寄存器的值 */
        send_frame[4] = tcp_data->set_val >> 8;
        send_frame[5] = tcp_data->set_val & 0xff;
        /* 组装CRC校验 */
        crc_code = crc16(send_frame, 6);
        send_frame[6] = crc_code >> 8;
        send_frame[7] = crc_code & 0xff;
        /* 要发送的帧长度 */
        send_frame_len = 8;
    }
    /* 写多个线圈或多个寄存器 */
    else if ((tcp_data->func_code == 0x0f) ||
             (tcp_data->func_code == 0x10))
    {
        /* 组装要写入寄存器的个数 */
        send_frame[4] = tcp_data->len >> 8;
        send_frame[5] = tcp_data->len & 0xff;
        /* 组装要写入数据的总字节数 */
        send_frame[6] = tcp_data->byte_count;

        if (tcp_data->func_code == 0x0f)
        {
            /* 组装要写入寄存器的值 */
            for (int i = 0; i < tcp_data->byte_count; i++)
            {
                send_frame[7 + i] = tcp_data->value[i] & 0xff;
            }

            /* 组装CRC校验 */
            crc_code = crc16(send_frame, (7 + tcp_data->byte_count));
            send_frame[7 + tcp_data->len] = crc_code >> 8;
            send_frame[8 + tcp_data->len] = crc_code & 0xff;
            /* 要发送的帧长度 */
            send_frame_len = 9 + tcp_data->byte_count;
        }
        else if (tcp_data->func_code == 0x10)
        {
            /* 组装要写入寄存器的值 */
            for (int i = 0; i < tcp_data->len; i++)
            {
                send_frame[7 + i * 2] = tcp_data->value[i] >> 8;
                send_frame[8 + i * 2] = tcp_data->value[i] & 0xff;
            }

            /* 组装CRC校验 */
            crc_code = crc16(send_frame, (7 + tcp_data->len * 2));
            send_frame[7 + tcp_data->len * 2] = crc_code >> 8;
            send_frame[8 + tcp_data->len * 2] = crc_code & 0xff;
            /* 要发送的帧长度 */
            send_frame_len = 9 + tcp_data->len * 2;
        }
    }

    /* 通过ZigBee进行数据通信 */
    if (comm_type == COMM_XBEE)
    {
        for (int i = 0; i < get_cfg().well_max_num; i++)
        {
            if (get_cfg().well_info[i].id == tcp_data->id)
            {
                XBeeAddress64 *addr64 =
                    new XBeeAddress64(get_cfg().well_info[i].addr >> 32,
                                      get_cfg().well_info[i].addr & 0xffffffff);
                if (addr64 == nullptr)
                {
                    return -1;
                }

                debug_custom("发送的xbee写数据帧>>", send_frame, send_frame_len);

                while (err_num < 2)
                {
                    /* 给井口发送ZigBee数据 */
                    xbeeTx(*xbee_handler,
                           to_zigbee.data,
                           to_zigbee.len,
                           *addr64,
                           NO_RESPONSE_FRAME_ID);

                    debug("\n--- %s ---\n", " xbeeTx has been sent");

                    ret = xbeeRx(*xbee_handler, recv_frame, &recv_len,
                                 &recv_addr64);

                    if (ret <= 0)
                    {
                        err_num++;
                        continue;
                    }

                    debug("--- %s ---\n", " xbeeRx has received");
                    break;
                }

                delete addr64;
                addr64 = nullptr;
                break;
            }
        }

        if (err_num >= 2)
        {
            debug("%s\n", "xbee rx gt error");
            return -1;
        }
    }
    /* 通过rs485进行数据通信 */
    else if (comm_type == COMM_485)
    {
        /* 打印 */
        debug_custom("发送的rs485写数据帧>>", send_frame, send_frame_len);
        /* 发送数据 */
#ifdef ARMCQ
        if (usart_send_data(rs485_info[0].fd, send_frame, send_frame_len) < 0)
        {
            return -1;
        }
#else  // ifdef ARMCQ
        if (uart_485->Write(send_frame, send_frame_len) < 0)
        {
            return -1;
        }
#endif // ifdef ARMCQ
        /* 延迟20ms */
        usleep(20000);
        /* 打印 */
        debug_custom("接收的rs485数据帧>>", nullptr, 0);
        /* 接收数据 */
#ifdef ARMCQ
        ret = usart_rev_data(rs485_info[0].fd, recv_frame);
#else  // ifdef ARMCQ
        ret = uart_485->Read(recv_frame, 15, 1000);
        /* 打印 */
        debug_custom(nullptr, recv_frame, strlen((const char *)recv_frame));
#endif // ifdef ARMCQ
        if ((ret <= 0) ||
            ((recv_frame[0] != tcp_data->id) &&
             ((recv_frame[1] != tcp_data->func_code) ||
              (recv_frame[1] != (tcp_data->func_code + 0x80)))) ||
            (crc16_check(recv_frame, 8) < 0))
        {
            debug_custom("接收rs485数据帧出错或超时>>", nullptr, 0);
            return -1;
        }
    }
    /* 通过井场RTU本身进行数据通信 */
    else if (comm_type == COMM_RTU)
    {
        //TODO：根据商定的协议进行代码编写
    }
    /* 清空内存 */
    memset(send_frame, 0, sizeof(send_frame));

    /* 开始组装返回给服务器的modbus TCP帧 */
    send_frame[0] = tcp_data->frame_header[0];
    send_frame[1] = tcp_data->frame_header[1];
    send_frame[2] = tcp_data->frame_header[2];
    send_frame[3] = tcp_data->frame_header[3];
    if (recv_frame[1] == tcp_data->func_code)
    {
        send_frame[4] = 0;
        send_frame[5] = 6;
        for (int i = 0; i < 6; i++)
        {
            send_frame[i + 6] = recv_frame[i];
        }

        send_frame_len = 12;
    }
    else if ((recv_frame[1] == 0x85) ||
             (recv_frame[1] == 0x86) ||
             (recv_frame[1] == 0x8f) ||
             (recv_frame[1] == 0x90))
    {
        send_frame[4] = 0;
        send_frame[5] = 3;
        for (int i = 0; i < 3; i++)
        {
            send_frame[i + 6] = recv_frame[i];
        }

        send_frame_len = 9;
    }
    else
    {
        debug_custom("接收到错误功能码>>", recv_frame, 1);
        return -1;
    }

    /* 打印 */
    debug_custom("要返回给上位机的写响应帧>>", send_frame, send_frame_len);

    /* 发送给上位机 */
    if (send(ctx->s, send_frame, send_frame_len, MSG_NOSIGNAL) < 0)
    {
        debug("%s\n", " modbus TCP send error");
        return -1;
    }
    return 0;
}