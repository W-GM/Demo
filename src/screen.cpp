#include "screen.h"
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Construct a new Screen:: Screen object
 *
 * @param name      串口的名字
 * @param bandrate  串口的波特率，默认为 115200
 * @param resh      屏幕的横向分辨率，默认为 320
 * @param resv      屏幕的纵向分辨率，默认为 240
 */
Screen::Screen(std::string name, int bandrate, int resh, int resv) : _ResH(resh),_ResV(resv)
{
    /**
     *  初始化操作屏幕的串口对象，若打开串口失败，则抛出异常
     */

    try
    {
        //_ScrSer = new uart(name.c_str(), bandrate, CS8, 0, 1, 10);
        _ScrSer = new uart();
        if(_ScrSer->Open(name.c_str(), bandrate) < 0)
        {
            fprintf(stderr, "serial_open(): %s\n", _ScrSer->errmsg());
            exit(1);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::abort();
    }
}

/**
 * @brief Destroy the Screen:: Screen object
 *
 */
Screen::~Screen()
{
    if (_ScrSer != nullptr)
    {
        delete _ScrSer;
    }
}

/**
 * @brief 发送握手信号，检查屏幕是否良好
 *
 * @return true
 * @return false
 */
bool Screen::IsOK()
{
    uint8_t buf = 0;
    /* 要发送的帧 */
    std::vector<unsigned char> msg = { 0xAA, 0x00, 0xcc, 0x33, 0xc3, 0x3c };

    /* 应当回应的帧 */
    std::vector<unsigned char> rsp =
    { 0xAA, 0x00, 0x4F, 0x4B, 0xCC, 0x33, 0xC3, 0x3C };
    int c;
    int i = 0;

    /* 发送握手信号 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }

    /* 等待回应 */
    #if 0
    while ((c = _ScrSer->receive()) != -1)
    {
        if (c == rsp[i])
        {
            i++;
        }
        else
        {
            /* 握手失败 */
            return false;
        }
    }
    #endif

    while ((c = _ScrSer->Read(&buf, sizeof(buf), 1000)) != -1)
    {
        if (buf == rsp[i])
        {
            i++;
        }
        else
        {
            /* 握手失败 */
            return false;
        }
    }

    if (i == 8)
    {
        /* 握手成功 */
        return true;
    }

    /* 握手失败 */
    return false;
}

/**
 * @brief 设置屏幕的背光强度
 * @note 设置为 0x01,0x1F,屏幕会闪烁，不推荐
 * @param value 要设置的屏幕亮度(0x00-0xFF)
 */
void Screen::SetBGBrightness(int value)
{
    std::vector<unsigned char> msg = { 0xAA, 0x30, 0x80, 0xCC, 0x33, 0xC3, 0x3C };

    if ((value < 0x00) || (value > 0XFF))
    {
        /* 参数错误，直接返回 */
        return;
    }

    /* 组帧 */
    msg[2] = value;

    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }
}

/**
 * @brief 写数据寄存器
 *
 * @param type          若为 0 则为，SRAM,掉电数据消失，为 1 ，则写 Flash
 * @param address       写数据存储器地址，SRAM(0x00-0x7FFF) Flash(0x00-0x3FFF)
 * @return true         写成功
 * @return false        写失败
 * @note                写 SRAM 时没有回应
 */
bool Screen::WriteDataRegister(int type, int16_t address,
                               std::vector<unsigned char>& data)
{
    int _type = 0;
    int c = 0;
    int count = 0;
    uint8_t buf = 0;
    std::vector<unsigned char> msg = { 0xAA, 0x31 };

    /* 期待的回应帧 */
    std::vector<unsigned char> rsp =
    { 0xAA, 0x31, 0xA5, 0x4F, 0x4B, 0xCC, 0x33, 0xC3, 0x3C };

    /* 检查类型有效性 */
    if ((type != 0) && (type != 1))
    {
        type = 0;
    }

    /* 检查地址有效性 */
    if (type == 0) /* SRAM */
    {
        if ((address < 0x0000) || (address > 0x7FFF))
        {
            return false;
        }
        _type = 0x5A;
    }
    else if (type == 1) /* Flash */
    {
        if ((address < 0x0000) || (address > 0x3FFF))
        {
            return false;
        }
        _type = 0xA5;
    }

    /* 组帧 */
    msg.push_back(_type);
    msg.push_back((address & 0xFF00) >> 8);
    msg.push_back((address & 0xff));

    for (int i = 0; i < data.size(); i++)
    {
        msg.push_back(data[i]);
    }

    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);

    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }

    /* 写 SRAM 时，没有回应 */
    if (type == 0)
    {
        return true;
    }

    /* 等待 Flash 回应 */
    #if 0
    while ((c = _ScrSer->receive()) != -1)
    {
        if (c == rsp[count])
        {
            count++;
        }
        else
        {
            /* 写入失败 */
            return false;
        }
    }
    #endif

    while ((c = _ScrSer->Read(&buf, sizeof(buf), 1000)) != -1)
    {
        if (buf == rsp[count])
        {
            count++;
        }
        else
        {
            /* 握手失败 */
            return false;
        }
    }

    if (count == 9)
    {
        /* 写入成功 */
        return true;
    }

    /* 写入失败 */
    return false;
}

