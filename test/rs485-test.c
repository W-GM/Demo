#include <stdio.h>
#include "rs485.h"

int main(int argc, char const *argv[])
{
    int i = 0, j = 0;
    int rs485_fd = usart_init(0, 9600, 8, 1, 1);

    if (rs485_fd < 0)
    {
        printf("rs485 uart init error\n");
        return -1;
    }

    u_int8_t sendata[3][8] = { {0x10, 0x04, 0x00, 0x06, 0x00, 0x08, 0x12, 0x8c},
                               {0x10, 0x04, 0x00, 0x0e, 0x00, 0x08, 0x93, 0x4e},
                               {0x10, 0x04, 0x00, 0x18, 0x00, 0x08, 0x72, 0x8a}};
    u_int8_t recvdata[256];

    while (1)
    {
        if (i >= 3)
        {
            i = 0;
        }
        printf("send[%d] ", i);

        for (j = 0; j < 8; j++)
        {
            printf("[%.2x]", sendata[i][j]);
        }
        puts("");

        if(usart_send_data(rs485_fd, sendata[i], 8) < 0)
        {
            printf("send rs485 data error\n");
            return -1;
        }
        usleep(100000);

        if (usart_rev_data(rs485_fd, recvdata) < 0)
        {
            printf("recv rs485 data error\n");
            return -1;
        }
        i++;
        sleep(1);
    }

    return 0;
}
