#include "rs485.h"

int set_usart_port(int fd, int baud_rate, int data_bits, int parity,
                   int stop_bits)
{
    struct termios newtio, oldtio;

    /*获取终端属性*/
    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("tcgetattr error");
        return -1;
    }

    bzero(&newtio, sizeof(newtio));

    /*设置控制模式*/
    newtio.c_cflag |= CLOCAL | CREAD; // 保证程序不占用串口
    // newtio.c_cflag |= CSIZE;		  //保证程序可以从串口中读取数据
    newtio.c_cflag &= ~CSIZE;         // 保证程序可以从串口中读取数据
    // newtio.c_cflag &= ~CRTSCTS;

    // newtio.c_iflag &= ~(INLCR | ICRNL);		   //不要回车和换行转换
    // newtio.c_iflag &= ~(IXON | IXOFF | IXANY); //不要软件流控制

    // newtio.c_oflag &= ~OPOST;						
    //   //*设置输出模式为原始输出*/
    // newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //原始模式

    /*设置等待时间和最小接受字符*/
    newtio.c_cc[VTIME] = 0; // 可以在select中设置
    newtio.c_cc[VMIN] = 0;  // 最少读取0个字符

    /*设置输入输出波特率，两者保持一致*/
    switch (baud_rate)
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;

    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;

    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;

    case 19200:
        cfsetispeed(&newtio, B19200);
        cfsetospeed(&newtio, B19200);
        break;

    case 38400:
        cfsetispeed(&newtio, B38400);
        cfsetospeed(&newtio, B38400);
        break;

    case 57600:
        cfsetospeed(&newtio, B57600);
        cfsetispeed(&newtio, B57600);
        break;

    case 230400:
        cfsetospeed(&newtio, B230400);
        cfsetispeed(&newtio, B230400);
        break;

    default:
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    }

    /*设置数据位，如果不在范围内按 8 处理*/
    switch (data_bits)
    {
    case 7:
        newtio.c_cflag &= ~CSIZE; // 屏蔽其它标志位
        newtio.c_cflag |= CS7;
        break;

    default:
    case 8:
        newtio.c_cflag &= ~CSIZE; // 屏蔽其它标志位
        newtio.c_cflag |= CS8;
        break;
    }

    /*设置校验位*/
    switch (parity)
    {
    /*无奇偶校验位*/
    default:
    case 1:
    {
        newtio.c_cflag &= ~PARENB; // PARENB：产生奇偶位，执行奇偶校验
        newtio.c_iflag &= ~INPCK;  // INPCK：使奇偶校验起作用
        break;
    }

    /*设置奇校验*/
    case 2:
    {
        newtio.c_cflag |= PARENB; // PARENB：产生奇偶位，执行奇偶校验
        newtio.c_cflag |= PARODD; // PARODD：若设置则为奇校验,否则为偶校验
        newtio.c_cflag |= INPCK;  // INPCK：使奇偶校验起作用
        newtio.c_cflag |= ISTRIP; // ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
        break;
    }

    /*设置偶校验*/
    case 3:
    {
        newtio.c_cflag |= PARENB;  // PARENB：产生奇偶位，执行奇偶校验
        newtio.c_cflag &= ~PARODD; // PARODD：若设置则为奇校验,否则为偶校验
        newtio.c_cflag |= INPCK;   // INPCK：使奇偶校验起作用
        newtio.c_cflag |= ISTRIP;  // ISTRIP：若设置则有效输入数字被剥离7个字节，否则保留全部8位
        break;
    }
    }

    /*设置停止位*/
    switch (stop_bits)
    {
    default:
    case 1:
    {
        newtio.c_cflag &= ~CSTOPB;
        break;
    }

    case 2:
    {
        newtio.c_cflag |= CSTOPB;
        break;
    }
    }

    /*如果发生数据溢出，只接受数据，但是不进行读操作*/
    tcflush(fd, TCIFLUSH);

    /*激活配置*/
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("usart set error");
        return -1;
    }

    return 0;
}

