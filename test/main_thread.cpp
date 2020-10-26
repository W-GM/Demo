/**
 * @file main_thread.cpp
 * @author wgm (wangguomin@scaszh.com)
 * @brief This is the CQOF wellsite RTU main program
 * @version 0.5
 * @date 2020-06-03
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <signal.h>

#include "multitask.h"

static void close_sigint(int dummy)
{
    printf("\nprogram is exited!\n");

    exit(5);
}

int main()
{
    
    MultiTask app;

    const char *error_type[] = {"config", "xbee", "i2c(di,do)", "spi(ai)", "485_0", "sql", "232", "485_1", "wifi"};
    int is_error = app.get_error();
    if(is_error > 0 && is_error < 9)
    {   
        printf("init %s error\n", error_type[is_error -1]);
        exit(-1);
    }
    else if(is_error != 0)
    {
        printf("init other error\n");
        exit(-1);
    }


    /* 设置子项 */
    // xmle2->SetText("192.11.11.1");

    /* 保存 */
    // doc.SaveFile("/home/wgm/wgm/PROJECT/CQ_PROJECT/build/myconfig.xml");

    signal(SIGINT, close_sigint);


    std::thread Init_thread(&MultiTask::thread_init, &app);
    Init_thread.join();

    std::thread Get_wellport_info_thread(&MultiTask::thread_get_wellport_info, &app);

#if 0   
    std::thread Host_request_thread(&MultiTask::thread_host_request, &app);
    std::thread Watchdogctl_thread(&MultiTask::thread_watchdogctl, &app);
    std::thread Update_thread(&MultiTask::thread_update, &app);
#endif

#ifdef SQL
    std::thread Sql_memory_thread(&MultiTask::thread_sql_memory, &app);
#endif
   
    Get_wellport_info_thread.join();
#if 0 
    Host_request_thread.join();
    Watchdogctl_thread.join();
    Update_thread.join();
#endif

#ifdef SQL
    Sql_memory_thread.join();
#endif

    return 0;
}
