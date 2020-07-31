#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char const *argv[])
{
    char buf[] = {1};
    int fd = open("/dev/watchdogctl", O_RDWR);
    if(fd == -1)
    {
        perror("open watchdogctl error");
        return -1;
    }
    while (1)
    {
        write(fd, buf, sizeof(buf));
        sleep(10);
        buf[0] = buf[0]?0:1;
    }

    close(fd);

    return 0;
}
