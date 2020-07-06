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

    const char *error_type[] = {"config", "xbee", "i2c", "spi", "485", "sql"};
    int is_error = app.get_error();
    if(is_error > 0 && is_error < 7)
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

    std::thread initthread(&MultiTask::initThread, &app);
    initthread.join();

    std::thread getwellportinfothread(&MultiTask::getWellPortInfoThread, &app);

    std::thread hostrequestprocthread(&MultiTask::hostRequestProcThread, &app);
#ifdef SQL
    std::thread sqlmemorythread(&MultiTask::sqlMemoryThread, &app);
#endif
    // std::thread updatethread(&MultiTask::updateThread, &app);


    getwellportinfothread.join();
    hostrequestprocthread.join();
#ifdef SQL
    sqlmemorythread.join();
#endif
    //updatethread.join();

    return 0;
}
