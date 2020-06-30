#include <unistd.h>
#include <iostream>
#include "uart.h"
using namespace std;

int main()
{
    int fd = 0;
    uint8_t buf[56];
    int ret = 0;
    uart serial = uart();
    
    fd = serial.Open("/dev/ttymxc3", 9600);

    ret = serial.Read(buf, sizeof(buf), 5000);

    buf[ret] = '\0';

    printf("%d:%s\n",ret, buf);

    serial.Close();


    return 0;

}