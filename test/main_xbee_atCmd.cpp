#include <iostream>
#include <string.h>
#include "xbee.h"
#include "uart.h"
#include "screen.h"
#include "i2c.h"

extern void xbeeRx(XBee                & xbee,
                   ZBRxIoSampleResponse& rxios,
                   Screen *scr);
extern void xbeeTx(XBee        & xbee,
                   uint8_t      *payload,
                   uint8_t       payloadlen,
                   XBeeAddress64 addr64);
extern void xbeeAtCmd(XBee   & xbee,
                      uint8_t *atCmd,
                      uint8_t *setVale,
                      uint8_t  valeLength);
extern void xbeeRemoteAtCmd(XBee         & xbee,
                            XBeeAddress64& remoteAddr,
                            uint8_t       *atCmd,
                            uint8_t       *setVale,
                            uint8_t        valeLength);

extern int i2c_start(i2c_t** i2c);

// 存储从从i2c_start函数中返回的分配到堆区的地址数组
i2c_t* i2cp[3] = {NULL, NULL, NULL};

int main(int argc, const char *argv[])
{
    int i = 0;
    uint8_t *setatcmd = NULL;     // 设置AT 命令
    uint8_t  setvale[64] = { 0 }; // AT 命令值
    uint8_t *setvalep = NULL;     // AT命令值值指针，未加命令值时为NULL，加命令值指向setvale
    uint8_t  setvalelen = 0;      // AT 命令值长度
    uint8_t  atCmd[][2] = {
        { 'S', 'H' }, { 'S', 'L' }, { 'S', 'M' }, { 'I', 'D' }, { 'C', 'E' }, 
        {'A', 'O'}, {'W', 'R'}
    };

    std::string arg1, arg2, arg3;

    if (argc >= 2)
    {
        arg1 = argv[1];
    }

    if (argc >= 3)
    {
        arg2 = argv[2];
    }

    if (argc == 4)
    {
        arg3 = argv[3];

        if (arg3 == "1010")
        {
            setvale[0] = 0x10;
            setvale[1] = 0x10;
            setvalelen = 2;
        }
        else if (arg3 == "1313")
        {
            setvale[0] = 0x13;
            setvale[1] = 0x13;
            setvalelen = 2;
        }
        else if (arg3 == "1")
        {
            setvale[0] = 0x01;
            setvalelen = 1;
        }
        else if (arg3 == "0")
        {
            setvale[0] = 0x00;
            setvalelen = 1;
        }

        setvalep = setvale;    
    }
    // 发送的数据
    uint8_t senddata[] = { 'a', 'v', 't', 'v' };

    XBee xbee = XBee();

    // SH + SL 64位远程地址
    XBeeAddress64 remoteAddr = XBeeAddress64(0x00000000, 0x0000ffff);
    // 发送数据时填充的64位地址
    // 0xffff为广播地址、0x0为终端节点发送协调器的地址、其他为指定地址
    XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x0000ffff);

    ZBRxIoSampleResponse rxios;

    // 初始化i2c并设置值
    i2c_start(i2cp);

    // 初始化屏幕串口
    Screen scr = Screen("/dev/ttymxc4");

    if (scr.IsOK() == true)
    {
        printf("握手失败\n");
    }
    else
    {
        printf("握手成功\n");
    }

    // 设置屏幕的背光强度
    scr.SetBGBrightness(0xff);

    // 清屏
    scr.ClearScreen(0x002f);

    // 初始化串口
    uart *serial = new uart();

    xbee.setSerial(serial);

    if (serial->Open("/dev/ttymxc6", 9600) < 0)
    {
        fprintf(stderr, "serial_open(): %s\n", serial->errmsg());
        exit(1);
    }

    if (arg1 == "tx")
    {
        // 发送帧
        xbeeTx(xbee, senddata, sizeof(senddata), addr64);
    }
    else if (arg1 == "rx")
    {
        // 接收帧
        xbeeRx(xbee, rxios, &scr);
    }
    else if (arg1 == "at")
    {
        if (arg2 == "sh")
        {
            setatcmd = atCmd[0];
        }
        else if (arg2 == "sl")
        {
            setatcmd = atCmd[1];
        }
        else if (arg2 == "sm")
        {
            setatcmd = atCmd[2];
        }
        else if (arg2 == "id")
        {
            setatcmd = atCmd[3];
        }
        else if (arg2 == "ce")
        {
            setatcmd = atCmd[4];
        }
        else if (arg2 == "ao")
        {
            setatcmd = atCmd[5];
        }
        else if (arg2 == "wr")
        {
            setatcmd = atCmd[6];
        }
        else
        {
            printf("at command input error\n");
            return -1;
        }

        // 发送AT Command
        xbeeAtCmd(xbee, setatcmd, setvalep, setvalelen);
    }
    else if (arg1 == "remote")
    {
        if (arg2 == "sh")
        {
            setatcmd = atCmd[2];
        }
        else if (arg2 == "sl")
        {
            setatcmd = atCmd[3];
        }
        else if (arg2 == "sm")
        {
            setatcmd = atCmd[4];
        }
        else if (arg2 == "id")
        {
            setatcmd = atCmd[1];
        }
        else
        {
            printf("at command input error\n");
            return -1;
        }

        // 发送Remote AT Command
        xbeeRemoteAtCmd(xbee, remoteAddr, setatcmd, setvalep, setvalelen);
    }

    // 关闭打开的i2c串口，释放从堆区分配的空间
    i2c_close(i2cp[0]);
    i2c_close(i2cp[1]);
    i2c_free(i2cp[0]);
    i2c_free(i2cp[1]);

    serial->Close();
    free(serial);

    return 0;
}
