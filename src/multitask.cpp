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

#include "modbus-private.h"

int static set_port(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;

    memset(&oldtio, 0, sizeof(oldtio));

    /* save the old serial port configuration */
    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("set_port/tcgetattr");
        return -1;
    }

    memset(&newtio, 0, sizeof(newtio));

    /* ignore modem control lines and enable receiver */
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    /* set character size */
    switch (nBits)
    {
    case 8:
        newtio.c_cflag |= CS8;
        break;

    case 7:
        newtio.c_cflag |= CS7;
        break;

    case 6:
        newtio.c_cflag |= CS6;
        break;

    case 5:
        newtio.c_cflag |= CS5;
        break;

    default:
        newtio.c_cflag |= CS8;
        break;
    }

    /* set the parity */
    switch (nEvent)
    {
    case 'o':
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;

    case 'e':
    case 'E':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;

    case 'n':
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;

    default:
        newtio.c_cflag &= ~PARENB;
        break;
    }

    /* set the stop bits */
    switch (nStop)
    {
    case 1:
        newtio.c_cflag &= ~CSTOPB;
        break;

    case 2:
        newtio.c_cflag |= CSTOPB;
        break;

    default:
        newtio.c_cflag &= ~CSTOPB;
        break;
    }

    /* set output and input baud rate */
    switch (nSpeed)
    {
    case 0:
        cfsetospeed(&newtio, B0);
        cfsetispeed(&newtio, B0);
        break;

    case 50:
        cfsetospeed(&newtio, B50);
        cfsetispeed(&newtio, B50);
        break;

    case 75:
        cfsetospeed(&newtio, B75);
        cfsetispeed(&newtio, B75);
        break;

    case 110:
        cfsetospeed(&newtio, B110);
        cfsetispeed(&newtio, B110);
        break;

    case 134:
        cfsetospeed(&newtio, B134);
        cfsetispeed(&newtio, B134);
        break;

    case 150:
        cfsetospeed(&newtio, B150);
        cfsetispeed(&newtio, B150);
        break;

    case 200:
        cfsetospeed(&newtio, B200);
        cfsetispeed(&newtio, B200);
        break;

    case 300:
        cfsetospeed(&newtio, B300);
        cfsetispeed(&newtio, B300);
        break;

    case 600:
        cfsetospeed(&newtio, B600);
        cfsetispeed(&newtio, B600);
        break;

    case 1200:
        cfsetospeed(&newtio, B1200);
        cfsetispeed(&newtio, B1200);
        break;

    case 1800:
        cfsetospeed(&newtio, B1800);
        cfsetispeed(&newtio, B1800);
        break;

    case 2400:
        cfsetospeed(&newtio, B2400);
        cfsetispeed(&newtio, B2400);
        break;

    case 4800:
        cfsetospeed(&newtio, B4800);
        cfsetispeed(&newtio, B4800);
        break;

    case 9600:
        cfsetospeed(&newtio, B9600);
        cfsetispeed(&newtio, B9600);
        break;

    case 19200:
        cfsetospeed(&newtio, B19200);
        cfsetispeed(&newtio, B19200);
        break;

    case 38400:
        cfsetospeed(&newtio, B38400);
        cfsetispeed(&newtio, B38400);
        break;

    case 57600:
        cfsetospeed(&newtio, B57600);
        cfsetispeed(&newtio, B57600);
        break;

    case 115200:
        cfsetospeed(&newtio, B115200);
        cfsetispeed(&newtio, B115200);
        break;

    case 230400:
        cfsetospeed(&newtio, B230400);
        cfsetispeed(&newtio, B230400);
        break;

    default:
        cfsetospeed(&newtio, B115200);
        cfsetispeed(&newtio, B115200);
        break;
    }

    /* set timeout in deciseconds for non-canonical read */
    newtio.c_cc[VTIME] = 0;

    /* set minimum number of characters for non-canonical read */
    newtio.c_cc[VMIN] = 0;

    /* flushes data received but not read */
    tcflush(fd, TCIFLUSH);

    /* set the parameters associated with the terminal from
            the termios structure and the change occurs immediately */
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("set_port/tcsetattr");
        return -1;
    }

    return 0;
}

/**
 * @brief: open serial port
 * @Param: dir: serial device path
 */
int static open_port(char *dir)
{
    int fd;

    fd = open(dir, O_RDWR);

    if (fd < 0)
    {
        perror("open_port");
    }
    return fd;
}

/**
 * @brief: main function
 * @Param: argc: number of parameters
 * @Param: argv: parameters list
 */
int static rs485_enable(const int fd, const RS485_ENABLE_t enable)
{
    struct serial_rs485 rs485conf;
    int res;

    /* Get configure from device */
    res = ioctl(fd, TIOCGRS485, &rs485conf);

    if (res < 0)
    {
        perror("Ioctl error on getting 485 configure:");
        close(fd);
        return res;
    }

    /* Set enable/disable to configure */
    if (enable)
    { // Enable rs485 mode
        rs485conf.flags |= SER_RS485_ENABLED;
    }
    else
    { // Disable rs485 mode
        rs485conf.flags &= ~(SER_RS485_ENABLED);
    }

    rs485conf.delay_rts_before_send = 0x00000004;

    /* Set configure to device */
    res = ioctl(fd, TIOCSRS485, &rs485conf);

    if (res < 0)
    {
        perror("Ioctl error on setting 485 configure:");
        close(fd);
    }

    return res;
}

