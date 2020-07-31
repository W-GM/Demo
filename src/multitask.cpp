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
 * @brief 读取配置文件
 *
 * @return int 成功：0  失败：-1
 */
int MultiTask::config_manage()
{
    uint8_t ret = 0;
    int     j = 0;
    char   *str;
    string  option[] = {
        "VERSION",
        "RTU_NAME",
        "PORT",
        "IP",
        "GATEWAY",
        "MAC",  /* 当前未配置 */
        "MASK",
        "MANIFOLD_PRESSURE_1",
        "MANIFOLD_PRESSURE_2",
        "MANIFOLD_RANGE_1",
        "MANIFOLD_RANGE_2",
        "XBEE_ID",
        "XBEE_SC",
        "XBEE_AO",
        "XBEE_CE",
        "WELLHEAD_1",
        "WELLHEAD_2",
        "WELLHEAD_3",
        "WELLHEAD_4",
        "WELLHEAD_5",
        "WELLHEAD_6",
        "WELLHEAD_7",
        "WELLHEAD_8",
        "WELLHEAD_9",
        "WELLHEAD_10",
        "WELLHEAD_11",
        "WELLHEAD_12",
        "WELLHEAD_13",
        "WELLHEAD_14",
        "WELLHEAD_15",
        "WELLHEAD_16",
        "VALVE_GROUP_125",
        "VALVE_GROUP_126",
        "VALVE_GROUP_127",
        "VALVE_GROUP_128",
    };

#ifdef ARMCQ
    const char *path = "/home/config/cq_config.xml";
#else // ifdef ARMCQ
    const char *path = "./config/cq_config.xml";
#endif // ifdef ARMCQ

    /* ---------- 打开并读取配置文件 ----------- */
    if (config.LoadFile(path) != XML_SUCCESS)
    {
        /* 读取文件失败 */
        printf("load config file error !\n");
        return -1;
    }

    /* 读取标签 */
    if ((config_label = config.FirstChildElement("CONFIG")) == nullptr)
    {
        printf("load or read xml file error\n");
        return -1;
    }

    /* 读取标签中的配置项 */

    for(int i = 0; i < 15; i++)
    {
        if ((config_option =
             config_label->FirstChildElement(option[i].c_str())) == nullptr)
        {
            debug("%s\n", "load or read xml file error\n");
            return -1;
        }

        switch (i)
        {
        case 0: /* 固件版本 */
            config_info.version = config_option->GetText();
            break;
        case 1: /* rtu名称 */
            config_info.rtu_name = config_option->GetText();
            break;
        case 2: /* 端口号 */
            config_info.port = atoi(config_option->GetText());
            break;
        case 3: /* ip地址 */
            config_info.ip = config_option->GetText();
            break;
        case 4: /* 网关 */
           config_info.gateway = config_option->GetText();
            break;
        case 5: /* mac地址 */
            config_info.mac = config_option->GetText();
            break;
        case 6: /* 子网掩码 */
            config_info.mask = config_option->GetText();
            break;
        case 7: /* 汇管压力配置1 */
            config_info.manifold_1.type = strtol(config_option->GetText(), &str, 16) >> 12;
            config_info.manifold_1.add = (strtol(config_option->GetText(), &str, 16) >> 8) & 0xf;
            config_info.manifold_1.id = strtol(config_option->GetText(), &str, 16) & 0xff;
            break;
        case 8: /* 汇管压力配置2 */
            config_info.manifold_2.type = strtol(config_option->GetText(), &str, 16) >> 12;
            config_info.manifold_2.add = (strtol(config_option->GetText(), &str, 16) >> 8) & 0xf;
            config_info.manifold_2.id = strtol(config_option->GetText(), &str, 16) & 0xff;
            break;
        case 9:
            config_info.manifold_1.range = strtol(config_option->GetText(), &str, 16);
            break;
        case 10:
            config_info.manifold_2.range = strtol(config_option->GetText(), &str, 16);
        case 11: /* 16位的PAN ID */
            config_info.xbee_id = config_option->GetText();
            break;
        case 12: /* SC 7fff */
            config_info.xbee_sc = config_option->GetText();
            break;
        case 13: /* AO 0:<不接收ack>  1:<接收ack> */
            config_info.xbee_ao = atoi(config_option->GetText());
            break;
        case 14: /* CE 0:<路由器>  1:<协调器> */
            config_info.xbee_ce = atoi(config_option->GetText());
            break;
        default:
            return -1;
        }
    }

    memset(config_info.well_info, 0, sizeof(config_info.well_info));

    /* 井口与阀组的配置信息 */
    for(int i = 15; i < 35; i++)
    {
        if ((config_option =
                 config_label->FirstChildElement(option[i].c_str())) == nullptr)
        {
            debug("%s\n", "load or read xml file error\n");
            return -1;
        }

        ret = strtol(config_option->GetText(), &str, 16) & 0xff;

        if(ret > 0)
        {
            if(i < 31)
            {
                /* 油井和水井 */
                config_info.well_info[j].id = i - 14;
            }
            else if(i >= 31)
            {
                /* 阀组 */
                config_info.well_info[j].id = i - 31 + 125;
            }
            
            config_info.well_info[j].type = ret;
            config_info.well_info[j].addr = 0xffff;
            j++;
        }
    }
    /* 添加站号为128的井场主RTU */
    if(config_info.well_info[j - 1].id != 128)
    {
        config_info.well_info[j].id = 128;
        config_info.well_info[j].type = 0;
        config_info.well_max_num = j + 1;
    }
    else
    {
        /* 添加配置的井口和阀组的总个数，包含未配置阀组的128站号 */
        config_info.well_max_num = j;
    }

    return 0;
}


int MultiTask::rs485(struct state_machine_current_info *curr_info,
                     struct data_block                 *data_block)
{
    int crcCode = 0;

    int ret = 0;

    /* 保存允许出错的次数 */
    int err_num = 0;

    /* 用于组装发送给rs485的数据 */
    uint8_t to_485_data[8] = { 0 };

    /* 8 to 16 */
    uint16_t uint8To16 = 0;

