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

    if(app.get_error())
    {
        exit(1);
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

    //std::thread sqlmemorythread(&MultiTask::sqlMemoryThread, &app);

    // std::thread updatethread(&MultiTask::updateThread, &app);


    getwellportinfothread.join();
    hostrequestprocthread.join();
    //sqlmemorythread.join();
    //updatethread.join();

    return 0;
}
