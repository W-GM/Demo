/**
 * @file screen.h
 * @author hn
 * @brief 屏幕操作函数库
 * @version 0.1
 * @date 2020-01-04
 *
 * @copyright Copyright (c) 2020
 *
 */
#ifndef SCREEN_H__
#define SCREEN_H__

#include "uart.h"
#include "constants.h"
#include <vector>


/* 定义屏幕中的点 */
struct Point {
    uint16_t _x;
    uint16_t _y;
    Point(uint16_t x, uint16_t y) : _x(x), _y(y) {}
};

/**/
enum Direction {
    Left  = 0x00,
    Right = 0x01,
    Up    = 0x02,
    Down  = 0x03
};

class Screen {
private:

    /**
     * @brief 屏幕水平分辨率，默认为 320
     *
     */
    int _ResH;

    /**
     * @brief 屏幕垂直分辨率，默认为 240
     *
     */
    int _ResV;

    /**
     * @brief 用于给屏幕发送指令的串口
     *
     */
    uart *_ScrSer;

public:
    Screen(std::string name,
           int         bandrate = 115200,
           int         resh = 320,
           int         resv = 240);

    ~Screen();

    /* configure and interface instruction */
    bool IsOK();
    void SetBGBrightness(int value);
    bool WriteDataRegister(int                         type,
                           int16_t                     address,
                           std::vector<unsigned char>& data);
    bool ReadDataRegister(int                         type,
                          int16_t                     address,
                          int                         length,
                          std::vector<unsigned char>& data);
    bool WritePicRegister(int picid);

    // TODO: 扩展串口配置未实现
    // TODO: 扩展串口数据发送
    // TODO: 扩展串口数据接收

    /* draw instruction  */
    void ClearScreen(int16_t color);
    void DrawPoint(int16_t             color,
                   unsigned char       xres,
                   unsigned char       yres,
                   std::vector<Point>& point);

    void DrawLine(int16_t             color,
                  std::vector<Point>& point);

    void DrawRectangle(int                 mode,
                       int16_t             color,
                       std::vector<Point>& point);

    // TODO:屏幕平移有问题
    void MoveScreen(int               mode,
                    Direction         dir,
                    int16_t           dist,
                    int16_t           color,
                    std::vector<Point>point);

    /* 文本相关指令 */
    void DisplayString(int                         adj,
                       int                         visual,
                       int                         size,
                       int16_t                     color,
                       int16_t                     bcolor,
                       Point                     & point,
                       std::vector<unsigned char>& str);

    void DisplayVariable(int     visual,
                         int     signal,
                         int     disyzero,
                         int     zerostyle,
                         int     size,
                         int16_t color,
                         int16_t bcolor,
                         int     integer,
                         int     fl,
                         Point & point,
                         int     var);
};

#endif // ifndef SCREEN_H__