int open_usart_port(int com_port)
{
    int   fd = 0;
    const char *dev[] = { "/dev/ttymxc1", "/dev/ttymxc2", "/dev/ttymxc3" };

    if ((com_port < 0) || (com_port > 3))
    {
        perror("the port is out range");
        return -1;
    }

    /*打开串口*/

    // fd = open(dev[com_port], O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(dev[com_port], O_RDWR);

    if (fd < 0)
    {
        perror("open serial port fail");
        return -1;
    }

    /*清除串口非阻塞标志*/
    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl F_SETFL");
        return -1;
    }
    return fd;
}

int rs485_enable(const int fd)
{
    struct serial_rs485 rs485conf;
    int res = -1;

    /* Get configure from device */
    res = ioctl(fd, TIOCGRS485, &rs485conf);

    if (res < 0)
    {
        perror("Ioctl error on getting 485 configure:");
        close(fd);
        return res;
    }

    //  rs485 enable mode
    rs485conf.flags |= SER_RS485_ENABLED;

    // rs485conf.flags &= ~(SER_RS485_ENABLED);

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

int rs485_change_bit(const int fd, int value)
{
    struct serial_rs485 rs485conf;
    int res = -1;

    /* Get configure from device */
    res = ioctl(fd, TIOCGRS485, &rs485conf);

    if (res < 0)
    {
        perror("Ioctl error on getting 485 configure:");
        close(fd);
        return res;
    }

    //  rs485 send mode
    if (value == 1)
    {
        rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
    }

    //  rs485 read mode
    else if (value == 0)
    {
        rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);
    }
    else
    {
        close(fd);
        return res;
    }

    /* Set configure to device */
    res = ioctl(fd, TIOCSRS485, &rs485conf);

    if (res < 0)
    {
        perror("Ioctl error on setting 485 configure:");
        close(fd);
    }

    return res;
}

int usart_init(int        com_port,
                __uint32_t BaudRate,
                __uint16_t WordLength,
                __uint16_t StopBits,
                __uint16_t Parity)
{
	int fd;
    int Ubr;     // 波特率
    int Uwl;     // 数据长度
    int Usb;     // 停止位
    int Uparity; // 校验位

    Ubr = BaudRate;

    if (WordLength == 8) Uwl = 8;
    else Uwl = 7;

    if (StopBits == 1) Usb = 1;
    else Usb = 2;

    if (Parity == 1) Uparity = 1;
    else if (Parity == 2) Uparity = 2;
    else Uparity = 3;

    if (com_port == 0)
    {
        if ((fd = open_usart_port(com_port)) == -1)
        {
            perror("open usart485_1port error");
            //exit(EXIT_FAILURE);
			return -EXIT_FAILURE;
        }

        if (rs485_enable(fd) < 0)
        {
            perror("enable usart485_1 port error");
            //exit(EXIT_FAILURE);
			return -EXIT_FAILURE;
        }

        if (set_usart_port(fd, Ubr, Uwl, Uparity, Usb) == -1)
        {
            perror("set usart485_1port error");
            //exit(EXIT_FAILURE);
			return -EXIT_FAILURE;
        }
        printf("RS485_1 INIT SUCCEED\n");
    }
    else
    {
        printf("usart port is out range");
        //exit(EXIT_FAILURE);
		return -EXIT_FAILURE;
    }
	return fd;
}

int usart_send_data(int usart_fd, u_int8_t *SendBuff, int datalen)
{
    if (datalen <= 0)
    {
        return -1;
    }

    if (usart_fd > 0)
    {
        if (write(usart_fd, SendBuff, datalen) < 0)
        {
            printf("usart send fail!!!\n");
			return -1;
        }
        printf("send ok\n");
        tcdrain(usart_fd); // tcdrain()是等待fd所引用的串口设备数据传输完毕
        rs485_change_bit(usart_fd, 0);
    }
	return 0;
}

int usart_rev_data(int usart_fd, u_int8_t *readBuffer)
{
    int  i, ReadByte = 0;

    if (usart_fd > 0)
    {
        ReadByte = read(usart_fd, readBuffer, 1024);

        if (ReadByte > 0)
        {
            printf("usart  %d  :", ReadByte);

            for (i = 0; i < ReadByte; i++)
            {
                printf("%02x ", readBuffer[i]);
            }
            printf("\n");
        }
    }
	return ReadByte;
}
