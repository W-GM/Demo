/**
 * @file spi_test.c
 * @author wgm (wgm136@136.com)
 * @brief 针对ADC7490所写的驱动程序
 * @version 0.1
 * @date 2020-05-11
 *
 * @copyright Copyright (c) 2020
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <unistd.h>

#include "spi.h"

// #define CYCLE /* 循环采集 */

#define SINGLE /* 单次采集 */

/* 保存发送给控制寄存器的数据，用于按指定位单独采集 */
// uint8_t spi_buf_tx[][2] = { 0x83, 0x30,
//                             0x87, 0x30,
//                             0x8b, 0x30,
//                             0x8f, 0x30,
//                             0x93, 0x30,
//                             0x97, 0x30,
//                             0x9b, 0x30,
//                             0x9f, 0x30,
//                             0xa3, 0x30,
//                             0xa7, 0x30 };

// struct spi_handler
// {
//     spi_t   *spi;       /* 指向spi_new成功的指针 */
//     uint8_t  buf_rx[2]; /* 用于保存接收到的值，前4位代表地址，后12位代表数据 */
//     uint8_t *buf_tx;    /* 指向保存发送给控制寄存器的数据，用于按指定位单独采集 */
// };


/**
 * @brief 基于ADC7490的初始化函数
 *
 * @param path spi的设备地址及路径
 * @return spi_t* 成功：spi_new成功后的指针；失败：NULL
 */
// spi_t* spi_init(const char *path)
// {
//     /* 用于保存初始化的值 */
//     uint8_t init[2] = { 0xff, 0xff };
// 
//     spi_t *spi = spi_new();
// 
//     if (spi == NULL)
//     {
//         fprintf(stderr, "spi new error\n");
//         return NULL;
//     }
// 
//     /* Open spidev1.0 with mode 2 and max speed 1MHz */
//     if (spi_open_advanced(spi, path, 2, 1000000, MSB_FIRST, 8, 0) < 0)
//     {
//         fprintf(stderr, "spi_open(): %s\n", spi_errmsg(spi));
//         return NULL;
//     }
// 
//     /* 初始化，将控制寄存器的所有位 置1 */
//     if (spi_transfer(spi, init, NULL, sizeof(init)) < 0)
//     {
//         fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi));
//         return NULL;
//     }
// 
//     return spi;
// }

int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        printf("parameter too few\n");
        return -1;
    }
    else if(argc > 2)
    {
        printf("parameter too much\n");
        return -1;
    }

    int n = argv[1][0] - 0x31;
    printf("n = %d\n", n);

    double add;

    struct spi_handler spi_ai;
    spi_ai.buf_rx[0] = 0;
    spi_ai.buf_rx[1] = 0;

    spi_ai.spi = spi_init("/dev/spidev0.0");
    if (spi_ai.spi == NULL)
    {
        printf("spi_init error\n");
        return -1;
    }

    /* 保存发送给控制寄存器的数据，用于按指定位单独采集 */
    spi_ai.buf_tx = spi_buf_tx;

    for (int i = 0; i < 2; i++)
    {
        if (spi_transfer(spi_ai.spi, spi_ai.buf_tx[n], spi_ai.buf_rx,
                         sizeof(spi_ai.buf_rx)) < 0)
        {
            fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi_ai.spi));
            exit(1);
        }
    }
    add = (((spi_ai.buf_rx[0] << 8) | (spi_ai.buf_rx[1] & 0xff)) & 0xfff) * 0.005055147;
    printf("adc[%d] >> %f\n", (spi_ai.buf_rx[0] >> 4), add);

    spi_close(spi_ai.spi);
    spi_free(spi_ai.spi);

    return 0;
}

#if 0

int main(void)
{
    double add;

    struct spi_handler spi_ai;

    spi_ai.spi = spi_init("/dev/spidev0.0");

    if (spi_ai.spi == NULL)
    {
        printf("spi_init error\n");
        return -1;
    }

    spi_ai.buf_rx[0] = 0;
    spi_ai.buf_rx[1] = 0;


#ifdef CYCLE

    /* 保存发送给控制寄存器的数据，用于按位循环采集 */
    uint8_t buf_tx_cycle[][2] = { 0xd3, 0xb0,
                                  0x00, 0x00 };

    /* 设置控制寄存器为写操作(WRITE=1),
     * 设置从0开始到指定地址的顺序转换(SEQ=SHADOW=1),
     * 设置指定的地址位(ADD3 TO ADD0)
     * 设置正常模式，不断电(PM1=PM0=1)
     * 设置串行传输结束时的输出状态，结束时处于三态(0)
     * 模拟输入范围从0 V扩展至REF IN (1)，设置为0时为之前的2倍，但VDD=4.75 V至5.25 V.
     * 设置转换结果的输出编码类型(0),设置为0：二进制补码，设置为1：直接为二进制
     */
    if (spi_transfer(spi, buf_tx_cycle[0], buf_rx, sizeof(buf_rx)) < 0)
    {
        fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi));
        exit(1);
    }

    /* 循环读取选择位的转换值 */
    while (1)
    {
        /* 此时要将写操作置0(WRITE=0) ,只循环读*/
        if (spi_transfer(spi, buf_tx_cycle[1], buf_rx, sizeof(buf_rx)) < 0)
        {
            fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi));
            exit(1);
        }

        add = (((buf_rx[0] << 8) | (buf_rx[1] & 0xff)) & 0xfff) * 0.005055147;
        printf("adc[%d] >> %f\n", (buf_rx[0] >> 4), add);

        sleep(1);

        printf("----------\n");
    }
#endif /* ifdef CYCLE */

#ifdef SINGLE
    int n = 9;

    /* 保存发送给控制寄存器的数据，用于按指定位单独采集 */
    spi_ai.buf_tx = spi_buf_tx;
    
    for (int j = 0; j < n; j++)
    {
        for (int i = 0; i < 2; i++)
        {
            if (spi_transfer(spi_ai.spi, spi_ai.buf_tx[j], spi_ai.buf_rx,
                             sizeof(spi_ai.buf_rx)) < 0)
            {
                fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi_ai.spi));
                exit(1);
            }
        }
        add = (((spi_ai.buf_rx[0] << 8) | (spi_ai.buf_rx[1] & 0xff)) & 0xfff) * 0.005055147;
        printf("adc[%d] >> %f\n", (spi_ai.buf_rx[0] >> 4), add);
    }
    printf("----------\n");

#endif /* ifdef SINGLE */

    spi_close(spi_ai.spi);

    spi_free(spi_ai.spi);

    return 0;
}
#endif