int static change_bit_rs485(int fd, int value)
{
    struct serial_rs485 rs485conf;
    int res;

    /* Get configure from device */
    res = ioctl(fd, TIOCGRS485, &rs485conf);

    if (res < 0)
    {
        perror("Ioctl error on getting 485 configure:");
        close(fd);
        return res;
    }

    /*rs485 send - recv*/
    if (value == 1) rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
    else if (value == 0) rs485conf.flags &= ~SER_RS485_RTS_AFTER_SEND;
    else;

    /* Set configure to device */
    res = ioctl(fd, TIOCSRS485, &rs485conf);

    if (res < 0)
    {
        perror("Ioctl error on setting 485 configure:");
        close(fd);
    }
    return res;
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
        // to_485_data[0] = class_id.valve_group_id[curr_info->id_oil_well - 1];
        to_485_data[0] = c_info.w_info[curr_info->id].id;
    }
    else
    {
        to_485_data[0] = 0x80;
    }

    to_485_data[1] = curr_info->func_code;
    to_485_data[2] = curr_info->start_addr >> 8;
    to_485_data[3] = curr_info->start_addr & 0xff;
    to_485_data[4] = curr_info->addr_len >> 8;
    to_485_data[5] = curr_info->addr_len & 0xff;
    crcCode = crc16(to_485_data, 6);
    to_485_data[6] = crcCode >> 8;
    to_485_data[7] = crcCode & 0xFF;

    debug("%s\n", "发送的rs485数据帧>>");

    for (int i = 0; i < 8; i++)
    {
        debug("[%x]", to_485_data[i]);
    }
    debug("%s\n", "");

    /* 出现错误时，重新发送并接收一遍数据，当错误超过两次时，便放弃当前数据，采下一组数据 */
    while (err_num < 2)
    {
        /* 发送数据 */
        write(rs485_info.fd, to_485_data, 8);

        usleep(20000); // 40ms

        debug("%s\n",              "");
        debug("%s\n", "接收的rs485数据帧>>");

        /* 等待接收数据 */
        change_bit_rs485(rs485_info.fd, 0); // recv mode
        ret = read(rs485_info.fd, from_485_data, sizeof(from_485_data));

        if (ret <= 0)
        {
            err_num++;
            continue;
        }

        for (int i = 0; i < ret; i++)
        {
            debug("[%x]", from_485_data[i]);
        }
        debug("%s\n", "");
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
    if (from_485_data[0] != to_485_data[0])
    {
        debug("%s\n", "recv rs485 data error");
        return -1;
    }
    else
    {
        if (from_485_data[1] != to_485_data[1])
        {
            debug("%s\n", "recv rs485 data error");
            return -1;
        }
        else
        {
            if (from_485_data[2] != curr_info->addr_len * 2)
            {
                debug("%s\n", "recv rs485 data error");
                return -1;
            }
        }
    }

    // 将数据存入临时缓存
    for (int i = 0; i < ret - 5; i += 2)
    {
        uint8To16 = (from_485_data[i + 3] << 8) |
                    (from_485_data[i + 4] & 0xff);

        if (curr_info->store_type == TYPE_VALVE_GROUP_DATA)
        {
            data_block[curr_info->id].valve_group_data.push_back(
                uint8To16);
        }
    }
    return 0;
}

