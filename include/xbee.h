#ifndef __XBEE_H__
#define __XBEE_H__

#include <sys/time.h>
#include "xbee_response.h"
#include "xbee_request.h"

#include "uart.h"

/**
 * 此类提供了通过串行端口使用XBee无线电发送和接收数据包的方法。
 * XBee无线电必须配置为API（数据包）模式（AP = 2）
 *
 * 此代码旨在只有一个线程的微控制器上运行，因此您有责任及时从串行缓冲区读取数据。
 * 这涉及到调用readPacket(…)的变体。
 * 如果您的串口接收数据的速度比您读取的速度快，您可能会丢失数据包。
 * Arduino只有一个128字节的串行缓冲区，
 * 因此如果两个或更多的数据包到达而不调用readPacket(…)，它很容易溢出。
 *
 * 为了节省资源，这个类一次只支持在内存中存储一个响应包。
 * 这意味着您必须在调用readPacket(…)之前完全使用该包，因为调用readPacket(…)会覆盖前面的响应。
 *
 * 该类创建一个大小为MAX_FRAME_DATA_SIZE的数组，用于存储响应包。
 * 您可能需要调整此值以保存内存。
 */
class XBee {
public:

    XBee();

    /**
     * 读取所有可用的串行字节，直到数据包被解析，或发生错误或缓冲区为空。
     * 此方法始终快速返回，因为它不会等待串行数据是否到达。
     * 如果您要在循环中及时执行其他操作，则需要使用此方法
     * 延迟可能引发问题。
     * 注意：调用此方法会重置当前响应，因此请确保您先使用了当前响应
     */
    int          readPacket();

    /**
     * 在超时之前最多等待timeout毫秒以获取响应数据包；如果读取数据包，则返回true。
     * 如果发生超时或错误，则返回false。
     */
    bool          readPacket(int timeout);

    /**
     * 读取，直到收到数据包或出现错误。
     * 注意:如果没有响应，Arduino代码就会挂在上面，所以要小心使用
     * 一直会调用！ 通常最好使用超时：readPacket（int）
     */
    void          readPacketUntilAvailable();

    /**
     *
     */
    void          getResponse(XBeeResponse& response);

    /**
     * 返回对当前响应的引用
     * 注意：一旦再次调用readPacket，此响应将被覆盖！
     */
    XBeeResponse& getResponse();

    /**
     * 从串行端口发送XBeeRequest（TX数据包）
     */
    void          send(XBeeRequest& request);

    /**
     * 返回1到255之间的顺序frame ID ？？？？？
     */
    uint8_t       getNextFrameId();

    /**
     * 指定串行端口。仅与支持多个串行端口（例如Mega）的Arduino相关
     */
    void          setSerial(uart* serial);

    /**
     * (源码解释)
     * 返回自Arduino开发板开始运行当前程序以来经过的毫秒数。大约50天后，该数字将溢出（返回零）。
     */
    unsigned long millis()
    {
        timeval start;

        gettimeofday(&start, NULL);
        return start.tv_sec;
    }

private:

    int    xbee_read();
    void    write(uint8_t val);
    void    sendByte(uint8_t b,
                     bool    escape);

    // 复位响应：丢弃先前的数据包并重新开始
    void resetResponse();

    XBeeResponse _response;
    bool _escape;           // 在APAT_2模式中的一个状态位
    uint8_t _pos;           // 响应的当前数据包位置。只是用于数据包解析的状态变量，与响应无关
    uint8_t _b;              // 最后读取的字节
    uint8_t _checksumTotal; // 校验和总计
    uint8_t _nextFrameId;   // ？？？
    // 缓冲区，用于接收RX数据包。仅保存api特定的帧数据，从api id字节之后到校验和之前
    uint8_t _responseFrameData[MAX_FRAME_DATA_SIZE];

    uart *_serial; // uart为自定义串口库中的串口类
};                 // << XBee end >>

#endif // ifndef __XBEE_H__
