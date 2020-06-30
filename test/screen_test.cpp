#include "screen.h"

struct Point poi = Point(0x00,0x22);

//uint8_t buf[] = "123abc";

std::vector<unsigned char>buf(5,'a');

int number = 0;

int main()
{  
    Screen scr = Screen("/dev/ttymxc4");

    

    if(scr.IsOK() == true)
    {
        printf("握手失败\n");
    }
    else
    {
        printf("握手成功\n");
    }
    
    //设置屏幕的背光强度
    scr.SetBGBrightness(0xff);

    //清屏
    scr.ClearScreen(0x002f);

    //在屏幕显示字符，支持，asciii 和 GB2312 编码
    scr.DisplayString(0, 1, 0x02, 0xffff, 0x0000, poi, buf);

#if 0
    //显示数据变量
    scr.DisplayVariable(1, 1, 0, 0x02, 0xffff, 0x0000, 0x06, 0x02, , number);

    //写数据寄存器
    if(scr.WriteDataRegister(0,) == true)
    {
        printf("写成功\n");
    }
    else
    {
        printf("写失败\n");
    }

    //读数据寄存器
    if(scr.ReadDataRegister(0x5A,) == true)
    {
        printf("参数正确\n");
    }
    else
    {
        printf("参数错误\n");
    }

    //把 32KB SRAM 数据存储器的内容写入指定的图片存储器空间
    if(scr.WritePicRegister() == true)
    {
        printf("成功执行\n");
    }
    else
    {
        printf("执行失败\n");
    }

    //清屏
    scr.ClearScreen();

    //绘制点
    scr.DrawPoint();

    //绘制线
    scr.DrawLine();
#endif








    return 0;
}