/**
 * @brief           读数据寄存器
 *
 * @param type      读存储器选择，0x5A = 32 kB SRAM, 0xA5 = 16 KB Flash
 * @param address   读数据寄存器地址，(0x0000 - 0x7FFF / 0x0000 - 0x3FFF)
 * @param length    读取的数据字节长度，(0x01-0xF0)
 * @param data      读取到的数据，
 * @return true     参数正确
 * @return false    参数错误
 */
bool Screen::ReadDataRegister(int type, int16_t address, int length,
                              std::vector<unsigned char>& data)
{
    int _type = 0;
    int c = 0;
    int _length = 0;
    uint8_t buf = 0;
    /* 发送帧 */
    std::vector<unsigned char> msg = { 0xAA, 0x32 };

    /* 回应帧 */
    std::vector<unsigned char> rsp = {};

    /* 检验数据的正确性 */
    if ((type != 0) && (type != 1))
    {
        _type = 0;
    }
    _type = type;

    if (_type == 0) /* SRAM */
    {
        if ((address < 0x0000) || (address > 0x7FFF))
        {
            return false;
        }
        msg.push_back(0x5A);
    }
    else if (_type == 1) /* Flash */
    {
        if ((address < 0x0000) || (address > 0x3FFF))
        {
            return false;
        }
        msg.push_back(0xA5);
    }


    if (length > 0xF0)
    {
        _length = 0xF0;
    }
    _length = length;

    /* 组帧 */

    msg.push_back((address & 0xFF00) >> 8);
    msg.push_back(address & 0xFF);
    msg.push_back(_length);

    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);

    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }

    /* 接收回应帧 */
    #if 0
    while ((c = _ScrSer->receive()) != -1)
    {
        rsp.push_back(c);
    }

    #endif

    while ((c = _ScrSer->Read(&buf, sizeof(buf), 1000)) != -1)
    {
        rsp.push_back(buf);
    }

    /* 校验接收帧的正确性 */
    if (rsp.size() != (length + 10))
    {
        data.resize(0);
        return false;
    }

    data.resize(0);

    for (int i = 0; i < rsp[5]; i++)
    {
        data.push_back(rsp[i + 6]);
    }

    return true;
}

/**
 * @brief           把 32KB SRAM 数据存储器的内容写入指定的图片存储器空间
 * @param picid     图片存储器空间位置,0x00-0x0F,每个空间 32KB
 * @return true     成功执行
 * @return false    执行失败
 */
bool Screen::WritePicRegister(int picid)
{
    int c = 0;
    int count = 0;
    uint8_t buf = 0;

    /* send frame  */
    std::vector<unsigned char> msg = { 0xAA, 0x33, 0x5A, 0xA5 };

    /* 期待的回应帧 */
    std::vector<unsigned char> rsp =
    { 0xAA, 0x33, 0x4F, 0x4B, 0xCC, 0x33, 0xC3, 0x3C };

    /* 检验参数有效性 */
    if ((picid < 0x00) || (picid > 0x0f))
    {
        return false;
    }

    /* 组帧 */
    msg.push_back(picid);
    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);


    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }

    /* wait for response  */
    #if 0
    while ((c = _ScrSer->receive()) != -1)
    {
        if (c == rsp[count])
        {
            count++;
        }
        else
        {
            /* write failed */
            return false;
        }
    }
    #endif

    while ((c = _ScrSer->Read(&buf, sizeof(buf), 1000)) != -1)
    {
        if (buf == rsp[count])
        {
            count++;
        }
        else
        {
            /* write failed */
            return false;
        }
    }

    if (count == 8)
    {
        /* write success */
        return true;
    }

    /* write failed */
    return false;
}

/**
 * @brief 清屏
 *
 * @param color     清屏的颜色值
 * @note            16bit 颜色,5R6G5B 模式
 */
void Screen::ClearScreen(int16_t color)
{
    std::vector<unsigned char> msg = { 0xAA, 0x01 };

    msg.push_back((color & 0xff00) >> 8);
    msg.push_back(color & 0xff);

    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);


    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }
}

