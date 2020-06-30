/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "i2c.h"

struct i2c_handle {
    int fd;

    struct {
        int  c_errno;
        char errmsg[96];
    } error;
};

static int _i2c_error(i2c_t *i2c, int code, int c_errno, const char *fmt, ...) {
    va_list ap;

    i2c->error.c_errno = c_errno;

    va_start(ap, fmt);
    vsnprintf(i2c->error.errmsg, sizeof(i2c->error.errmsg), fmt, ap);
    va_end(ap);

    /* Tack on strerror() and errno */
    if (c_errno) {
        char buf[64];
        strerror_r(c_errno, buf, sizeof(buf));
        snprintf(i2c->error.errmsg + strlen(i2c->error.errmsg),
                 sizeof(i2c->error.errmsg) - strlen(i2c->error.errmsg),
                 ": %s [errno %d]",
                 buf,
                 c_errno);
    }

    return code;
}

i2c_t* i2c_new(void) {
    i2c_t *i2c = (i2c_t *)calloc(1, sizeof(i2c_t));

    if (i2c == NULL) return NULL;

    i2c->fd = -1;

    return i2c;
}

void i2c_free(i2c_t *i2c) {
    free(i2c);
}

int i2c_open(i2c_t *i2c, const char *path) {
    unsigned long supported_funcs;

    memset(i2c, 0, sizeof(i2c_t));

    /* Open device */
    if ((i2c->fd = open(path, O_RDWR)) < 0) return _i2c_error(i2c,
                                                              I2C_ERROR_OPEN,
                                                              errno,
                                                              "Opening I2C device \"%s\"",
                                                              path);

    /* Query supported functions */
    if (ioctl(i2c->fd, I2C_FUNCS, &supported_funcs) < 0) {
        int errsv = errno;
        close(i2c->fd);
        return _i2c_error(i2c, I2C_ERROR_QUERY, errsv, "Querying I2C functions");
    }

    if (!(supported_funcs & I2C_FUNC_I2C)) {
        close(i2c->fd);
        return _i2c_error(i2c,
                          I2C_ERROR_NOT_SUPPORTED,
                          0,
                          "I2C not supported on %s",
                          path);
    }

    return 0;
}

int i2c_transfer(i2c_t *i2c, struct i2c_msg *msgs, size_t count) {
    struct i2c_rdwr_ioctl_data i2c_rdwr_data;

    /* Prepare I2C transfer structure */
    memset(&i2c_rdwr_data, 0, sizeof(struct i2c_rdwr_ioctl_data));
    i2c_rdwr_data.msgs = msgs;
    i2c_rdwr_data.nmsgs = count;

    /* Transfer */
    if (ioctl(i2c->fd, I2C_RDWR, &i2c_rdwr_data) < 0) return _i2c_error(i2c,
                                                                        I2C_ERROR_TRANSFER,
                                                                        errno,
                                                                        "I2C transfer");

    return 0;
}

int i2c_close(i2c_t *i2c) {
    if (i2c->fd < 0) return 0;

    /* Close fd */
    if (close(i2c->fd) < 0) return _i2c_error(i2c,
                                              I2C_ERROR_CLOSE,
                                              errno,
                                              "Closing I2C device");

    i2c->fd = -1;

    return 0;
}

int i2c_tostring(i2c_t *i2c, char *str, size_t len) {
    return snprintf(str, len, "I2C (fd=%d)", i2c->fd);
}

const char* i2c_errmsg(i2c_t *i2c) {
    return i2c->error.errmsg;
}

int i2c_errno(i2c_t *i2c) {
    return i2c->error.c_errno;
}

int i2c_fd(i2c_t *i2c) {
    return i2c->fd;
}

/*********** custom function [wgm-2020-05-07] *************/

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
    struct i2c_msg msg;

    struct i2c_handler *i2c_handler =
        (struct i2c_handler *)calloc(1, sizeof(struct i2c_handler));

    if (i2c_handler == NULL)
    {
        fprintf(stderr, "i2c_handler calloc error\n");
        return NULL;
    }

    i2c_handler->dev_addr = dev_addr;

    i2c_handler->i2c = i2c_new();

    if (i2c_handler->i2c == NULL)
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

    msg.addr = dev_addr; /* 7位的设备地址 */
    msg.flags = 0;       /* 读写位（0为写） */
    msg.buf = buf;       /* 存放寄存器的地址，和要发送给寄存器的值 */
    msg.len = 2;         /* buf所对应的长度，以字节为单位 */

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
int i2c_read(struct i2c_handler *i2c_handler,
             uint8_t             reg_addr,
             uint8_t            *rx_data,
             uint16_t            rx_len)
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
