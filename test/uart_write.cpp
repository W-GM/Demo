
#include <unistd.h>
#include <iostream>
#include "uart.h"
using namespace std;

int main()
{
    //int fd = 0;
    uint8_t buf[] = "123456";
    uart serial = uart();
    
    serial.Open("/dev/ttymxc3", 9600);

    serial.Write(buf, sizeof(buf));

    serial.Close();


    return 0;

}