/**
 * @brief 绘制点
 *
 * @param color     点的颜色
 * @param xres      实际像素点 X 方向像素大小,0x01-0x0F。
 * @param yres      实际像素点 Y 方向像素大小,0x01-0x0F。
 * @param point     要绘制的点
 * @note            16bit 颜色,5R6G5B 模式
 */
void Screen::DrawPoint(int16_t             color,
                       unsigned char       xres,
                       unsigned char       yres,
                       std::vector<Point>& point)
{
    if (point.size() == 0)
    {
        return;
    }

    /* 要发送的帧 */
    std::vector<unsigned char> msg = { 0xAA, 0x02 };

    msg.push_back((color & 0xff00) >> 8);
    msg.push_back(color & 0xff);
    msg.push_back(xres);
    msg.push_back(yres);

    for (int i = 0; i < point.size(); i++)
    {
        msg.push_back((point[i]._x & 0xff00) >> 8);
        msg.push_back(point[i]._x & 0xff);
        msg.push_back((point[i]._y & 0xff00) >> 8);
        msg.push_back(point[i]._y & 0xff);
    }

    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);


    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }
}

/**
 * @brief 绘制线
 *
 * @param color     线的颜色
 * @param point     绘制线需要的点
 */
void Screen::DrawLine(int16_t color, std::vector<Point>& point)
{
    if (point.size() < 2)
    {
        return;
    }
    std::vector<unsigned char> msg = { 0xAA, 0x03 };

    msg.push_back((color & 0xff00) >> 8);
    msg.push_back(color & 0xff);

    for (int i = 0; i < point.size(); i++)
    {
        msg.push_back((point[i]._x & 0xff00) >> 8);
        msg.push_back(point[i]._x & 0xff);
        msg.push_back((point[i]._y & 0xff00) >> 8);
        msg.push_back(point[i]._y & 0xff);
    }

    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);

    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }
}

// TODO:经测试 02 mode 貌似没有什么用

/**
 * @brief 绘制一个矩形
 *
 * @param mode      矩形模式，0x00 - 颜色显示矩形框，0x01 - 颜色填充矩形区域，0x02 - XOR 矩形区域
 * @param color     颜色
 * @param point     矩形的左上角和右下角点
 */
void Screen::DrawRectangle(int mode, int16_t color, std::vector<Point>& point)
{
    int _mode = 0;

    /* 判断数据有效性 */
    if ((mode < 0x00) || (mode > 0x02))
    {
        _mode = 0;
    }

    if (point.size() < 2)
    {
        return;
    }

    /* 要发送的信息  */
    std::vector<unsigned char> msg = { 0xAA, 0x05 };
    msg.push_back(_mode);
    msg.push_back((color & 0xff00) >> 8);
    msg.push_back(color & 0xff);

    for (int i = 0; i < 2; i++)
    {
        msg.push_back((point[i]._x & 0xff00) >> 8);
        msg.push_back(point[i]._x & 0xff);
        msg.push_back((point[i]._y & 0xff00) >> 8);
        msg.push_back(point[i]._y & 0xff);
    }

    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);

    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }
}

void Screen::MoveScreen(int mode, Direction  dir, int16_t  color, int16_t dist,
                        std::vector<Point>point)
{
    std::vector<unsigned char> msg = { 0xAA, 0x09 };

    /* 检查参数有效性，并设置 _mode */
    int _mode = 0;
    int _dir = 0;

    if ((mode != 0) && (mode != 1))
    {
        _mode = 0;
    }

    if (point.size() < 2)
    {
        return;
    }

    if (mode == 0)
    {
        _mode = 0b00000000;
    }
    else
    {
        _mode = 0b10000000;
    }

    if ((dir < 0x00) || (dir > 0x03))
    {
        _dir = 0;
    }
    _dir = dir;
    _mode |= _dir;

    msg.push_back(_mode);
    msg.push_back((dist) & 0xFF00 >> 8);
    msg.push_back(dist & 0XFF);
    msg.push_back((color & 0xFF00) >> 8);
    msg.push_back(color & 0xFF);

    for (int i = 0; i < point.size(); i++)
    {
        msg.push_back((point[i]._x & 0xff00) >> 8);
        msg.push_back(point[i]._x & 0xff);
        msg.push_back((point[i]._y & 0xff00) >> 8);
        msg.push_back(point[i]._y & 0xff);
    }

    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);

    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }
}

/**
 * @brief 在屏幕显示字符，支持，asciii 和 GB2312 编码
 *
 * @param adj       字符宽度是否调整
 * @param visual    背景色是否显示
 * @param size      字号大小
 * @param color     前景色
 * @param bcolor    背景色
 * @param str       要显示的字符
 */
