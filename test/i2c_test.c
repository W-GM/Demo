/**
 * @file i2c_test.c
 * @author wgm (wgm136@136.com)
 * @brief 基于PCA9539APW的驱动程序
 * @version 0.1
 * @date 2020-05-11
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "i2c.h"

#define PAC9539


#ifdef PAC9539 /* 整理后的代码，基于PAC9539编写 */
static struct i2c_msg msg;

/**
 * @brief i2c管理
 * 
 */
struct i2c_handler{
    i2c_t *i2c; /* 指向i2c_new成功后的指针 */
    uint16_t dev_addr;/* 7位设备地址 */
};

/**
 * @brief 初始化并设置i2c设备的各引脚为输入还是输出
 *
 * @param path 指向i2c设备节点的路径
 * @param dev_addr 7位设备地址
 * @param reg_val_0 设置各P0口是输入还是输出的值（按位对齐，1为输入，0为输出）
 * @param reg_val_1 设置各P1口是输入还是输出的值（按位对齐，1为输入，0为输出）
 * @return i2c_t* 成功：i2c_handler结构体指针；失败：-1
 */
struct i2c_handler* i2c_init(const char *path,
                uint16_t    dev_addr,
                uint8_t     reg_val_0,
                uint8_t     reg_val_1)
{
    uint8_t buf[2] = { 0 };

    struct i2c_handler *i2c_handler = (struct i2c_handler *)calloc(1, sizeof(struct i2c_handler));
    if(i2c_handler == NULL)
    {
        fprintf(stderr, "i2c_handler calloc error\n");
        return NULL;
    }

    i2c_handler->dev_addr = dev_addr;

    i2c_handler->i2c = i2c_new();
    if(i2c_handler->i2c == NULL)
    {
        fprintf(stderr, "i2c_new error\n");
        return NULL;
    }

    /* open the i2c-2 bus */
    if (i2c_open(i2c_handler->i2c, path) < 0)
    {
        fprintf(stderr, "i2c_open():%s\n", i2c_errmsg(i2c_handler->i2c));
        return NULL;
    }
    else
    {
        printf("i2c bus opened\n");
    }

    /************* 配置i2c设备为输入还是输出 **************/

    /* 向配置寄存器6中写设定值,让对应位为1的所有P0口为输入口，其余口为输出口 */
    buf[0] = 0x06;
    buf[1] = reg_val_0;

    msg.addr  = dev_addr; /* 7位的设备地址 */ 
    msg.flags = 0;        /* 读写位（0为写） */
    msg.buf   = buf;      /* 存放寄存器的地址，和要发送给寄存器的值 */
    msg.len   = 2;        /* buf所对应的长度，以字节为单位 */

    if (i2c_transfer(i2c_handler->i2c, &msg, 1) < 0)
    {
        fprintf(stderr, "i2c_transfer():%s\n", i2c_errmsg(i2c_handler->i2c));
        return NULL;
    }

    /* 向配置寄存器7中写设定值,让对应位为1的所有P0口为输入口，其余口为输出口 */

    buf[0] = 0x07;
    buf[1] = reg_val_1;
    msg.buf = buf;
    msg.len = 2;

    if (i2c_transfer(i2c_handler->i2c, &msg, 1) < 0)
    {
        fprintf(stderr, "i2c_transfer():%s\n", i2c_errmsg(i2c_handler->i2c));
        return NULL;
    }

    return i2c_handler;
}

/**
 * @brief i2c写（输出）函数
 * 
 * @param i2c_handler 指向i2c_handler结构体指针
 * @param tx_buf 指向存放寄存器的地址，和要发送给寄存器的值的数组
 * @param tx_len 数组长度
 * @return int 成功：0；失败：-1
 */
int i2c_write(struct i2c_handler *i2c_handler, uint8_t *tx_buf, uint16_t tx_len)
{
    struct i2c_msg msg;
    msg.flags = 0;
    msg.addr = i2c_handler->dev_addr;
    msg.buf = tx_buf;
    msg.len = tx_len;

    if (i2c_transfer(i2c_handler->i2c, &msg, 1) < 0)
    {
        fprintf(stderr, "i2c_transfer():%s\n", i2c_errmsg(i2c_handler->i2c));
        return -1;
    }

    return 0;
}

/**
 * @brief i2c读（输入）函数
 * 
 * @param i2c_handler 指向i2c_handler结构体指针
 * @param reg_addr 寄存器地址
 * @param rx_data 指向接收到的数据的指针
 * @param rx_len 要接收的数据长度
 * @return int 成功：0；失败：-1;
 */