    /* 用于接收rs485发送的数据 */
    uint8_t from_485_data[MODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    XBeeAddress64 addr64;

    /* 开始组装RTU数据 */
    if (curr_info->store_type == TYPE_VALVE_GROUP_DATA)
    {
        // to_485_data[0] = get_config().well_info[curr_info->id_addr].id;
        if (get_config().well_info[curr_info->id_addr].id == 128)
        {
            to_485_data[0] = 16;
        }
        else if (get_config().well_info[curr_info->id_addr].id == 127)
        {
            to_485_data[0] = 15;
        }
        else if (get_config().well_info[curr_info->id_addr].id == 126)
        {
            to_485_data[0] = 14;
        }
        else if (get_config().well_info[curr_info->id_addr].id == 125)
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
    if((curr_info->func_code == 0x03) || (curr_info->func_code == 0x04))
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

        // write(rs485_info.fd, to_485_data, 8);
    #ifdef ARMCQ
        usart_send_data(rs485_info.fd, to_485_data, 8);
    #else // ifdef ARMCQ
        uart_485->Write(to_485_data, 8);
    #endif // ifdef ARMCQ

        usleep(20000); // 40ms

        debug("%s\n", "");
        debug("%s\n", "接收的rs485数据帧>>");

        /* 等待接收数据 */

        // ret = read(rs485_info.fd, from_485_data, sizeof(from_485_data));
    #ifdef ARMCQ
        ret = usart_rev_data(rs485_info.fd, from_485_data);
    #else // ifdef ARMCQ
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
        return -1;
    }

    /* 判断接收到的rs485数据是否正确 */
    if((from_485_data[0] != to_485_data[0]) ||
       (from_485_data[1] != to_485_data[1]) || 
       (from_485_data[2] != curr_info->addr_len * 2))
    {
        debug("%s\n", "recv rs485 data error");
        return -1;
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
    return 0;
}

#if 0

/**
 * @brief 用于在与上位机通讯线程中给ZigBee发送数据过程
 *
 * @param query 指向接收到的上位机发送来的帧
 * @param rc 接收到的帧长度
 * @return int 成功：0；失败：-1
 */
int MultiTask::to_xbee(struct tcp_data *tcp_data)
{
    int ret = 0;
    uint16_t crc_code = 0;

    /* 保存要发送给上位机的总数据 */
    std::vector<uint8_t> to_tcp_data;

    short to_len = 0;
    int   count = 0;

    /* 允许出错的数量 */
    int err_num = 0;

    /* 用于组合发送给上位机的数据帧 */
    std::vector<uint8_t> to_tcp_frame;

    /* 用于接收井口 RTU 发送的数据 */
    uint8_t rtu_data[256] = { 0 };

    /* 接收到的 RTU 数据长度 */
    int xbeeRtulen = 0;

    uint64_t add64 = 0;

    time_t tim = time(nullptr);

    /* 用于判断xbee数据是否发送接收成功，可以返回给上位机 */
    bool is_ready_data = false;

    /* 用于组装0x10帧时的一个循环标志量 */
    int group_0x10_sign = 0;

    /* 设置frame ID ，0为不响应，1为响应 */
    uint8_t set_frame_id = 0;

    /* 将要请求的寄存器数据长度保存在to_len */
    if (tcp_data->func_code == 0x06)
    {
        to_len = 1;
    }
    else
    {
        to_len = tcp_data->len;
    }

    /* ############ 上锁 ############ */
    m_tx_rx.lock();

    while (to_len > 0)
    {
        memset(rtu_data, 0, sizeof(rtu_data));
        to_tcp_data.clear();

        /* 判断功能码的类型 */
        if (tcp_data->func_code == 0x06)
        {
            /* 组装下发给各井口的指令帧 */
            to_zigbee.data[0] = tcp_data->rtu_id;
            to_zigbee.data[1] = tcp_data->func_code;
            to_zigbee.data[2] = tcp_data->start_addr >> 8;
            to_zigbee.data[3] = tcp_data->start_addr & 0xff;
            to_zigbee.data[4] = tcp_data->set_val >> 8;
            to_zigbee.data[5] = tcp_data->set_val & 0xff;

            /* 填充 CRC 校验码 */
            crc_code = crc16(to_zigbee.data, 6);
            to_zigbee.data[6] = crc_code >> 8;
            to_zigbee.data[7] = crc_code & 0x00FF;
            to_zigbee.len = 8;

            set_frame_id = DEFAULT_FRAME_ID;
        }
        else if (tcp_data->func_code == 0x10)
        {
            /* 组装下发给各井口的指令帧 */
            to_zigbee.data[0] = tcp_data->rtu_id;
            to_zigbee.data[1] = tcp_data->func_code;
            to_zigbee.data[2] = tcp_data->start_addr >> 8;
            to_zigbee.data[3] = tcp_data->start_addr & 0xff;
            to_zigbee.data[4] = tcp_data->len >> 8;
            to_zigbee.data[5] = tcp_data->len & 0xff;
            to_zigbee.data[6] = tcp_data->byte_count;

            for (int i = 0; i < (tcp_data->byte_count / 2); i++)
            {
                to_zigbee.data[7 + i] = tcp_data->value[i] >> 8;
                to_zigbee.data[8 + i] = tcp_data->value[i] & 0xff;
                group_0x10_sign = i;
            }

            /* 填充 CRC 校验码 */
            crc_code = crc16(to_zigbee.data, (8 + group_0x10_sign));
            to_zigbee.data[9 + group_0x10_sign] = crc_code >> 8;
            to_zigbee.data[10 + group_0x10_sign] = crc_code & 0x00FF;
            to_zigbee.len = 11 + group_0x10_sign;

            set_frame_id = DEFAULT_FRAME_ID;
        }
        else if (tcp_data->func_code == 0x03)
        {
            /* 复制原有内容 */
            to_zigbee.data[0] = tcp_data->rtu_id;
            to_zigbee.data[1] = tcp_data->func_code;

            if (to_len > 40)
            {
                to_zigbee.data[2] = (tcp_data->start_addr + count * 40) >> 8;
                to_zigbee.data[3] = (tcp_data->start_addr + count * 40) & 0xff;
                to_zigbee.data[4] = 0;
                to_zigbee.data[5] = 40;
            }
            else
            {
                to_zigbee.data[2] = (tcp_data->start_addr + count * 40) >> 8;
                to_zigbee.data[3] = (tcp_data->start_addr + count * 40) & 0xff;
                to_zigbee.data[4] = to_len >> 8;
                to_zigbee.data[5] = to_len & 0xff;
            }

            /* 填充 CRC 校验码 */
            crc_code = crc16(to_zigbee.data, 6);
            to_zigbee.data[6] = crc_code >> 8;
            to_zigbee.data[7] = crc_code & 0x00FF;
            to_zigbee.len = 8;

            set_frame_id = NO_RESPONSE_FRAME_ID;
        }

        for (int i = 0; i < get_config().well_max_num; i++)
        {
            if (get_config().well_info[i].id == tcp_data->rtu_id)
            {
                XBeeAddress64 addr64 = XBeeAddress64(
                    get_config().well_info[i].addr >> 32,
                    get_config().well_info[i].addr &
                    0xffffffff);

                for (int i = 0; i < 8; i++)
                {
                    debug("to_zigbee>>(%x)", to_zigbee.data[i]);
                }
                debug("%s\n", "");

                while (err_num < 2)
                {
                    /* 给井口发送ZigBee数据 */
                    xbeeTx(*xbee_handler,
                           to_zigbee.data,
                           to_zigbee.len,
                           addr64,
                           set_frame_id);

                    debug("\n--- %s ---\n", " xbeeTx has been sent");

                    ret = xbeeRx(*xbee_handler, rtu_data, &xbeeRtulen,
                                 &add64);

                    // for (int i = 0; i < xbeeRtulen; i++)
                    // {
                    //     debug("rtu_data>>(%x)", rtu_data[i]);
                    // }
                    // debug("%s\n", "");
                    // printf("add64>>%x  %x\n", uint32_t(add64 >> 32),
                    //        uint32_t(add64 & 0xffffffff));
                    // debug("ret>>%d\n", ret);

                    /* 接收井口的ACK */
                    if (tcp_data->func_code == 0x03)
                    {
                        if ((ret <= 0) || \
                            ((to_zigbee.data[5] * 2) != rtu_data[2]))
                        {
                            err_num++;
                            continue;
                        }
                    }
                    else
                    {
                        if (ret <= 0)
                        {
                            err_num++;
                            continue;
                        }
                    }


                    debug("--- %s ---\n", " xbeeRx has received");
                    break;
                }
                break;
            }
        }

        if (err_num >= 2)
        {
            debug("%s\n", "xbee rx gt error");
            break;
        }

        debug("+++++time >> %ld\n", time(nullptr) - tim);

        /* 将 xbeeRx 数据存入共享数据区 */
        if ((tcp_data->func_code == 0x06) || (tcp_data->func_code == 0x10))
        {
            for (int i = 0; i < xbeeRtulen - 2; i++)
            {
                to_tcp_data.push_back(rtu_data[i]);
            }
        }
        else
        {
            for (int i = 0; i < xbeeRtulen - 5; i++)
            {
                to_tcp_data.push_back(rtu_data[i + 3]);
            }
        }

        count++;
        to_len -= 40;
        is_ready_data = true;
    }

    /* ############ 解锁 ########### */
    m_tx_rx.unlock();

    if (is_ready_data == false)
    {
        return -1;
    }

    /* 组装发送的数据 */
    to_tcp_frame.clear();

    /* 加帧头 */
    to_tcp_frame.push_back(tcp_data->frame_header[0]);
    to_tcp_frame.push_back(tcp_data->frame_header[1]);
    to_tcp_frame.push_back(tcp_data->frame_header[2]);
    to_tcp_frame.push_back(tcp_data->frame_header[3]);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) >> 8);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) & 0xff);

    /* 加站号，功能码，数据长度 */
    if ((tcp_data->func_code == 0x06) || (tcp_data->func_code == 0x10))
    {
        for (size_t i = 0; i < to_tcp_data.size(); i++)
        {
            to_tcp_frame.push_back(to_tcp_data[i]);
        }
    }
    else
    {
        to_tcp_frame.push_back(tcp_data->rtu_id);
        to_tcp_frame.push_back(tcp_data->func_code);
        to_tcp_frame.push_back((tcp_data->len * 2) & 0xff);

        /* 加数据 */
        for (int i = 0; i < tcp_data->len * 2; i++)
        {
            to_tcp_frame.push_back(to_tcp_data[i]);
        }
    }

    std::string to_data(to_tcp_frame.begin(), to_tcp_frame.end());

    if ((tcp_data->func_code == 0x06) || (tcp_data->func_code == 0x10))
    {
        debug("%s\n", "要发送给上位机命令响应帧>>");
    }
    else
    {
        debug("%s\n", "要发送给上位机的功图数据>>");
    }

    for (size_t i = 0; i < to_tcp_frame.size(); i++)
    {
        debug("[%.2x]", to_tcp_frame[i]);
    }
    debug("%s\n", "");

    /* 发送给上位机 */
    if (send(ctx->s, to_data.c_str(), to_tcp_frame.size(),
             MSG_NOSIGNAL) < 0)
    {
        debug("%s\n",
              " modbus TCP send error");
        return -1;
    }
    debug("time >> %ld\n", time(nullptr) - tim);
    debug("--- %s ---\n",
          " ##########modbus TCP  has been sent############");

    return 0;
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
    int id_site = 0;

    /* 用于判断是否是从井场40051(汇管压力)开始读取多位寄存器值 */
    int st = 0;

    /* 用于保存阀组间寄存器起始地址与100的整除值 */
    int ch = 0;

    /* 用于保存起始地址加长度与对应寄存器区间最大值的差值 */
    int dif = 0;

    /* 用于保存。。。*/
    int dif_len = 0;

    /* 用于组合发送给上位机的数据帧 */
    std::vector<uint8_t> to_tcp_frame;

    /* 判断站号是否在当前配置相里，没有则退出当前函数 */
    while (id_site < get_config().well_max_num)
    {
        if (tcp_data->rtu_id != get_config().well_info[id_site].id)
        {
            id_site++;
            continue;
        }
        break;
    }

    if (id_site >= get_config().well_max_num)
    {
        return -1;
    }

    /* 组装帧头 */
    to_tcp_frame.push_back(tcp_data->frame_header[0]);
    to_tcp_frame.push_back(tcp_data->frame_header[1]);
    to_tcp_frame.push_back(tcp_data->frame_header[2]);
    to_tcp_frame.push_back(tcp_data->frame_header[3]);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) >> 8);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) & 0xff);

    /* 加站号，功能码 */
    to_tcp_frame.push_back(tcp_data->rtu_id);
    to_tcp_frame.push_back(tcp_data->func_code);

    /* 请求读寄存器 */
    if (tcp_data->func_code == 0x03)
    {
        /* 加寄存器长度 */
        to_tcp_frame.push_back((tcp_data->len * 2) & 0xff);

        /* 油井数据类型 */
        if (get_config().well_info[id_site].type == TYPE_OIL_WELL_DATA)
        {
            if ((tcp_data->start_addr >= 0) && (tcp_data->start_addr <= 209))
            {
                dif = tcp_data->start_addr + tcp_data->len - 210;

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
                    to_tcp_frame.push_back( \
                        to_db.rd_db[id_site].oil_basic_data[i] >> 8);
                    to_tcp_frame.push_back( \
                        to_db.rd_db[id_site].oil_basic_data[i] & 0xff);
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
            else if ((tcp_data->start_addr >= 1410) &&
                     (tcp_data->start_addr <= 1419))
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

                /* 基础数据已经准备好，可以读取并发给上位机 */
                for (uint16_t i = tcp_data->start_addr - 1200;
                     i < (tcp_data->start_addr - 1200 + dif_len); i++)
                {
                    to_tcp_frame.push_back( \
                        to_db.rd_db[id_site].oil_basic_data[i] >> 8);
                    to_tcp_frame.push_back( \
                        to_db.rd_db[id_site].oil_basic_data[i] & 0xff);
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
            else if ((tcp_data->start_addr >= 210) &&
                     (tcp_data->start_addr <= 1409))
            {
                dif = tcp_data->start_addr + tcp_data->len - 1410;

                if (dif <= 0)
                {
                    dif = 0;
                }

                debug("%s\n", "*******接收并发送功图数据*******");

                // TODO：此处并未像之前一般，对请求的数据长度做判断处理
                if (to_xbee(tcp_data) < 0)
                {
                    debug("%s\n", "*******发送功图数据失败*******");
                    return -1;
                }
                return 0;
            }

            else
            {
                ret = -1;
            }
        }

        /* 水井数据类型 */
        else if (get_config().well_info[id_site].type ==
                 TYPE_WATER_WELL_DATA)
        {
            /* 判断查询的数据为哪一部分 */
            if ((tcp_data->start_addr >= 0) && \
                ((tcp_data->start_addr + tcp_data->len) <= 20))
            {
                /* ###### 上锁 ###### */
                m_rd_db.lock();

                /* 基础数据已经准备好，可以读取并发给上位机 */
                for (uint16_t i = tcp_data->start_addr;
                     i < (tcp_data->start_addr + tcp_data->len); i++)
                {
                    to_tcp_frame.push_back( \
                        to_db.rd_db[id_site].water_well_data[i] >> 8);
                    to_tcp_frame.push_back( \
                        to_db.rd_db[id_site].water_well_data[i] & 0xff);
                }

                /* ###### 解锁 ###### */
                m_rd_db.unlock();
            }
            else
            {
                ret = -1;
            }
        }

        /* 阀组间数据类型 */
        else if (get_config().well_info[id_site].type ==
                 TYPE_VALVE_GROUP_DATA)
        {
            /* ###### 上锁 ###### */
            m_rd_db.lock();

            if ((tcp_data->start_addr >= 50) && (tcp_data->len <= 55))
            {
                /* 汇管压力 */
                if (tcp_data->start_addr == 50)
                {
                    to_tcp_frame.push_back( \
                        to_db.rd_db[to_id_manifold_1].manifold_pressure[0] >> 16);
                    to_tcp_frame.push_back( \
                        to_db.rd_db[to_id_manifold_1].manifold_pressure[0] >> 8);

                    st = 1;
                }

                if ((tcp_data->start_addr == 51) || (tcp_data->len > 1))
                {
                    if (tcp_data->start_addr >= 100)
                    {
                        ch = tcp_data->start_addr / 100 - 1;
                    }

                    for (uint16_t i =                                            \
                             tcp_data->start_addr - ch * 100 + ch * 6 - 51 + st; \
                         i < (tcp_data->start_addr - ch * 100 + ch * 6 - 51 +    \
                              tcp_data->len);                                    \
                         i++)
                    {
                        to_tcp_frame.push_back( \
                            to_db.rd_db[id_site].valve_group_data_manage[i] >> 8);
                        to_tcp_frame.push_back( \
                            to_db.rd_db[id_site].valve_group_data_manage[i] &
                            0xff);
                    }
                }
            }
            else
            {
                ret = -1;
            }

            /* ###### 解锁 ###### */
            m_rd_db.unlock();
        }
    }

    /* 请求写单个寄存器 */
    else if (tcp_data->func_code == 0x06)
    {
        if (get_config().valve_group != 1)
        {
            if (to_xbee(tcp_data) < 0)
            {
                debug("%s\n", "*******发送写单个寄存器指令失败*******");
                return -1;
            }
        }
        else
        {
            // TODO：通过485的阀组连接方式，写寄存器待完善
        }

        return 0;
    }

    /* 请求写多个寄存器 */
    else if (tcp_data->func_code == 0x10)
    {
        if (get_config().valve_group != 1)
        {
            if (to_xbee(tcp_data) < 0)
            {
                debug("%s\n", "*******发送写多个寄存器指令失败*******");
                return -1;
            }
        }
        else
        {
            // TODO：通过485的阀组连接方式，写寄存器待完善
        }
        return 0;
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

#endif

/**
 * @brief 用于在与井口通讯线程中状态机的交互过程
 *
 * @param curr_info 用于指向状态机的当前状态信息
 * @param data_block 用于指向存储站号的一组数据
 * @return int 成功：0；失败：-1
 */
int MultiTask::state_machine_operation(
    struct state_machine_current_info *curr_info,
    struct data_block                 *data_block)
{
    /* 保存CRC校验结果的临时变量 */
    int crcCode = 0;

    int ret = 0;

    /* 保存允许出错的次数 */
    int err_num = 0;

    /* 保存接收到的64位从机地址 */
    uint64_t slave_addr64 = 0;

    /* 用于组装发送给ZigBee的数据 */
    uint8_t to_xbee_data[8] = { 0 };

    /* 8 to 16 */
    uint16_t uint8To16 = 0;

    /* 用于接收井口 RTU 发送的数据 */
    uint8_t xbee_rtu_data[MODBUS_RTU_MAX_ADU_LENGTH] = { 0 };

    /* 接收到的 RTU 数据长度 */
    int xbeeRtulen = 0;

    XBeeAddress64 addr64;

    /* ------ 开始组装RTU数据 ------ */

    /* 要发送的站号 */
    if(current_info.store_type == TYPE_VALVE_GROUP_DATA)
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

    addr64 = XBeeAddress64(                                    \
        get_config().well_info[curr_info->id_addr].addr >> 32, \
        get_config().well_info[curr_info->id_addr].addr & 0xffffffff);

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

        if ((ret <= 0) || \
            ((curr_info->addr_len * 2) != (uint16_t)xbee_rtu_data[2]))
        {
            err_num++;
            continue;
        }
        break;
    }

    if (err_num >= 2)
    {
        debug("站号(%d)数据接收失败-------------->>\n", curr_info->id);

        curr_info->phase = PHASE_START;
        curr_info->isGetTime = false;
        return -1;
    }

    /* 保存目标的64位地址 */
    if (get_config().well_info[curr_info->id_addr].addr == 0xffff)
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
            /* 偏移量为-1,基础数据与功图基础数据第一部分在第0~209位，功图基础数据第二部分在210~219位 */
            data_block[curr_info->id_addr].oil_basic_data.push_back(uint8To16);
        }

        else if (curr_info->store_type == TYPE_WATER_WELL_DATA)
        {
            data_block[curr_info->id_addr].water_well_data.push_back(uint8To16);
        }
        else if (curr_info->store_type == TYPE_MANIFOLD_PRESSURE_1)
        {
            wellsite_info.manifold_pressure[0] = uint8To16;
        }
        else if(curr_info->store_type == TYPE_MANIFOLD_PRESSURE_2)
        {
            wellsite_info.manifold_pressure[1] = uint8To16;
        }
        else if (curr_info->store_type == TYPE_VALVE_GROUP_DATA)
        {
            data_block[curr_info->id_addr].valve_group_data.push_back(uint8To16);
        }
    }
    return 0;
}