int MultiTask::config_manage()
{
    uint8_t ret = 0;
    int     j = 0;
    char   *str;
    string  option[30] = {
        "PORT",
        "IP",
        "GATEWAY",
        "MAC",
        "MASK",
        "VALVE_GROUP",
        "VALVE_SEL",
        "MANIFOLD_PRESSURE",
        "MANIFOLD_PRESSURE_ID",
        "XBEE_ID",
        "XBEE_SC",
        "XBEE_AO",
        "XBEE_CE",
        "WELLHEAD_MAX_NUM",
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
        "WELLHEAD_16"
    };

    // const char *path = "/home/wgm/wgm/PROJECT/CQ_PROJECT/build/myconfig.xml";
    const char *path = "./config/myconfig.xml";

    /* ---------- 打开并读取配置文件 ----------- */
    if (config.LoadFile(path) !=
        XML_SUCCESS)
    {
        /* 读取文件失败 */
        printf("load file error !\n");
        is_error = true;
        return -1;
    }

    /* 读取标签 */
    if ((config_label = config.FirstChildElement("CONFIG")) == nullptr)
    {
        printf("load or read xml file error\n");
        is_error = true;
        return -1;
    }

    /* 读取标签中的配置项 */
    /* 端口号 */
    if ((config_option =
             config_label->FirstChildElement(option[0].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.port = atoi(config_option->GetText());

    /* ip地址 */
    if ((config_option =
             config_label->FirstChildElement(option[1].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.ip = config_option->GetText();

    /* 网关 */
    if ((config_option =
             config_label->FirstChildElement(option[2].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.gateway = config_option->GetText();

    /* mac地址 */
    if ((config_option =
             config_label->FirstChildElement(option[3].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.mac = config_option->GetText();

    /* 子网掩码 */
    if ((config_option =
             config_label->FirstChildElement(option[4].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.mask = config_option->GetText();

    /* 阀组连接方式 */
    if ((config_option =
             config_label->FirstChildElement(option[5].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.valve_group = atoi(config_option->GetText());

    if ((config_option =
             config_label->FirstChildElement(option[6].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.valve_SEL = atoi(config_option->GetText());

    /* 汇管压力连接方式 */
    if ((config_option =
             config_label->FirstChildElement(option[7].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.manifold_pressure = atoi(config_option->GetText());

    if ((config_option =
             config_label->FirstChildElement(option[8].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.manifold_pressure_id = atoi(config_option->GetText());

    /* ID 16位的PAN ID */
    if ((config_option =
             config_label->FirstChildElement(option[9].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.xbee_id = config_option->GetText();

    /* SC 7fff */
    if ((config_option =
             config_label->FirstChildElement(option[10].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.xbee_sc = config_option->GetText();

    /* AO 0:<不接收ack>  1:<接收ack> */
    if ((config_option =
             config_label->FirstChildElement(option[11].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.xbee_ao = atoi(config_option->GetText());

    /* CE 0:<路由器>  1:<协调器> */
    if ((config_option =
             config_label->FirstChildElement(option[12].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }
    c_info.xbee_ce = atoi(config_option->GetText());

    /* 配置的井口最大个数 */
    if ((config_option =
             config_label->FirstChildElement(option[13].c_str())) == nullptr)
    {
        debug("%s\n", "load or read xml file error\n");
        return -1;
    }

    c_info.well_max_num = atoi(config_option->GetText());

    // if (c_info.valve_group == 1)
    // {

    /* 添加阀组间设备 */
    c_info.well_max_num++;

    // }

    c_info.w_info = nullptr;
    c_info.w_info = new struct well_info[c_info.well_max_num];

    if (c_info.w_info == nullptr)
    {
        debug("%s\n", "new struct well_info error");
        return -1;
    }

    /* 井口的配置信息 */
    for (int i = 14; i < 30; i++)
    {
        if ((config_option =
                 config_label->FirstChildElement(option[i].c_str())) == nullptr)
        {
            debug("%s\n", "load or read xml file error\n");
            return -1;
        }

        ret = strtol(config_option->GetText(), &str, 16) & 0xff;

        if ((ret & 0x80) != 0)
        {
            c_info.w_info[j].type = (ret & 0x70) >> 4;
            c_info.w_info[j].id = (ret & 0xf) + 1;
            c_info.w_info[j].addr = 0xffff;
            j++;
        }
    }

    // if (c_info.valve_group == 1)
    // {

    /* 添加阀组间设备 */
    c_info.w_info[j].type = 2;

    switch (c_info.valve_SEL)
    {
    case 0:
        c_info.w_info[j].id = 0x7d;
        break;

    case 1:
        c_info.w_info[j].id = 0x7e;
        break;

    case 2:
        c_info.w_info[j].id = 0x7f;
        break;

    case 3:
        c_info.w_info[j].id = 0x80;
        break;

    default:
        c_info.w_info[j].id = 0x80;
        break;
    }
    c_info.w_info[j].addr = 0xffff;

    // }

    return 0;
}

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
    char    *rrr;

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

    /* 将要请求的寄存器数据长度保存在to_len */
    to_len = tcp_data->len;

    while (to_len > 0)
    {
        memset(rtu_data, 0, sizeof(rtu_data));

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

        uint16_t addr16 = strtol(c_info.xbee_id.c_str(), &rrr, 16) & 0xffff;

        for (int i = 0; i < c_info.well_max_num; i++)
        {
            if (c_info.w_info[i].id == tcp_data->rtu_id)
            {
                XBeeAddress64 addr64 = XBeeAddress64(
                    c_info.w_info[i].addr >> 32,
                    c_info.w_info[i].addr &
                    0xffffffff);

                for (int i = 0; i < 8; i++)
                {
                    debug("to_zigbee>>(%x)", to_zigbee.data[i]);
                }
                debug("%s\n", "");

                while (err_num < 2)
                {
                    /* ############ 上锁 ############ */
                    m_tx_rx.lock();

                    /* 给井口发送ZigBee数据 */
                    xbeeTx(*xbee_handler,
                           to_zigbee.data,
                           to_zigbee.len,
                           addr64,
                           addr16);

                    debug("\n--- %s ---\n", " xbeeTx has been sent");

                    ret = xbeeRx(*xbee_handler, rtu_data, &xbeeRtulen,
                                 &add64);

                    /* ############ 解锁 ########### */
                    m_tx_rx.unlock();

                    // for (int i = 0; i < xbeeRtulen; i++)
                    // {
                    //     debug("rtu_data>>(%x)", rtu_data[i]);
                    // }
                    // debug("%s\n", "");
                    // printf("add64>>%x  %x\n", uint32_t(add64 >> 32),
                    //        uint32_t(add64 & 0xffffffff));
                    // debug("ret>>%d\n", ret);

                    /* 接收井口的ACK */
                    if (ret <= 0)
                    {
                        err_num++;
                        continue;
                    }

                    debug("--- %s ---\n", " xbeeRx has received");
                    break;
                }
                break;
            }
        }

        if (err_num >= 2)
        {
            debug("%s\n", "xbee rx error");
            break;
        }

        /* 将 xbeeRx 数据存入共享数据区 */
        for (int i = 0; i < xbeeRtulen - 5; i++)
        {
            to_tcp_data.push_back(rtu_data[i + 3]);
        }

        count++;
        to_len -= 40;
    }
#if 0

    /* 复制原有内容 */

    to_zigbee.data[0] = tcp_data->rtu_id;
    to_zigbee.data[1] = tcp_data->func_code;
    to_zigbee.data[2] = tcp_data->start_addr >> 8;
    to_zigbee.data[3] = tcp_data->start_addr & 0xff;
    to_zigbee.data[4] = tcp_data->len >> 8;
    to_zigbee.data[5] = tcp_data->len & 0xff;

    /* 填充 CRC 校验码 */
    crc_code = crc16(to_zigbee.data, 6);
    to_zigbee.data[6] = crc_code >> 8;
    to_zigbee.data[7] = crc_code & 0x00FF;
    to_zigbee.len = 8;

    uint16_t addr16 = strtol(c_info.xbee_id.c_str(), &rrr, 16) & 0xffff;

    for (int i = 0; i < c_info.well_max_num; i++)
    {
        if (c_info.w_info[i].id == tcp_data->rtu_id)
        {
            XBeeAddress64 addr64 = XBeeAddress64(
                c_info.w_info[i].addr >> 32,
                c_info.w_info[i].addr &
                0xffffffff);

            while (err_num < 2)
            {
                /* ############ 上锁 ############ */
                m_tx_rx.lock();

                /* 给井口发送ZigBee数据 */
                xbeeTx(*xbee_handler,
                       to_zigbee.data,
                       to_zigbee.len,
                       addr64,
                       addr16);

                debug("\n--- %s ---\n", " xbeeTx has been sent");

                ret = xbeeRx(*xbee_handler, rtu_data, &xbeeRtulen,
                             &add64);

                /* ############ 解锁 ########### */
                m_tx_rx.unlock();

                for (int i = 0; i < xbeeRtulen; i++)
                {
                    debug("rtu_data>>(%x)", rtu_data[i]);
                }
                debug("%s\n", "");
                printf("add64>>%x  %x\n", uint32_t(add64 >> 32),
                       uint32_t(add64 & 0xffffffff));
                debug("ret>>%d\n", ret);

                /* 接收井口的ACK */
                if (ret <= 0)
                {
                    err_num++;
                    continue;
                }

                debug("--- %s ---\n", " xbeeRx has received");
            }
            break;
        }
    }

    if (err_num >= 2)
    {
        debug("%s\n", "xbee rx error");
        return -1;
    }

    /* 将 xbeeRx 数据存入共享数据区 */
    for (int i = 0; i < xbeeRtulen - 5; i++)
    {
        tcp_data->value[i] = rtu_data[i + 3];
    }
#endif // if 0

    /* 组装发送的数据 */
    to_tcp_frame.clear();

    /* 加帧头 */
    to_tcp_frame.push_back(tcp_data->frame_header[0]);
    to_tcp_frame.push_back(tcp_data->frame_header[1]);
    to_tcp_frame.push_back(tcp_data->frame_header[2]);
    to_tcp_frame.push_back(tcp_data->frame_header[3]);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) >> 8);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) & 0xff);

    // to_tcp_frame += tcp_data->frame_header[0];
    // to_tcp_frame += tcp_data->frame_header[1];
    // to_tcp_frame += tcp_data->frame_header[2];
    // to_tcp_frame += tcp_data->frame_header[3];
    // to_tcp_frame += (tcp_data->len * 2 + 3) >> 8;
    // to_tcp_frame += (tcp_data->len * 2 + 3) & 0xff;

    /* 加站号，功能码，数据长度 */
    to_tcp_frame.push_back(tcp_data->rtu_id);
    to_tcp_frame.push_back(tcp_data->func_code);
    to_tcp_frame.push_back((tcp_data->len * 2) & 0xff);

    // to_tcp_frame += tcp_data->rtu_id;
    // to_tcp_frame += tcp_data->func_code;
    // to_tcp_frame += (tcp_data->len * 2) & 0xff;

    /* 加数据 */
    for (int i = 0; i < tcp_data->len * 2; i++)
    {
        to_tcp_frame.push_back(to_tcp_data[i]);
    }

    std::string to_data(to_tcp_frame.begin(), to_tcp_frame.end());

    debug("%s\n", "要发送给上位机的功图数据>>");

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
    debug("--- %s ---\n",
          " ##########modbus TCP  has been sent############");

    return 0;
}

/**
 * @brief 将临时数据中的数据返给上位机
 *
 * @param tcp_data 指向保存上位机数据的结构体
 * @param query 指向保存上位机的临时数据
 * @param rc 接收到的数据长度
 * @return int 成功：0；失败：-1
 */
int MultiTask::sel_data_to_tcp(struct tcp_data *tcp_data)
{
    int ret = 0;

    int id = 0;

    /* 用于获取当前时间 */

    // time_t cur_time = time(nullptr);

    /* 用于组合发送给上位机的数据帧 */

    std::vector<uint8_t> to_tcp_frame;

    /* 组装帧头 */
    to_tcp_frame.push_back(tcp_data->frame_header[0]);
    to_tcp_frame.push_back(tcp_data->frame_header[1]);
    to_tcp_frame.push_back(tcp_data->frame_header[2]);
    to_tcp_frame.push_back(tcp_data->frame_header[3]);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) >> 8);
    to_tcp_frame.push_back((tcp_data->len * 2 + 3) & 0xff);

    /* 加站号，功能码，数据长度 */
    to_tcp_frame.push_back(tcp_data->rtu_id);
    to_tcp_frame.push_back(tcp_data->func_code);
    to_tcp_frame.push_back((tcp_data->len * 2) & 0xff);

    int i = 0;

    while (i < c_info.well_max_num)
    {
        if (tcp_data->rtu_id != c_info.w_info[i].id)
        {
            i++;
            continue;
        }
        break;
    }

    if (i >= c_info.well_max_num)
    {
        return -1;
    }

    while (tcp_data->rtu_id != c_info.w_info[id].id)
    {
        id++;
    }
    debug("id>>%d\n", id);

    if ((tcp_data->func_code == 0x03) && (c_info.w_info[id].type == 0))
    {
        /* 上锁 */
        m_rd_db.lock();

        /* 判断查询的数据为哪一部分 */
        if ((tcp_data->start_addr >= 0) &&
            ((tcp_data->start_addr + tcp_data->len) <= 200))
        {
            /* 基础数据已经准备好，可以读取并发给上位机 */
            for (uint16_t i = tcp_data->start_addr;
                 i < (tcp_data->start_addr + tcp_data->len); i++)
            {
                to_tcp_frame.push_back(to_db.rd_db[id].basic_value[i] >> 8);
                to_tcp_frame.push_back(to_db.rd_db[id].basic_value[i] & 0xff);
            }
        }
        else if ((tcp_data->start_addr >= 200) &&
                 ((tcp_data->start_addr + tcp_data->len) <= 210))
        {
            /* 基础数据(功图基础数据1)已经准备好，可以读取并发给上位机 */
            for (uint16_t i = (tcp_data->start_addr - 200);
                 i < ((tcp_data->start_addr - 200) + tcp_data->len);
                 i++)
            {
                to_tcp_frame.push_back(
                    to_db.rd_db[id].ind_diagram_basic_val[i] >> 8);
                to_tcp_frame.push_back(
                    to_db.rd_db[id].ind_diagram_basic_val[i] & 0xff);
            }
        }
        else if ((tcp_data->start_addr >= 1410) &&
                 ((tcp_data->start_addr + tcp_data->len) <= 1420))
        {
            /* 基础数据(功图基础数据2)已经准备好，可以读取并发给上位机 */
            for (uint16_t i = (tcp_data->start_addr - 1410);
                 i < ((tcp_data->start_addr - 1410) + tcp_data->len);
                 i++)
            {
                to_tcp_frame.push_back(
                    to_db.rd_db[id].ind_diagram_basic_val[i + 7] >> 8);
                to_tcp_frame.push_back(
                    to_db.rd_db[id].ind_diagram_basic_val[i + 7] & 0xff);
            }
        }

        /* 解锁 */
        m_rd_db.unlock();

        if ((tcp_data->start_addr >= 210) &&
            ((tcp_data->start_addr + tcp_data->len) <= 1410))
        {
            /* 功图数据已经准备好 */

            // if ((cur_time - to_db.rd_db[id].cur_time_diagram) < 60)
            // {
            //     /* 如果临时缓存区里的功图数据采集时间小于60秒，则返回缓存区里的功图数据 */
            //     for (uint16_t i = (tcp_data->start_addr - 210);
            //          i < ((tcp_data->start_addr - 210) + tcp_data->len);
            //          i++)
            //     {
            //         to_tcp_frame.push_back(
            //             to_db.rd_db[id].ind_diagram_basic_val[i] >> 8);
            //         to_tcp_frame.push_back(
            //             to_db.rd_db[id].ind_diagram_basic_val[i] & 0xff);
            //     }
            // }
            // else
            // {
            //     if (to_xbee(tcp_data) < 0)
            //     {
            //         ret = -1;
            //     }
            //     return 0;
            // }

            debug("%s\n", "*******接收并发送功图数据*******");

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

            // return -1;
        }
    }
    else if ((tcp_data->func_code == 0x03) && (c_info.w_info[id].type == 2))
    {
        /* 上锁 */
        m_rd_db.lock();

        /* 判断查询的数据为哪一部分 */
        if ((tcp_data->start_addr >= 0) &&
            ((tcp_data->start_addr + tcp_data->len) <= 60))
        {
            /* 阀组间数据已经准备好，可以读取并发给上位机 */
            for (uint16_t i = tcp_data->start_addr;
                 i < (tcp_data->start_addr + tcp_data->len); i++)
            {
                to_tcp_frame.push_back(
                    to_db.rd_db[id].valve_group_data[i] >> 8);
                to_tcp_frame.push_back(
                    to_db.rd_db[id].valve_group_data[i] & 0xff);
            }
        }
        else if ((tcp_data->start_addr >= 100) &&
                 ((tcp_data->start_addr + tcp_data->len) <= 107))
        {
            /* 基础数据(功图基础数据2)已经准备好，可以读取并发给上位机 */
            for (uint16_t i = (tcp_data->start_addr - 100);
                 i < ((tcp_data->start_addr - 100) + tcp_data->len);
                 i++)
            {
                to_tcp_frame.push_back(
                    to_db.rd_db[id].valve_group_data[i + 60] >> 8);
                to_tcp_frame.push_back(
                    to_db.rd_db[id].valve_group_data[i + 60] & 0xff);
            }
        }
        else if ((tcp_data->start_addr >= 200) &&
                 ((tcp_data->start_addr + tcp_data->len) <= 207))
        {
            /* 基础数据(功图基础数据2)已经准备好，可以读取并发给上位机 */
            for (uint16_t i = (tcp_data->start_addr - 200);
                 i < ((tcp_data->start_addr - 200) + tcp_data->len);
                 i++)
            {
                to_tcp_frame.push_back(
                    to_db.rd_db[id].valve_group_data[i + 67] >> 8);
                to_tcp_frame.push_back(
                    to_db.rd_db[id].valve_group_data[i + 67] & 0xff);
            }
        }
        else if ((tcp_data->start_addr >= 300) &&
                 ((tcp_data->start_addr + tcp_data->len) <= 307))
        {
            /* 基础数据(功图基础数据2)已经准备好，可以读取并发给上位机 */
            for (uint16_t i = (tcp_data->start_addr - 300);
                 i < ((tcp_data->start_addr - 300) + tcp_data->len);
                 i++)
            {
                to_tcp_frame.push_back(
                    to_db.rd_db[id].valve_group_data[i + 74] >> 8);
                to_tcp_frame.push_back(
                    to_db.rd_db[id].valve_group_data[i + 74] & 0xff);
            }
        }
        else
        {
            ret = -1;
        }

        /* 解锁 */
        m_rd_db.unlock();
    }
    else if ((tcp_data->func_code == 0x04) && (tcp_data->rtu_id <= 16) &&
             (tcp_data->rtu_id > 0))
    {
        if ((tcp_data->start_addr >= 0) &&
            ((tcp_data->len + tcp_data->start_addr) <= 8))
        {
            /* 上锁 */
            m_rd_db.lock();

            /* 汇管压力数据已经准备好，可以读取并发给上位机 */
            for (uint16_t i = tcp_data->start_addr;
                 i < (tcp_data->start_addr + tcp_data->len); i++)
            {
                to_tcp_frame.push_back(
                    to_db.rd_db[tcp_data->rtu_id].manifold_pressure[i] >> 8);
                to_tcp_frame.push_back(
                    to_db.rd_db[tcp_data->rtu_id].manifold_pressure[i] & 0xff);
            }

            /* 解锁 */
            m_rd_db.unlock();
        }
        else
        {
            return -1;
        }
    }
    else
    {
        // ret = -1;
        return -1;
    }

    // if(ret == -1)
    // {
    //     return -1;
    // }

    /* 数据异常，发送0 */
    if (ret == -1)
    {
        uint8_t n = 0;

        for (int i = 0; i < tcp_data->len * 2; i++)
        {
            to_tcp_frame.push_back(n);

            // to_tcp_frame += n;
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
    int crcCode = 0;

    char *rrr;

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

    /* 开始组装RTU数据 */
    if (curr_info->store_type == TYPE_INDICATOR_DIAGRAM)
    {
        to_xbee_data[0] = curr_info->id_indicator_diagram;

        /* 要发送的64位地址 */
        for (int i = 0; i < c_info.well_max_num; i++)
        {
            if (c_info.w_info[i].id == curr_info->id_indicator_diagram)
            {
                addr64 = XBeeAddress64(
                    c_info.w_info[i].addr >> 32,
                    c_info.w_info[i].addr &
                    0xffffffff);
                break;
            }
        }

        // addr64 = XBeeAddress64(
        //     slave_addr[curr_info->id_indicator_diagram] >> 32,
        //     slave_addr[curr_info->id_indicator_diagram] &
        //     0xffffffff);
    }

    // else if (curr_info->store_type == TYPE_VALVE_GROUP_DATA)
    // {
    //     to_xbee_data[0] = class_id.valve_group_id[curr_info->id_oil_well
    // -
    // 1];
    // }
    else
    {
        to_xbee_data[0] = curr_info->id_oil_well;
    }

    /* 要发送的64位地址 */
    for (int i = 0; i < c_info.well_max_num; i++)
    {
        if (c_info.w_info[i].id == curr_info->id_oil_well)
        {
            addr64 = XBeeAddress64(
                c_info.w_info[i].addr >> 32,
                c_info.w_info[i].addr &
                0xffffffff);
            break;
        }
    }

    // addr64 = XBeeAddress64(
    //     slave_addr[curr_info->id_oil_well] >> 32,
    //     slave_addr[curr_info->id_oil_well] &
    //     0xffffffff);

    to_xbee_data[1] = curr_info->func_code;
    to_xbee_data[2] = curr_info->start_addr >> 8;
    to_xbee_data[3] = curr_info->start_addr & 0xff;
    to_xbee_data[4] = curr_info->addr_len >> 8;
    to_xbee_data[5] = curr_info->addr_len & 0xff;
    crcCode = crc16(to_xbee_data, 6);
    to_xbee_data[6] = crcCode >> 8;
    to_xbee_data[7] = crcCode & 0xFF;

    uint16_t addr16 = strtol(c_info.xbee_id.c_str(), &rrr, 16) & 0xffff;

    debug("%s\n", "发送的数据帧>>");

    /* 出现错误时，重新发送并接收一遍数据，当错误超过两次时，便放弃当前数据，采下一组数据 */
    while (err_num < 2)
    {
        /* ############ 上锁 ############ */
        m_tx_rx.lock();

        /* 发送数据 */
        xbeeTx(*xbee_handler, to_xbee_data, 8, addr64, addr16);

        debug("%s\n",         "");
        debug("%s\n", "接收的数据帧>>");

        /* 等待接收数据 */
        ret =
            xbeeRx(*xbee_handler, xbee_rtu_data, &xbeeRtulen, &slave_addr64);

        /* ############ 解锁 ########### */
        m_tx_rx.unlock();

        if (ret <= 0)
        {
            err_num++;
            continue;
        }
        break;
    }

    if (err_num >= 2)
    {
        debug("%s\n", "xbee rx error");

        curr_info->phase = PHASE_START;
        curr_info->isGetTime = false;
        return -1;
    }

    for (int i = 0; i < c_info.well_max_num; i++)
    {
        if (c_info.w_info[i].id == curr_info->id_oil_well)
        {
            /* 如果发送的是广播，则保存对应井口的64位地址 */
            if ((curr_info->store_type == TYPE_BASIC_DATA) &&
                (c_info.w_info[i].addr == 0xffff))
            {
                c_info.w_info[i].addr = slave_addr64;
            }
            break;
        }
    }

    /* ################ 上锁 ################# */

    // lock_guard<mutex> m_db_lock(m_data_block);

    // 将数据存入临时缓存
    for (int i = 0; i < xbeeRtulen - 5; i += 2)
    {
        uint8To16 = (xbee_rtu_data[i + 3] << 8) |
                    (xbee_rtu_data[i + 4] & 0xff);

        if (curr_info->store_type == TYPE_BASIC_DATA)
        {
            data_block[curr_info->id].basic_value.push_back(uint8To16);
        }
        else if (curr_info->store_type == TYPE_INDICATOR_DIAGRAM_BASIC_DATA)
        {
            data_block[curr_info->id].ind_diagram_basic_val.push_back(
                uint8To16);
        }
        else if (curr_info->store_type == TYPE_INDICATOR_DIAGRAM)
        {
            // TODO：此处的id_indicator_diagtam后面酌情考虑修改
            data_block[curr_info->id_ind].ind_diagram.push_back(
                uint8To16);
        }
        else if (curr_info->store_type == TYPE_WATER_WELL_DATA)
        {
            data_block[curr_info->id].water_well_data.push_back(
                uint8To16);
        }
        else if (curr_info->store_type == TYPE_MANIFOLD_PRESSURE)
        {
            data_block[curr_info->id].manifold_pressure.push_back(
                uint8To16);
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
    if (config_manage() < 0)
    {
        is_error = true;
        exit(-1);
    }

    /* ------------ 初始化xbee ------------ */
    xbee_handler = new XBee();

    if (xbee_handler == nullptr)
    {
        fprintf(stderr, "xbee_hanler new error\n");
        is_error = true;
        exit(-1);
    }

    /* ------------ 初始化uart ------------ */
    xbee_ser = new uart();

    if (xbee_ser == nullptr)
    {
        fprintf(stderr, "xbee_ser new error\n");
        is_error = true;
        exit(-1);
    }

    xbee_handler->setSerial(xbee_ser);

    /* 打开串口 */
    if (xbee_ser->Open("/dev/ttymxc6", 9600) < 0)
    {
        fprintf(stderr, "serial_open(): %s\n", xbee_ser->errmsg());
        is_error = true;
        exit(-1);
    }

    /* ------------ 初始化sqlite3 ------------ */
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
        is_error = true;
        exit(-1);
    }

    /*开启 高速，写同步 */
    if (sql_handler->writeSync() < 0)
    {
        is_error = true;
        exit(-1);
    }

    /* 创建数据表 */
    strcpy(sql_tab_name.tab_basic,                               "basicInfo");
    strcpy(sql_tab_name.tab_indicator_diagram_basic, "indicatorDiagramBasic");
    strcpy(sql_tab_name.tab_indicator_diagram,            "indicatorDiagram");
    strcpy(sql_tab_name.tab_water_well,                          "waterWell");
    strcpy(sql_tab_name.tab_valve_group,                        "valveGroup");

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

    free(tcp_data.value);

    modbus_free(ctx);
    ctx = nullptr;

    if (DI_AI_DO.AI)
    {
        spi_close(spi_ai.spi);
        spi_free(spi_ai.spi);
    }

    if (DI_AI_DO.DI || DI_AI_DO.DO)
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

    delete c_info.w_info;
    c_info.w_info = nullptr;
}

/**
 * @brief 初始化线程
 *
 */
void MultiTask::initThread()
{
    debug("%s\n", "This is initThread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /* -----------------初始化xbee模块------------------- */

    /* 接收到的AT命令ACK */
    uint8_t recv_ack[9] = { 0 };

    /* 初始化的AT命令 */
    uint8_t uint_at_command[9][15];

    for (int i = 0; i < 9; i++)
    {
        memset(uint_at_command[i], 0, 15);
    }
    string atsc = "ATSC" + c_info.xbee_sc + "\r";
    string atce = "ATCE" + to_string(c_info.xbee_ce) + "\r";
    string atao = "ATAO" + to_string(c_info.xbee_ao) + "\r";
    string atid = "ATID" + c_info.xbee_id + "\r";
    strcpy((char *)uint_at_command[0],     "+++");
    strcpy((char *)uint_at_command[1],  "ATRE\r");
    strcpy((char *)uint_at_command[2],      atsc.c_str());
    strcpy((char *)uint_at_command[3], "ATAP1\r");
    strcpy((char *)uint_at_command[4],      atce.c_str());
    strcpy((char *)uint_at_command[5],      atao.c_str());
    strcpy((char *)uint_at_command[6],      atid.c_str());
    strcpy((char *)uint_at_command[7],  "ATWR\r");
    strcpy((char *)uint_at_command[8],  "ATCN\r");

    /* 如果发送失败,重发 */
    for (int i = 0; i < 3; i++)
    {
        memset(recv_ack, 0, 9);

        /* 进入AT命令模式 */
        xbee_ser->Write(uint_at_command[0], 3);

        if (xbee_ser->Read(recv_ack, 10, 1000) <= 0)
        {
            debug("%s\n", "uart read error or timeout");
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
    }

    /* ------------初始化modbus TCP服务器---------------- */
    /* IP地址 */
    memset(server_info.ip_addr, 0, 16);
    strcpy(server_info.ip_addr, c_info.ip.c_str());

    /* 端口号 */
    server_info.port = c_info.port;

    /* -------------初始化DI，DO，AI的配置--------------- */
    DI_AI_DO.DI = false;
    DI_AI_DO.DO = false;

    if (c_info.manifold_pressure == 1)
    {
        DI_AI_DO.AI = true;
    }
    else
    {
        DI_AI_DO.AI = false;
    }

    /* --------------初始化上位机发送来的数据------------ */
    tcp_data.rtu_id = 0;
    tcp_data.func_code = 0x03;
    tcp_data.start_addr = 0;
    tcp_data.len = 0;
    tcp_data.value = nullptr;

    /* --------------初始化状态机状态信息---------------- */
    current_info.phase = PHASE_START;
    current_info.isGetTime = true;
    current_info.id = 0;
    current_info.id_oil_well = 0;
    current_info.func_code = 0x03;
    current_info.store_type = TYPE_BASIC_DATA;
    current_info.id_ind = 0;
    current_info.is_again = false;
    current_info.is_add_id = false;

    /* -------给data_block在堆区分配内存，并初始化------- */

    /* 在堆区分配一段内存用于存储所有配置的井口基础数据 */
    data_block = new struct data_block[c_info.well_max_num];
    data_block_2 = new struct data_block[c_info.well_max_num];

    if ((data_block == nullptr) || (data_block_2 == nullptr))
    {
        debug("%s\n", "new data_block error");
        return;
    }

    /* 初始化 */
    for (int i = 0; i < c_info.well_max_num; i++)
    {
        data_block[i].rtu_id = 0;
        data_block[i].cur_time_diagram = 0;
        data_block[i].basic_value.resize(200);
        data_block[i].ind_diagram_basic_val.resize(20);
        data_block[i].ind_diagram.resize(800);
        data_block[i].water_well_data.resize(100);
        data_block[i].valve_group_data.resize(100);
        data_block[i].manifold_pressure.resize(10);

        data_block_2[i].rtu_id = 0;
        data_block_2[i].cur_time_diagram = 0;
        data_block_2[i].basic_value.resize(200);
        data_block_2[i].ind_diagram_basic_val.resize(20);
        data_block_2[i].ind_diagram.resize(800);
        data_block_2[i].water_well_data.resize(100);
        data_block_2[i].valve_group_data.resize(100);
        data_block_2[i].manifold_pressure.resize(10);
    }

    /* 初始化写指针指向data_block,读指针指向data_block_2 */
    to_db.wr_db = data_block;
    to_db.rd_db = data_block_2;

    /* ----------------- i2c初始化 --------------------- */
    if (DI_AI_DO.DI || DI_AI_DO.DO)
    {
        i2c_D = i2c_init("/dev/i2c-2", 0x74, 0xff, 0x03);

        if (i2c_D == nullptr)
        {
            debug("%s\n", "i2c_D init error");
            return;
        }
    }

    /* ------------------ spi初始化 --------------------- */
    if (DI_AI_DO.AI)
    {
        spi_ai.spi = spi_init("/dev/spidev0.0");

        if (spi_ai.spi == nullptr)
        {
            printf("spi_init error\n");
            return;
        }

        spi_ai.val[0] = 0;
        spi_ai.val[1] = 0;
        spi_ai.buf_rx[0] = 0;
        spi_ai.buf_rx[1] = 0;

        spi_ai.buf_tx = spi_buf_tx;

        for (int i = 0; i < 10; i++)
        {
            debug("%x\n", spi_ai.buf_tx[i][0]);
        }
    }

    /* ------------------ rs485初始化 ------------------ */
    if (c_info.valve_group == 1)
    {
        rs485_info.fd = open_port("/dev/ttymxc1");

        // rs485_info.fd = open_port("/dev/ttyUSB1");

        if (rs485_info.fd < 0)
        {
            perror("open failed");
            return;
        }

        rs485_enable(rs485_info.fd, ENABLE);

        int i = set_port(rs485_info.fd, 9600, 8, 'N', 1);

        if (i < 0)
        {
            perror("set_port failed");
            return;
        }
    }

    /* ---------------- 站号类别初始化 ------------------ */
    memset(class_id.oil_well_id,    0, sizeof(class_id.oil_well_id));
    memset(class_id.water_well_id,  0, sizeof(class_id.water_well_id));
    memset(class_id.valve_group_id, 0, sizeof(class_id.valve_group_id));

    for (int i = 0; i < c_info.well_max_num; i++)
    {
        if (c_info.w_info[i].type == 0)
        {
            class_id.oil_well_id[i] = c_info.w_info[i].id;
        }
        else if (c_info.w_info[i].type == 1)
        {
            class_id.water_well_id[i] = c_info.w_info[i].id;
        }
        else if (c_info.w_info[i].type == 2)
        {
            class_id.valve_group_id[i] = c_info.w_info[i].id;
        }
    }
}

/**
 * @brief 获取井口数据线程
 *
 */
void MultiTask::getWellPortInfoThread()
{
    debug("%s\n", "This is getWellPortInfoThread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /* 用于超时的计时器 */
    time_t t_start = 0;

    /* 用于存入数据库的 string */
    std::string sql = "";

    /* 返回值 */
    int ret = 0;

    while (1)
    {
        switch (current_info.phase)
        {
        case PHASE_START:
        {
            if (current_info.isGetTime)
            {
                t_start = time(nullptr);
            }

            if (current_info.is_add_id)
            {
                current_info.id++;
            }

            /* 用于切换1到最后一口井口从机地址 */
            if (current_info.id < c_info.well_max_num)
            {
                current_info.id_oil_well = c_info.w_info[current_info.id].id;
                current_info.is_add_id = true;
            }
            else
            {
                current_info.id = 0;
                current_info.id_oil_well = c_info.w_info[current_info.id].id;

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

            if ((current_info.id_oil_well ==
                 125) |
                (current_info.id_oil_well ==
                 126) |
                (current_info.id_oil_well ==
                 127) |
                (current_info.id_oil_well ==
                 128))
            {
                current_info.phase = PHASE_VALVE_GROUP;
            }
            else
            {
                current_info.phase = PHASE_BASIC1;
            }
            break;
        }

        case PHASE_BASIC1:
        {
            to_db.wr_db[current_info.id].basic_value.clear();
            to_db.wr_db[current_info.id].ind_diagram_basic_val.clear();

            /* 设置当前阶段的状态信息 */
            current_info.start_addr = 0x00;
            current_info.addr_len = 0x1e;
            current_info.store_type = TYPE_BASIC_DATA;

            ret = state_machine_operation(&current_info, to_db.wr_db);

            if (ret < 0)
            {
                break;
            }

            current_info.phase = PHASE_BASIC2;
            break;
        }

        case PHASE_BASIC2:
        {
            current_info.start_addr = 0x1e;
            current_info.addr_len = 0x1f;

            ret = state_machine_operation(&current_info, to_db.wr_db);

            if (ret < 0)
            {
                break;
            }

            current_info.phase = PHASE_BASIC3;
            break;
        }

        case PHASE_BASIC3:
        {
            current_info.start_addr = 0x3d;
            current_info.addr_len = 0x20;

            ret = state_machine_operation(&current_info, to_db.wr_db);

            if (ret < 0)
            {
                break;
            }

            current_info.phase = PHASE_BASIC4;
            break;
        }

        case PHASE_BASIC4:
        {
            current_info.start_addr = 0x5d;
            current_info.addr_len = 0x21;

            ret = state_machine_operation(&current_info, to_db.wr_db);

            if (ret < 0)
            {
                break;
            }

            current_info.phase = PHASE_BASIC5;
            break;
        }

        case PHASE_BASIC5:
        {
            current_info.start_addr = 0x7e;
            current_info.addr_len = 0x20;

            ret = state_machine_operation(&current_info, to_db.wr_db);

            if (ret < 0)
            {
                break;
            }

            current_info.phase = PHASE_INDICATOR_DIAGRAM_BASIC_1;
            break;
        }

        case PHASE_INDICATOR_DIAGRAM_BASIC_1:
        {
            current_info.start_addr = 0xc8;
            current_info.addr_len = 0x07;
            current_info.store_type = TYPE_INDICATOR_DIAGRAM_BASIC_DATA;

            ret = state_machine_operation(&current_info, to_db.wr_db);

            if (ret < 0)
            {
                break;
            }

            current_info.phase = PHASE_INDICATOR_DIAGRAM_BASIC_2;
            break;
        }

        case PHASE_INDICATOR_DIAGRAM_BASIC_2:
        {
            current_info.time = getCurTime(nullptr);
            current_info.start_addr = 0x0582;
            current_info.addr_len = 0x08;

            ret = state_machine_operation(&current_info, to_db.wr_db);

            if (ret < 0)
            {
                break;
            }

            /* 汇管压力(AI) 配置 */
            if (current_info.id_oil_well == c_info.manifold_pressure_id)
            {
                current_info.store_type = TYPE_MANIFOLD_PRESSURE;
                to_db.wr_db[current_info.id].manifold_pressure.clear();

                current_info.start_addr = 0x00;
                current_info.addr_len = 0x06;
                current_info.func_code = 0x04;

                ret = state_machine_operation(&current_info, to_db.wr_db);

                if (ret < 0)
                {
                    break;
                }
            }
            current_info.func_code = 0x03;

            /* 唤醒所有线程. */
            cv_basic_data.notify_all();

            /* 判断是否到达10(暂定为1分钟)分钟 */
            if (time(nullptr) - t_start < 600)
            {
                current_info.phase = PHASE_START;

                current_info.isGetTime = false;
            }
            else
            {
                if (current_info.id == current_info.id_ind)
                {
                    current_info.phase = PHASE_INDICATOR_DIAGRAM;
                }
                else
                {
                    current_info.phase = PHASE_START;
                }
            }
            break;
        }

        case PHASE_INDICATOR_DIAGRAM:
        {
            current_info.store_type = TYPE_INDICATOR_DIAGRAM;
            to_db.wr_db[current_info.id_ind].ind_diagram.clear();

            current_info.id_indicator_diagram =
                c_info.w_info[current_info.id_ind].id;

            /* 分包循环采集所有的工图数据 */
            for (int i = 0; i < 25; i++)
            {
                current_info.start_addr = 0xd2 + i * 32;
                current_info.addr_len = 32;

                ret = state_machine_operation(&current_info, to_db.wr_db);

                if (ret < 0)
                {
                    break;
                }
            }

            if (ret < 0)
            {
                if (current_info.id_ind < c_info.well_max_num - 1)
                {
                    current_info.id_ind++;
                }
                else
                {
                    current_info.id_ind = 0;
                    current_info.isGetTime = true;
                }
                current_info.phase = PHASE_START;
            }
            else
            {
                current_info.phase = PHASE_POWER_DIAGRAM;
            }

            break;
        }

        case PHASE_POWER_DIAGRAM:
        {
            /* 分包循环采集所有的功率图数据 */
            for (int i = 0; i < 25; i++)
            {
                current_info.start_addr = 0x3f2 + i * 32;
                current_info.addr_len = 16;

                ret = state_machine_operation(&current_info, to_db.wr_db);

                if (ret < 0)
                {
                    break;
                }
            }

            /* 记录当前的功图采集时间，以及功图采集已完成 */
            to_db.wr_db[current_info.id_ind].cur_time_diagram = time(
                nullptr);

            if (current_info.id_ind < c_info.well_max_num - 1)
            {
                current_info.id_ind++;
            }
            else
            {
                current_info.id_ind = 0;
                current_info.isGetTime = true;
            }

            current_info.phase = PHASE_START;
            break;
        }

        case PHASE_VALVE_GROUP:
        {
            if (c_info.valve_group == 1)
            {
                /* 初始化，清零 */
                to_db.wr_db[current_info.id].valve_group_data.clear();

                /* 阀组间的基本数据 */
                for (int i = 0; i < 2; i++)
                {
                    /* 设置当前阶段的状态信息 */
                    current_info.start_addr = 0x00 + i * 0x1e;
                    current_info.addr_len = 0x1e;
                    current_info.store_type = TYPE_VALVE_GROUP_DATA;

                    ret = rs485(&current_info, to_db.wr_db);

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

                /* 阀组间的各注水井的数据，此处为2口井 */
                for (int i = 0; i < 2; i++)
                {
                    /* 设置当前阶段的状态信息 */
                    current_info.start_addr = (0x64 + i * 0x64);
                    current_info.addr_len = 0x7;

                    ret = rs485(&current_info, to_db.wr_db);

                    if (ret < 0)
                    {
                        break;
                    }
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

/**
 * @brief 处理上位机请求线程
 *
 */
void MultiTask::hostRequestProcThread()
{
    debug("%s\n", "This is hostRequestProcThread");
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

                    tcp_data.value = (uint16_t *)calloc(
                        tcp_data.len, sizeof(uint16_t));

                    if (tcp_data.value == nullptr)
                    {
                        debug("%s\n", "calloc tcp_data.value error");
                        continue;
                    }

                    /*  */
                    sel_data_to_tcp(&tcp_data);

                    free(tcp_data.value);
                    tcp_data.value = nullptr;
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

                // TODO: 自己实现数据的分析处理/发送
            }
        }
    }
}

#if 0

/**
 * @brief 数据库存储线程
 *
 */
void MultiTask::sqlMemoryThread()
{
    debug("%s\n", "This is sqlMemoryThread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /* 用于组装sql语句 */
    std::string sql = "";

    int err_num = 0;

    /* 获取当前时间 */
    time_t c_time = 0;

    /* 保存要删除的语句 */
    std::string del_statement = "";

    /* 用于保存当前井口编号 */
    int id_oil_well = 0;

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
        id_oil_well = current_info.id_oil_well;

        id = current_info.id;

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

        cycles[0] = data_block[id].basic_value.size() - 1;
        cycles[1] = data_block[id].ind_diagram_basic_val.size() - 1;

        cur_store_type = current_info.store_type;

        // id_ind_diagrom = current_info.id_indicator_diagram;
        // TODO：测试
        id_ind_diagrom = id_oil_well;

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
                    sql += std::to_string(id_oil_well);
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
                    sql += std::to_string(data_block[id].basic_value[i]);
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

#endif // if 0

/**
 * @brief 升级线程
 *
 */
void MultiTask::updateThread()
{
    debug("%s\n", "This is updateThread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