int i2c_read(struct i2c_handler *i2c_handler, uint8_t reg_addr, uint8_t *rx_data, uint16_t rx_len)
{
    /* 读取数据 */

    struct i2c_msg msgs[2] =
    {
        { .addr = i2c_handler->dev_addr, .flags = 0,
          .len = 1,
          .buf = &reg_addr },
        { .addr = i2c_handler->dev_addr, .flags = I2C_M_RD,
          .len = rx_len,
          .buf = rx_data },
    };

    if (i2c_transfer(i2c_handler->i2c, msgs, 2) < 0)
    {
        fprintf(stderr, "i2c_transfer(): %s\n", i2c_errmsg(i2c_handler->i2c));
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("参数错误\n");
        return -1;
    }
    uint8_t msg_data[1] = { 0 };

    struct i2c_handler *i2c_di = i2c_init("/dev/i2c-1", 0x68, 0xff, 0x03);

    if (i2c_di == NULL)
    {
        printf("error\n");
        return -1;
    }

    if (i2c_read(i2c_di, 0x01, msg_data, 1) < 0)
    {
        printf("error\n");
        return -1;
    }
    printf("%02x\n", msg_data[0]);

    i2c_close(i2c_di->i2c);
    i2c_free(i2c_di->i2c);
    free(i2c_di);
    i2c_di = NULL;
    return 0;
}
#endif
#if 0 /* 原始的i2c代码 */
int main(int argc, char **argv)
{
    i2c_t *i2c;

    i2c = i2c_new();

    /* open the i2c-3 bus */
    if (i2c_open(i2c, "/dev/i2c-1") < 0)
    {
        fprintf(stderr, "i2c_open():%s\n", i2c_errmsg(i2c));
        exit(1);
    }
    else
    {
        printf("i2c-2 bus opened\n");
    }

    /* 写数据寄存器 6 0xff */

    // 向配置寄存器6中写0xff,让所有的P0口为输入口

    // uint8_t buf[2] = { 0x00, (0x05 << 4) | 0x03 };
// 
    // struct i2c_msg msg;
    // msg.addr = PCA9539A_I2C_ADDR; // 7位的设备地址
    // msg.flags = 0;                // 读写位（0为写）
    // msg.buf = buf;                // 存放寄存器的地址，和要发送给寄存器的值
    // msg.len = 2;
// 
    // if (i2c_transfer(i2c, &msg, 1) < 0)
    // {
    //     fprintf(stderr, "i2c_transfer():%s\n", i2c_errmsg(i2c));
    //     exit(1);
    // }
// 
    // /* 写数据寄存器 7 0x03 */
// 
    // // 向配置寄存器7中写0x03,让P1口的第一和第二引脚为输入口，其余口为输出口
// 
    // buf[0] = 0x07;
    // buf[1] = 0x03;
    // msg.buf = buf;
    // msg.len = 2;
// 
    // if (i2c_transfer(i2c, &msg, 1) < 0)
    // {
    //     fprintf(stderr, "i2c_transfer():%s\n", i2c_errmsg(i2c));
    //     exit(1);
    // }
// 
    // /* 写数据寄存器 3 0x00 */
// 
    // buf[0] = 0x03;
    // buf[1] = 0xff;
    // msg.buf = buf;
    // msg.len = 2;
// 
    // if (i2c_transfer(i2c, &msg, 1) < 0)
    // {
    //     fprintf(stderr, "i2c_transfer():%s\n", i2c_errmsg(i2c));
    //     exit(1);
    // }

    /* 读取数据 */
    uint8_t msg_addr[1] = { 0x00 };
    uint8_t msg_data[1] = { 0x00 };

    struct i2c_msg msgs[2] =
    {
        { .addr = PCA9539A_I2C_ADDR, .flags = 0,
          .len = 1, .buf = msg_addr },
        { .addr = PCA9539A_I2C_ADDR, .flags = I2C_M_RD,
          .len = 1, .buf = msg_data },
    };

    if (i2c_transfer(i2c, msgs, 2) < 0)
    {
        fprintf(stderr, "i2c_transfer(): %s\n", i2c_errmsg(i2c));
        exit(1);
    }

    printf("0x%02x: %d\n", msg_addr[0], ((msg_data[0] >> 4) * 10 + (msg_data[0] & 0xf)));

    i2c_close(i2c);
    i2c_free(i2c);
    return 0;
}

#endif /* if 0 */