void Screen::DisplayString(int adj, int visual, int size,
                           int16_t color, int16_t bcolor,
                           Point& point,
                           std::vector<unsigned char>& str)
{
    int _mode = 0;
    int _visual = 0;
    int _size = 0;
    std::vector<unsigned char> msg = { 0xAA, 0x11 };

    if ((adj != 0) && (adj != 1))
    {
        _mode = 0;
    }


    if ((visual != 0) && (visual != 1))
    {
        _visual = 0;
    }
    _visual = visual;

    if ((size < 0x00) || (size > 0x09))
    {
        _size = 0;
    }
    _size = size;

    if (_mode == 1)
    {
        _mode |= 0b10000000;
    }

    if (_visual == 1)
    {
        _mode |= 0b01000000;
    }

    _mode |= _size & 0b00001111;
    msg.push_back(_mode);
    msg.push_back((color & 0xFF00) >> 8);
    msg.push_back(color & 0xFF);
    msg.push_back((bcolor & 0xFF00) >> 8);
    msg.push_back(bcolor & 0xFF);

    msg.push_back((point._x & 0xff00) >> 8);
    msg.push_back(point._x & 0xff);
    msg.push_back((point._y & 0xff00) >> 8);
    msg.push_back(point._y & 0xff);

    for (int i = 0; i < str.size(); i++)
    {
        msg.push_back(str[i]);
    }

    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);

    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }
}

/**
 * @brief 显示数据变量
 *
 * @param visual    背景色是否显示
 * @param signal    是否为有符号数 0 - 无符号数 1 - 有符号数
 * @param disyzero  是否显示无效 0
 * @param zerostyle 无效 0 的显示方式，0 - 无效，1-空格
 * @param size      字号大小
 * @param color     前景色
 * @param bcolor    后景色
 * @param integer   整数位数
 * @param fl        小数位数
 * @param point     字符显示位置
 * @param var       变量值
 */
void Screen::DisplayVariable(int     visual,
                             int     signal,
                             int     disyzero,
                             int     zerostyle,
                             int     size,
                             int16_t color,
                             int16_t bcolor,
                             int     integer,
                             int     fl,
                             Point & point,
                             int     var)
{
    std::vector<unsigned char> msg = { 0xaa, 0x14 };

    int _mode = 0;
    int _visual = 0;
    int _signal = 0;
    int _disyzero = 0;
    int _zerostyle = 0;
    int _size = 0;

    if ((visual != 0) && (visual != 1))
    {
        _visual = 0;
    }
    _visual = visual;

    if ((signal != 0) && (signal != 1))
    {
        _signal = 0;
    }
    _signal = signal;

    if ((disyzero != 0) && (disyzero != 1))
    {
        _disyzero = 0;
    }
    _disyzero = disyzero;

    if ((zerostyle != 0) && (zerostyle != 1))
    {
        _zerostyle = 0;
    }
    _zerostyle = zerostyle;

    if ((size < 0x00) || (size > 0x09))
    {
        _size = 0;
    }
    _size = size;

    if (_visual == 1)
    {
        _mode |= 0b10000000;
    }

    if (_signal == 1)
    {
        _mode |= 0b01000000;
    }

    if (_disyzero == 1)
    {
        _mode |= 0b00100000;
    }

    if (_zerostyle == 1)
    {
        _mode |= 0b00010000;
    }

    _mode |= _size & 0b00000111;

    msg.push_back(_mode);

    msg.push_back((color & 0xFF00) >> 8);
    msg.push_back(color & 0xFF);
    msg.push_back((bcolor & 0xFF00) >> 8);
    msg.push_back(bcolor & 0xFF);

    msg.push_back(integer);
    msg.push_back(fl);

    msg.push_back((point._x & 0xff00) >> 8);
    msg.push_back(point._x & 0xff);
    msg.push_back((point._y & 0xff00) >> 8);
    msg.push_back(point._y & 0xff);

    msg.push_back((var & 0xFF000000) >> 24);
    msg.push_back((var & 0xFF0000) >> 16);
    msg.push_back((var & 0xFF00) >> 8);
    msg.push_back((var & 0xFF));


    msg.push_back(0xCC);
    msg.push_back(0x33);
    msg.push_back(0xC3);
    msg.push_back(0x3C);

    /* 发送帧 */
    for (int i = 0; i < msg.size(); i++)
    {
        //_ScrSer->transmit((const char *)&msg[i]);
        _ScrSer->Write((const uint8_t *)&msg[i], 1);
    }
}
