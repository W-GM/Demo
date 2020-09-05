#include <iostream>
#include <string.h>
#include "xbee.h"
#include "uart.h"
#include "xbee_op.h"

#define ARMCQ

int main(int argc, const char *argv[])
{
    char m = 0;
    long ret = 0, n = 0;
    char   *str;
    char setatcmd[4] = {0};     // AT 命令
    char  setvale[10] = { 0 }; // AT 命令值
    uint8_t  setvalelen = 0;      // AT 命令值长度

    char xbee_data[256] = {0}; // 用于保存发送或接收的数据
    int xbee_len = 0; // 用于保存数据长度
    uint64_t slave_addr64 = 0; //用于保存接收到的从机地址

     // SH + SL 64位远程地址
    XBeeAddress64 remoteAddr = XBeeAddress64(0x00000000, 0x0000ffff);
    // 发送数据时填充的64位地址
    // 0xffff为广播地址、0x0为终端节点发送协调器的地址、其他为指定地址
    XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x0000ffff);

    ZBRxIoSampleResponse rxios;

    std::string arg1, arg2, arg3;

    if(argc < 2)
    {
        printf("too few parameter!\n");
    }
    else if(argc > 4)
    {
        printf("too much parameter!\n");
    }

    if(argc == 2)
    {
        arg1 = argv[1];
    }
    else if(argc == 3)
    {
        arg1 = argv[1];
        arg2 = argv[2];
    }
    else if(argc == 4)
    {
        arg1 = argv[1];
        arg2 = argv[2];
        arg3 = argv[3];
    }

    /* ------------ 初始化xbee ------------ */
    XBee *xbee_handler = nullptr;
    xbee_handler = new XBee();
    if (xbee_handler == nullptr)
    {
        fprintf(stderr, "xbee_hanler new error\n");
        return -1;
    }

    /* ------------ 初始化xbee对应的uart ------------ */
    uart *xbee_ser = nullptr;
    xbee_ser = new uart();
    if (xbee_ser == nullptr)
    {
        fprintf(stderr, "xbee_ser new error\n");
        return -1;
    }
    xbee_handler->setSerial(xbee_ser);

 /* 打开串口 */
#ifdef ARMCQ

    if (xbee_ser->Open("/dev/ttymxc6", 9600) < 0)
#else // ifdef ARMCQ

    if (xbee_ser->Open("/dev/ttyUSB0", 9600) < 0)
#endif // ifdef ARMCQ
    {
        fprintf(stderr, "serial_open(): %s\n", xbee_ser->errmsg());
        return -1;
    }

    if(arg1 == "tx")
    {
        strcpy(xbee_data, arg2.c_str());
        xbee_len = arg2.size();
        printf("发送的数据帧 >> ");
         /* 发送数据 */
        xbeeTx(*xbee_handler, (uint8_t *)xbee_data, xbee_len, addr64, DEFAULT_FRAME_ID);
        puts("");
        
        /* 等待接收数据 */
        ret = xbeeRx(*xbee_handler, nullptr, nullptr, nullptr);

    }
    else if(arg1 == "rx")
    {
        while (1)
        {
            printf("接受的数据帧 >> ");
            /* 等待接收数据 */
            ret = xbeeRx(*xbee_handler, (uint8_t *)xbee_data, &xbee_len, &slave_addr64);
            puts("");
        }
        
    }
    else if(arg1 == "remote")
    {
        if(argc == 3)
        {
            strcpy(setatcmd, arg2.c_str());

            // 发送Remote AT Command
            xbeeRemoteAtCmd(*xbee_handler, remoteAddr, (uint8_t *)setatcmd, nullptr, 0);
        }
        else if(argc == 4)
        {
            strcpy(setatcmd, arg2.c_str());
            strcpy(setvale, arg3.c_str());
            setvalelen = arg3.size();

            // 发送Remote AT Command
            xbeeRemoteAtCmd(*xbee_handler, remoteAddr, (uint8_t *)setatcmd, (uint8_t *)setvale, setvalelen);
        }
    }
    else
    {
        if(argc == 2)
        {
            strcpy(setatcmd, arg1.c_str());

            // 发送AT Command
            xbeeAtCmd(*xbee_handler, (uint8_t *)setatcmd, nullptr, 0);
        }
        else if(argc == 3)
        {
            strcpy(setatcmd, arg1.c_str());
            ret = strtol(arg2.c_str(), &str, 16);
            memcpy(setvale, &ret, sizeof(long));
            
            for(int i = 1; i <= 8; i++)
            {
                m = (ret >> (64 - i * 8)) & 0xff;
                if(m != 0)
                {
                    setvale[n] = m;
                    n++;
                }
            }
            setvalelen = n;

            // 发送AT Command
            xbeeAtCmd(*xbee_handler, (uint8_t *)setatcmd, (uint8_t *)setvale, setvalelen);
        }
    }

    delete xbee_ser;
    xbee_ser = nullptr;

    delete xbee_handler;
    xbee_handler = nullptr;
    return 0;
}