/**
 * @brief Construct a new Multi Task:: Multi Task object
 *
 */
MultiTask::MultiTask()
{
    /* ---------读取配置文件中的配置项--------- */
    if (config_manage() < 0)
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

    /* ------------ 初始化xbee对应的uart ------------ */
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
#else // ifdef ARMCQ

    if (xbee_ser->Open("/dev/ttyUSB0", 9600) < 0)
#endif // ifdef ARMCQ
    {
        // fprintf(stderr, "serial_open(): %s\n", xbee_ser->errmsg());
        is_error = ERROR_XBEE;
        return;
    }

    /* -------------初始化DI，DO，AI的配置--------------- */

    // TODO：当前AI仅可用作连接汇管，后期再做进一步考量
    // TODO：当前DI，DO并未做实际的配置，后期做进一步的考量
    DI_AI_DO.en_di = false;
    DI_AI_DO.en_do = false;

    /* 当汇管压力连接到井场RTU上时，使能AI(SPI) */
    if ((get_config().manifold_1.type == CON_WIRED) || 
        (get_config().manifold_2.type == CON_WIRED))
    {
        DI_AI_DO.en_ai = true;
    }
    else
    {
        DI_AI_DO.en_ai = false;
    }

    /* ----------------- i2c初始化 --------------------- */
    if (DI_AI_DO.en_di || DI_AI_DO.en_do)
    {
        i2c_D = i2c_init("/dev/i2c-2", 0x74, 0xff, 0x03);

        if (i2c_D == nullptr)
        {
            is_error = ERROR_I2C;
            return;
        }
    }

    /* ------------------ spi初始化 --------------------- */
    if (DI_AI_DO.en_ai)
    {
        spi_ai.spi = spi_init("/dev/spidev0.0");

        if (spi_ai.spi == nullptr)
        {
            is_error = ERROR_SPI;
            return;
        }

        spi_ai.val[0] = 0;
        spi_ai.val[1] = 0;
        spi_ai.buf_rx[0] = 0;
        spi_ai.buf_rx[1] = 0;

        spi_ai.buf_tx = spi_buf_tx;
    }

    /* ------------------ rs485初始化 ------------------ */
    for(int i = 0; i < get_config().well_max_num; i++)
    {
        if ((get_config().well_info[i].type & 0xf) == CON_WIRED)
        {
        #ifdef ARMCQ
            rs485_info.fd = usart_init(0, 9600, 8, 1, 1);

            if (rs485_info.fd < 0)
            {
                is_error = ERROR_485;
                return;
            }
        #else // ifdef ARMCQ
            uart_485 = new uart();

            if (uart_485 == nullptr)
            {
                // fprintf(stderr, "uart_485 new error\n");
                is_error = ERROR_485;
                return;
            }

            /* 打开串口 */
            if (uart_485->Open("/dev/ttyUSB1", 9600) < 0)
            {
                // fprintf(stderr, "serial_open(): %s\n", uart_485->errmsg());
                is_error = ERROR_485;
                return;
            }
        #endif // ifdef ARM_CQ
        
        break;
        }
    }

    /* ------------ 初始化sqlite3 ------------ */
#ifdef SQL
    sql_handler = new sqlControl();

    char path[] = "./wellsiteRTUData.db";

    /* 打开数据库，没有则创建 */
    if (sql_handler->sqlOpen(path) == -1)
    {
        delete sql_handler;
        delete xbee_ser;
        delete xbee_handler;
        sql_handler = nullptr;
        xbee_handler = nullptr;
        xbee_ser = nullptr;
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
    strcpy(sql_tab_name.tab_basic,                   "basicInfo");
    strcpy(sql_tab_name.tab_indicator_diagram_basic, "indicatorDiagramBasic");
    strcpy(sql_tab_name.tab_indicator_diagram,       "indicatorDiagram");
    strcpy(sql_tab_name.tab_water_well,              "waterWell");
    strcpy(sql_tab_name.tab_valve_group,             "valveGroup");

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
    tcsetattr(rs485_info.fd, TCSANOW, &rs485_info.oldtio);
    close(rs485_info.fd);

    // free(tcp_data.value);

    modbus_free(ctx);
    ctx = nullptr;

    if (DI_AI_DO.en_ai)
    {
        spi_close(spi_ai.spi);
        spi_free(spi_ai.spi);
    }

    if (DI_AI_DO.en_di || DI_AI_DO.en_do)
    {
        i2c_close(i2c_D->i2c);
        i2c_free(i2c_D->i2c);
        free(i2c_D);
        i2c_D = NULL;
    }

    delete data_block;
    data_block = nullptr;

    delete sql_handler;
    sql_handler = nullptr;

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

    /* -----------------初始化xbee模块------------------- */
#if 0
    /* 接收到的AT命令ACK */
    uint8_t recv_ack[9] = { 0 };

    /* 初始化的AT命令 */
    uint8_t uint_at_command[9][20];

    for (int i = 0; i < 9; i++)
    {
        memset(uint_at_command[i], 0, 20);
    }
    string atsc = "ATSC" + get_config().xbee_sc + "\r";
    string atce = "ATCE" + to_string(get_config().xbee_ce) + "\r";
    string atao = "ATAO" + to_string(get_config().xbee_ao) + "\r";
    string atid = "ATID" + get_config().xbee_id + "\r";
    strcpy((char *)uint_at_command[0], "+++");
    strcpy((char *)uint_at_command[1], "ATRE\r");
    strcpy((char *)uint_at_command[2], atsc.c_str());
    strcpy((char *)uint_at_command[3], "ATAP1\r");
    strcpy((char *)uint_at_command[4], atce.c_str());
    strcpy((char *)uint_at_command[5], atao.c_str());
    strcpy((char *)uint_at_command[6], atid.c_str());
    strcpy((char *)uint_at_command[7], "ATWR\r");
    strcpy((char *)uint_at_command[8], "ATCN\r");

    /* 如果发送失败,重发 */
    for (int i = 0; i < 3; i++)
    {
        memset(recv_ack, 0, 9);

        /* 进入AT命令模式 */
        xbee_ser->Write(uint_at_command[0], 3);

        if (xbee_ser->Read(recv_ack, 10, 1000) <= 0)
        {
            debug("%s\n", "AT command read error or timeout");
            continue;
        }

        /* 判断是否成功进入AT命令模式 */
        if (strncmp((char *)recv_ack, "OK", 2) == 0)
        {
            /* 循环发送AT命令 */
            for (int j = 1; j < 9; j++)
            {
                xbee_ser->Write(uint_at_command[j],
                                strlen((char *)uint_at_command[j]));

                if (xbee_ser->Read(recv_ack, 10, 1000) <= 0)
                {
                    debug("%s\n", "uart read error or timeout");
                    break;
                }

                if (strncmp((char *)recv_ack, "OK", 2) != 0)
                {
                    debug("%s\n", recv_ack);
                    break;
                }
            }
            break;
        }
        else
        {
            debug("%s\n", "AT command response is not OK ");
        }
    }
#endif
    /* ------------初始化modbus TCP服务器---------------- */
    /* IP地址 */
    memset(server_info.ip_addr, 0, 16);
    strcpy(server_info.ip_addr, get_config().ip.c_str());

    /* 端口号 */
    server_info.port = get_config().port;

#ifdef ARMCQ

    /* 初始化开发板上的1网口 */
    /* 设置Mac地址 */
    std::string mac;
    mac += "ifconfig eth1 hw ether 00.c9.";
    mac += get_config().ip;
    replace(mac.begin(),mac.end(),'.',':');
    system(mac.c_str());

    /* 启动网卡1 */
    system("ifconfig eth1 up");

    /* 设置IP和子网掩码 */
    std::string ip;
    ip += "ifconfig eth1 ";
    ip += get_config().ip;
    ip += " netmask ";
    ip += get_config().mask;
    system(ip.c_str());

    /* 设置网关 */
    std::string gw;
    gw += "route add default gw ";
    gw += get_config().gateway;
    gw += " dev eth1";
    system(gw.c_str());
#endif // ifdef ARMCQ

    /* --------------初始化上位机发送来的数据------------ */
    tcp_data.rtu_id = 0;
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
    data_block = new struct data_block[get_config().well_max_num];
    data_block_2 = new struct data_block[get_config().well_max_num];

    if ((data_block == nullptr) || (data_block_2 == nullptr))
    {
        debug("%s\n", "new data_block error");
        return;
    }

    //TODO：数据块部分的初始化后续根据实际情况再做修改

    /* 初始化 */
    for (int i = 0; i < get_config().well_max_num; i++)
    {

        data_block[i].rtu_id = get_config().well_info[i].id;
        data_block_2[i].rtu_id = get_config().well_info[i].id;

        data_block[i].injection_well_num = 0;
        data_block_2[i].injection_well_num = 0;

        switch (get_config().well_info[i].type)
        {
        case 0x01: /* 油井 */
            data_block[i].cur_time_diagram = 0;
            data_block[i].oil_basic_data.resize(200);
            data_block[i].ind_diagram.resize(800);

            data_block_2[i].cur_time_diagram = 0;
            data_block_2[i].oil_basic_data.resize(200);
            data_block_2[i].ind_diagram.resize(800);
            break;
        case 0x02: /* 水井 */
            data_block[i].water_well_data.resize(100);
            data_block_2[i].water_well_data.resize(100);
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
    if(get_config().well_info[get_config().well_max_num - 1].type == 0)
    {
        data_block[get_config().well_max_num - 1].wellsite_rtu.resize(100);
        data_block_2[get_config().well_max_num - 1].wellsite_rtu.resize(100);
    }

    /* 初始化写指针指向data_block,读指针指向data_block_2 */
    to_db.wr_db = data_block;
    to_db.rd_db = data_block_2;
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

    /* 保存每次读取油井基础数据的地址长度 */
    uint16_t oil_basic_addr_len[] =
    { 0x00, 0x1e, 0x1f, 0x20, 0x21, 0x20, 0x2a, 0x0a };


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

            /* 用于切换1到最后一口配置井口从机地址 */
            if (current_info.id_addr < get_config().well_max_num)
            {
                current_info.id =
                    get_config().well_info[current_info.id_addr].id;
                current_info.is_add_id = true;
            }
            else
            {
                usleep(100000);
                current_info.id_addr = 0;
                current_info.id =
                    get_config().well_info[current_info.id_addr].id;

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

                /* 汇管有线连接 */
                if((get_config().manifold_1.type == CON_WIRED) || 
                   (get_config().manifold_2.type == CON_WIRED))
                {
                    current_info.phase = PHASE_MANIFOLD_PRESSURE;
                }
            }

            /* 判断当前的站号的类型，并执行对应的程序 */
            if(get_config().well_info[current_info.id_addr].type == 
               TYPE_OIL_WELL_DATA)  
            {
                current_info.phase = PHASE_OIL_WELL_BASIC;
            }
            else if (get_config().well_info[current_info.id_addr].type == 
                     TYPE_WATER_WELL_DATA)
            {
                current_info.phase = PHASE_WATER_WELL;
            }
            else if((get_config().well_info[current_info.id_addr].type >> 4) == 
                    TYPE_VALVE_GROUP_DATA)
            {
                current_info.phase = PHASE_VALVE_GROUP;
            }
            else if((get_config().well_info[current_info.id_addr].id == 128) &&
                    (get_config().well_info[current_info.id_addr].type == 0))
            {
                current_info.phase = PHASE_WELLSITE_RTU;
            }
            else
            {
                current_info.phase = PHASE_START;
            }

            break;
        }

        case PHASE_OIL_WELL_BASIC:
        {
            to_db.wr_db[current_info.id_addr].oil_basic_data.clear();

            /* 设置当前阶段的状态信息 */
            current_info.func_code = 0x03;
            current_info.start_addr = 0;
            current_info.store_type = TYPE_OIL_WELL_DATA;

            /* 功图基础数据第一部分存储在油井基础数据的第200位(包含)开始，共10位 */
            /* 功图基础数据第二部分存储在油井基础数据的第210位(包含)开始，共10位 */
            for (int i = 0; i < 8; i++)
            {
                if (i == 7)
                {
                    current_info.start_addr = 0x0582;
                    current_info.addr_len = oil_basic_addr_len[i];
                }
                else
                {
                    current_info.start_addr += oil_basic_addr_len[i];
                    current_info.addr_len = oil_basic_addr_len[i + 1];
                }

                ret = state_machine_operation(&current_info, to_db.wr_db);

                if (ret < 0)
                {
                    break;
                }
            }

            if (ret < 0)
            {
                current_info.phase = PHASE_START;
                break;
            }

            /* 判断当前站号是否有故障,并记录故障信息到井场RTU上 */
            if(to_db.wr_db[current_info.id_addr].oil_basic_data[52] == 1)
            {
                /* 故障设备代码 */
                wellsite_info.fault_info[0] = 100 + current_info.id;

                /* 故障部位代码 */
                wellsite_info.fault_info[1] = 
                    to_db.wr_db[current_info.id_addr].oil_basic_data[53];

                /* 故障时间 */
                wellsite_info.fault_info[2] = 
                    ((get_current_time().year - 2000) << 8) | 
                    (get_current_time().month & 0xff);
                wellsite_info.fault_info[3] = 
                    (get_current_time().day << 8) | 
                    (get_current_time().hour & 0xff);
                wellsite_info.fault_info[4] = 
                    (get_current_time().minute << 8) | 
                    (get_current_time().second & 0xff);
            }

            /* 判断当前站号时候配置了汇管 */
            if(((get_config().manifold_1.type == CON_WIRELESS) && 
                (get_config().manifold_1.id == current_info.id)) || 
               ((get_config().manifold_2.type == CON_WIRELESS) && 
                (get_config().manifold_2.id == current_info.id)))
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

            /* 判断当前站号时候配置了汇管 */
            if(((get_config().manifold_1.type == CON_WIRELESS) && 
                (get_config().manifold_1.id == current_info.id)) || 
               ((get_config().manifold_2.type == CON_WIRELESS) && 
                (get_config().manifold_2.id == current_info.id)))
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

            if ((get_config().well_info[current_info.id_addr].type & 0xf) == 
                 CON_WIRED)
            {
                ret = rs485(&current_info, to_db.wr_db);
            }
            else if ((get_config().well_info[current_info.id_addr].type & 0xf) == 
                      CON_WIRELESS)
            {
                ret = state_machine_operation(&current_info, to_db.wr_db);
            }

            if (ret < 0)
            {
                current_info.phase = PHASE_START;
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

                if ((get_config().well_info[current_info.id_addr].type & 0xf) == 
                     CON_WIRED)
                {
                    ret = rs485(&current_info, to_db.wr_db);
                }
                else if ((get_config().well_info[current_info.id_addr].type & 0xf) == 
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
                current_info.phase = PHASE_START;
                break;
            }

            current_info.phase = PHASE_WELLSITE_RTU;
            break;
        }

        case PHASE_MANIFOLD_PRESSURE:
        {
            /* 汇管1配置 */
            if((get_config().manifold_1.type == CON_WIRELESS) && 
               (get_config().manifold_1.id == current_info.id))
            {
                if(to_db.wr_db[current_info.id_addr].oil_basic_data[15] == ANKONG)
                {
                    //TODO:当前配置在安特井口的汇管,默认直接去读取井口配置的AI
                    current_info.store_type = TYPE_MANIFOLD_PRESSURE_1;
                    current_info.start_addr = get_config().manifold_1.add -1;
                    current_info.addr_len = 0x01;
                    current_info.func_code = 0x04;

                    ret = state_machine_operation(&current_info, to_db.wr_db);

                    if (ret < 0)
                    {
                        break;
                    }
                }
                else if(to_db.wr_db[current_info.id_addr].oil_basic_data[15] == ANTE)
                {
                    //TODO:当前配置在安特井口的汇管,默认从40051寄存器中读取,默认为处理过的数据,后根据现场实际情况进行修改
                    wellsite_info.manifold_pressure[0] = 
                        to_db.wr_db[current_info.id_addr].oil_basic_data[50];
                } 
            }
            else if (get_config().manifold_1.type == CON_WIRED)
            {
                for (int i = 0; i < 2; i++)
                {
                    if (spi_transfer(spi_ai.spi, 
                            spi_ai.buf_tx[get_config().manifold_1.add -1], 
                            spi_ai.buf_rx,
                            sizeof(spi_ai.buf_rx)) < 0)
                    {
                        fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi_ai.spi));
                        exit(1);
                    }
                }
                //TODO:当前连接到井场AI的汇管,不清楚采集的数据单位是MPa还是Pa,默认为MPa,后期根据现场实际情况修改
                wellsite_info.manifold_pressure[0] = 
                    (((spi_ai.buf_rx[0] << 8) | (spi_ai.buf_rx[1] & 0xff)) & 0xfff) * 
                        get_config().manifold_1.range;
            }

            /* 汇管2配置 */
            if((get_config().manifold_2.type == CON_WIRELESS) && 
               (get_config().manifold_2.id == current_info.id))
            {
                if(to_db.wr_db[current_info.id_addr].oil_basic_data[15] == ANKONG)
                {
                    //TODO:当前配置在安特井口的汇管,默认直接去读取井口配置的AI
                    current_info.store_type = TYPE_MANIFOLD_PRESSURE_2;
                    current_info.start_addr = get_config().manifold_2.add -1;
                    current_info.addr_len = 0x01;
                    current_info.func_code = 0x04;

                    ret = state_machine_operation(&current_info, to_db.wr_db);

                    if (ret < 0)
                    {
                        break;
                    }
                }
                else if(to_db.wr_db[current_info.id_addr].oil_basic_data[15] == ANTE)
                {
                    //TODO:当前配置在安特井口的汇管2,默认从40060寄存器中读取,默认为处理过的数据,后根据现场实际情况进行修改
                    wellsite_info.manifold_pressure[1] = 
                        to_db.wr_db[current_info.id_addr].oil_basic_data[59];
                }                 
            }
            else if (get_config().manifold_1.type == CON_WIRED)
            {
                for (int i = 0; i < 2; i++)
                {
                    if (spi_transfer(spi_ai.spi, 
                            spi_ai.buf_tx[get_config().manifold_2.add -1], 
                            spi_ai.buf_rx,
                            sizeof(spi_ai.buf_rx)) < 0)
                    {
                        fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi_ai.spi));
                        exit(1);
                    }
                }
                wellsite_info.manifold_pressure[1] = 
                    (((spi_ai.buf_rx[0] << 8) | (spi_ai.buf_rx[1] & 0xff)) & 0xfff) * 
                        get_config().manifold_2.range;
            }
            
            current_info.phase = PHASE_START;
            break;
        }

        case PHASE_WELLSITE_RTU:
        {
            if(current_info.id == 128)
            {
               //TODO:井场及厂家信息,汇管,故障
               for(int i = 0; i < 50; i++)
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

               /* 注水井总的汇管压力 */
               to_db.wr_db[current_info.id_addr].wellsite_rtu[51] = 0;

               /* 井场故障信息 */
               for(int i = 0; i < 5; i++)
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
                for(int i = 0; i < 60; i++)
                {
                    to_db.wr_db[current_info.id_addr].wellsite_rtu[i];
                }
            }
            
            if((get_config().well_info[current_info.id_addr].type >> 4) == 
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
            current_info.phase = PHASE_START;
            break;
        }
        default:
        {
            current_info.phase = PHASE_START;
            break;
        }
        }
    }
    delete[] data_block;
    data_block = nullptr;
}
#if 0
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
                    tcp_data.rtu_id = query[6];
                    tcp_data.func_code = query[7];
                    tcp_data.start_addr = (query[8] << 8) |
                                          (query[9] & 0xff);
                    tcp_data.len = (query[10] << 8) | (query[11] & 0xff);

                    /* 读寄存器 */
                    if (tcp_data.func_code == 0x03)
                    {
                        tcp_data.len = \
                            (query[10] << 8) | (query[11] & 0xff);
                    }

                    /* 写单个寄存器 */
                    else if (tcp_data.func_code == 0x06)
                    {
                        tcp_data.set_val = \
                            (query[10] << 8) | (query[11] & 0xff);
                    }

                    /* 写多个寄存器 */
                    else if (tcp_data.func_code == 0x10)
                    {
                        tcp_data.len = \
                            (query[10] << 8) | (query[11] & 0xff);
                        tcp_data.byte_count = query[12];

                        memset(tcp_data.value, 0, sizeof(tcp_data.value));

                        for (int i = 0; i < (tcp_data.len / 2); i++)
                        {
                            tcp_data.value[i] = \
                                (query[13 + i] << 8) | (query[14 + i] & 0xff);
                        }
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

#endif

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
    int cycles[3] = { 0 };

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
    if(fd == -1)
    {
        perror("open watchdogctl error");
        close(fd);
        return;
    }
    while (1)
    {
        if(write(fd, buf, sizeof(buf)) < 0)
        {
            break;
        }
        sleep(10);
        buf[0] = buf[0]?0:1;
    }

    close(fd);
}
