/********************************************************************
 *              copyright (C) 2014 all rights reserved
 *			 @file: rs485.c
 *                @Created: 2014-8-6 14:00
 *                 @Author: conway chen
 *        @Description: test the rs485 function of sending and receiving
 *	  @Modify Date: 2014-8-6 14:00
 *********************************************************************/
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

/* Table of CRC values for high-order byte */
static const u_int8_t table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static const u_int8_t table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

u_int16_t crc16(u_int8_t *buffer, u_int16_t buffer_length)
{
    u_int8_t crc_hi = 0xFF; /* high CRC byte initialized */
    u_int8_t crc_lo = 0xFF; /* low CRC byte initialized */
    unsigned int i;        /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ table_crc_hi[i];
        crc_lo = table_crc_lo[i];
    }

    return crc_hi << 8 | crc_lo;
}

/**
 * @brief: set the properties of serial port
 * @Param: fd: file descriptor
 * @Param: nSpeed: Baud Rate
 * @Param: nBits: character size
 * @Param: nEvent: parity of serial port
 * @Param: nStop: stop bits
 */

typedef enum { DISABLE = 0, ENABLE } RS485_ENABLE_t;

int set_port(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;

    memset(&oldtio, 0, sizeof(oldtio));

    /* save the old serial port configuration */
    if (tcgetattr(fd, &oldtio) != 0) {
        perror("set_port/tcgetattr");
        return -1;
    }

    memset(&newtio, 0, sizeof(newtio));

    /* ignore modem control lines and enable receiver */
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    /* set character size */
    switch (nBits) {
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
    switch (nEvent) {
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
    switch (nStop) {
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
    switch (nSpeed) {
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
int open_port(char *dir)
{
    int fd;

    fd = open(dir, O_RDWR);

    if (fd < 0) {
        perror("open_port");
    }
    return fd;
}

/**
 * @brief: print usage message
 * @Param: stream: output device
 * @Param: exit_code: error code which want to exit
 */
void print_usage(FILE *stream, int exit_code)
{
    fprintf(stream, "Usage: option [ dev... ] \n");
    fprintf(stream,
            "\t-h  --help     Display this usage information.\n"
            "\t-d  --device   The device ttyS[0-3] or ttySCMA[0-1]\n"
            "\t-b  --baudrate Set the baud rate you can select\n"
            "\t               [230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300]\n"
            "\t-s  --string   Write the device data\n"
            "\t-e  --1 or 0   Write 1 to enable the rs485_mode(only at atmel)\n");
    exit(exit_code);
}

/**
 * @brief: main function
 * @Param: argc: number of parameters
 * @Param: argv: parameters list
 */
int rs485_enable(const int fd, const RS485_ENABLE_t enable)
{
    struct serial_rs485 rs485conf;
    int res;

    /* Get configure from device */
    res = ioctl(fd, TIOCGRS485, &rs485conf);

    if (res < 0) {
        perror("Ioctl error on getting 485 configure:");
        close(fd);
        return res;
    }

    /* Set enable/disable to configure */
    if (enable) { // Enable rs485 mode
        rs485conf.flags |= SER_RS485_ENABLED;
    } else {      // Disable rs485 mode
        rs485conf.flags &= ~(SER_RS485_ENABLED);
    }

    rs485conf.delay_rts_before_send = 0x00000004;

    /* Set configure to device */
    res = ioctl(fd, TIOCSRS485, &rs485conf);

    if (res < 0) {
        perror("Ioctl error on setting 485 configure:");
        close(fd);
    }

    return res;
}

int change_bit_rs485(int fd, int value)
{
    struct serial_rs485 rs485conf;
    int res;

    /* Get configure from device */
    res = ioctl(fd, TIOCGRS485, &rs485conf);

    if (res < 0) {
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

    if (res < 0) {
        perror("Ioctl error on setting 485 configure:");
        close(fd);
    }
    return res;
}


int main(int argc, char *argv[])
{
    u_int8_t write_buf[50] = {0x80, 0x03, 0x00, 0x00, 0x00, 0x10, 0x5a, 0x17};
    u_int8_t  read_buf[100] = {0};
    int   fd, i, nread;
    struct termios oldtio;

    //u_int16_t crcCode = 0;

    /* open serial port */
    fd = open_port("/dev/ttymxc1");
    //fd = open_port("/dev/ttyUSB1");

    if (fd < 0) {
        perror("open failed");
        return -1;
    }

    rs485_enable(fd, ENABLE);
    printf("--------\n");

    /* set serial port */
    i = set_port(fd, 9600, 8, 'N', 1);

    if (i < 0) {
        perror("set_port failed");
        return -1;
    }

    // send  4
    write(fd, write_buf, 8);
    usleep(40000);           // 40ms

    change_bit_rs485(fd, 0); // recv mode

    //		usleep(500000);
    while (1) {
        nread = read(fd, read_buf, sizeof(read_buf));

        //printf("----%d----\n", nread);

        if (nread > 0) {
            printf("RECV[%3d]: ", nread);

            for (int i = 0; i < nread; i++) 
            {
                printf("0x%02x ", read_buf[i]);
            }
            printf("\n");


            /* 组装发送的数据帧 */
            // write_buf[0] = read_buf[0];
            // write_buf[1] = read_buf[1];
            // write_buf[2] =read_buf[5] * 2;
            // for(i = 0; i < write_buf[2]; i++)
            // {
            //     write_buf[i +3] = i;
            // }
            // crcCode = crc16(write_buf, 3 + write_buf[2]);
            // write_buf[i] = crcCode >> 8;
            // write_buf[i + 1] = crcCode & 0xff;


            // send
            write(fd, read_buf, nread);

            usleep(20000);           // 40ms
            change_bit_rs485(fd, 0); // recv mode
        }
        //sleep(1);
    }

    /* restore the old configuration */
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    return 0;
}
