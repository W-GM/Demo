#include <stdio.h>
#include <termios.h>
//#include <linux/ioctl.h>
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
 * @brief 设置串口
 * 
 * @param fd 要设置的串口对应的文件描述符
 * @param baud_rate 波特率
 * @param data_bits 数据长度
 * @param parity 校验位
 * @param stop_bits 停止位
 * @return int 成功：0  失败：-1
 */
int set_usart_port(int fd,
                   int baud_rate,
                   int data_bits,
                   int parity,
                   int stop_bits);

/**
 * @brief 打开串口
 * 
 * @param com_port 要打开的哪一个串口 0~2对应ttymxc1~3
 * @return int 成功：串口对应的文件描述符  失败：-1
 */
int open_usart_port(int com_port);

/**
 * @brief rs485串口使能
 * 
 * @param fd 串口对应文件描述符
 * @return int 成功：>=0  失败：<0
 */
int rs485_enable(const int fd);

/**
 * @brief 
 * 
 * @param fd 
 * @param value 
 * @return int 
 */
int rs485_change_bit(const int fd,
                     int       value);

/**
 * @brief rs485串口初始化
 * 
 * @param com_port 要打开的对应串口 0~2 对应 ttymxc1~3
 * @param BaudRate 波特率
 * @param WordLength 数据长度
 * @param StopBits 停止位
 * @param Parity 校验位
 * @return int 成功：对应串口的文件描述符  失败：-1
 */
int usart_init(int        com_port,
               __uint32_t BaudRate,
               __uint16_t WordLength,
               __uint16_t StopBits,
               __uint16_t Parity);

/**
 * @brief 发送数据
 * 
 * @param usart_fd 串口对应文件描述符
 * @param SendBuff 指向发送的数据的内存
 * @param datalen 数据长度
 * @return int 成功：0  失败：-1
 */
int usart_send_data(int       usart_fd,
                     u_int8_t *SendBuff,
                     int       datalen);

/**
 * @brief 接收数据
 * 
 * @param usart_fd 串口对应文件描述符
 * @param readBuffer 指向接收数据的内存
 * @return int 成功：接收到的数据长度  失败：<0
 */
int usart_rev_data(int       usart_fd,
                   u_int8_t *readBuffer);
