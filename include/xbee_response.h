#ifndef __XBEE_RESPONSE_H__
#define __XBEE_RESPONSE_H__

#include <stdint.h>
#include "xbee_address.h"
#include "constants.h"

/**
 * 此值确定用于接收RX数据包的字节数组的大小
 * 如果RX数据包超出此大小，则无法解析！
 * 此值由1系列无线电的最大数据包大小（100字节有效负载+ 64位地址+选项字节和rssi字节）确定
 */
#define MAX_FRAME_DATA_SIZE 110

// 广播地址
#define BROADCAST_ADDRESS 0xffff
#define ZB_BROADCAST_ADDRESS 0xfffe

// 错误代码
#define NO_ERROR 0
#define CHECKSUM_FAILURE 1
#define PACKET_EXCEEDS_BYTE_ARRAY_LENGTH 2
#define UNEXPECTED_START_BYTE 3

// 帧数据的不变长度（不包括帧ID或api ID或可变数据大小（例如有效载荷，处于命令设置值处

/**
 * 所有xbee响应的基类（Rx数据包）
 *
 */
class XBeeResponse {
public:

    /**
     * 默认构造函数
     */
    XBeeResponse();

    /**
     * 返回响应的Api ID(Frame ID)
     */
    uint8_t  getApiId();
    void     setApiId(uint8_t apiId);

    /**
     * 返回数据包的MSB长度
     */
    uint8_t  getMsbLength();
    void     setMsbLength(uint8_t msbLength);

    /**
     * 返回数据包的LSB长度
     */
    uint8_t  getLsbLength();
    void     setLsbLength(uint8_t lsbLength);

    /**
     * 返回数据包校验和
     */
    uint8_t  getChecksum();
    void     setChecksum(uint8_t checksum);

    /**
     * 返回帧数据的长度：api id之后和校验和之前的所有字节
     * 请注意直到版本0.1.2，这是错误地包括长度的校验和。
     */
    uint8_t  getFrameDataLength();
    void     setFrameData(uint8_t *frameDataPtr);

    /**
     * 返回包含响应的缓冲区。
     * 以跟在API ID之后的字节开头，包括校验和之前的所有字节
     * 长度由getFrameDataLength（）指定
     * 注意：与Digi对帧数据的定义不同，它不是以API ID开头。
     * 这样做的原因是所有响应都包含一个API ID，而我的框架数据仅包含特定于API的数据。
     */
    uint8_t* getFrameData();

    void     setFrameLength(uint8_t frameLength);

    // 支持未来的65535字节数据包

    /**
     * 返回数据包的长度
     */
    uint16_t getPacketLength();

    /**
     * 将响应重置为默认值
     */
    void     reset();

    /**
     * 初始化响应
     */
    void     init();
#ifdef SERIES_2

    /**
     * 只有在getApiId() == ZB_TX_STATUS_RESPONSE(0x8b)来填充响应时，才调用ZBTxStatusResponse类的实例
     */
    void getZBTxStatusResponse(XBeeResponse& response);

    /**
     * 只有当getApiId() == ZB_RX_RESPONSE(0x90)来填充响应时，才使用ZBRxResponse类的实例进行调用
     */
    void getZBRxResponse(XBeeResponse& response);

    /**
     * 只有当getApiId() == ZB_EXPLICIT_RX_RESPONSE(0x91)来填充响应时，才调用ZBExplicitRxResponse类的实例
     */
    void getZBExplicitRxResponse(XBeeResponse& response);

    /**
     * 只有当getApiId() == ZB_IO_SAMPLE_RESPONSE(0x92)来填充响应时，才使用ZBRxIoSampleResponse类的实例调用
     */
    void getZBRxIoSampleResponse(XBeeResponse& response);
#endif // ifdef SERIES_2
#ifdef SERIES_1

    /**
     * 仅在getApiId（）== TX_STATUS_RESPONSE时使用TxStatusResponse实例进行调用
     */
    void getTxStatusResponse(XBeeResponse& response);

    /**
     * 仅当getApiId（）== RX_16_RESPONSE时才使用Rx16Response实例进行调用
     */
    void getRx16Response(XBeeResponse& response);

    /**
     * 仅当getApiId（）== RX_64_RESPONSE时才使用Rx64Response实例进行调用
     */
    void getRx64Response(XBeeResponse& response);

    /**
     * 仅当getApiId（）== RX_16_IO_RESPONSE时，才使用Rx16IoSampleResponse实例进行调用
     */
    void getRx16IoSampleResponse(XBeeResponse& response);

    /**
     * 仅在getApiId（）== RX_64_IO_RESPONSE时使用Rx64IoSampleResponse实例进行调用
     */
    void getRx64IoSampleResponse(XBeeResponse& response);
#endif // ifdef SERIES_1

    /**
     * 只有在getApiId() == AT_COMMAND_RESPONSE时才使用AtCommandResponse实例调用
     */
    void    getAtCommandResponse(XBeeResponse& responses);

    /**
     * 只有在getApiId() ==
     * REMOTE_AT_COMMAND_RESPONSE时才使用RemoteAtCommandResponse实例调用
     */
    void    getRemoteAtCommandResponse(XBeeResponse& response);

    /**
     * 只有在getApiId() == MODEM_STATUS_RESPONSE时才调用ModemStatusResponse实例
     */
    void    getModemStatusResponse(XBeeResponse& response);

    /**
     * 如果响应已成功解析，并且已完成，可以使用，则返回true
     */
    bool    isAvailable();
    void    setAvailable(bool complete);

    /**
     * 如果响应包含错误，则返回true
     */
    bool    isError();

    /**
     * 返回错误代码，如果成功则返回零。
     * 错误代码包括:CHECKSUM_FAILURE, packet_s_byte_array_length, ted_start_byte
     */
    uint8_t getErrorCode();
    void    setErrorCode(uint8_t errorCode);

protected:

    // frameData指针
    uint8_t *_frameDataPtr;

private:

    void setCommon(XBeeResponse& target);
    uint8_t _apiId;       // 响应的api的id值(frame id)
    uint8_t _msbLength;   // 数据包的高字节长度
    uint8_t _lsbLength;   // 数据包的低字节长度
    uint8_t _checksum;    // 数据包的校验和
    uint8_t _frameLength; // 帧数据的长度：api id(包含)之后和校验和之前的所有字节
    bool _complete;       // 如果响应已成功解析并且完整且可以使用，则返回true
    uint8_t _errorCode;   // 错误代码(包括：CHECKSUM_FAILURE，PACKET_EXCEEDS_BYTE_ARRAY_LENGTH，UNEXPECTED_START_BYTE)
};                        // << XBeeResponse end >>

/****************以下为 XBeeResponse 的子类******************************/

/**+++++++++++++++++FrameIdResponse++++++++++++++++++++**/

/**
 *这个类是由包括frame ID的所有响应扩展
 */
class FrameIdResponse : public XBeeResponse {
public:

    FrameIdResponse();
    uint8_t getFrameId();

private:

    uint8_t _frameId;
};

#ifdef SERIES_2

/**
 * series 2系列TX状态数据包
 */
class ZBTxStatusResponse : public FrameIdResponse {
public:

    ZBTxStatusResponse();
    uint16_t getRemoteAddress();
    uint8_t  getTxRetryCount();
    uint8_t  getDeliveryStatus();
    uint8_t  getDiscoveryStatus();
    bool     isSuccess();

    static const uint8_t API_ID = ZB_TX_STATUS_RESPONSE;
};
#endif // ifdef SERIES_2

#ifdef SERIES_1

/**
 * 表示serial 1 TX状态数据包
 */
class TxStatusResponse : public FrameIdResponse {
public:

    TxStatusResponse();
    uint8_t getStatus();
    bool    isSuccess();

    static const uint8_t API_ID = TX_STATUS_RESPONSE;
};
#endif // ifdef SERIES_1

/**
 * 表示一个AT Command RX数据包
 */
class AtCommandResponse : public FrameIdResponse {
public:

    AtCommandResponse();

    /**
     * 返回包含两个字符的命令的数组
     */
    uint8_t* getCommand();

    /**
     * 返回命令状态码。
     * 零表示成功的命令
     */
    uint8_t  getStatus();

    /**
     * 返回包含命令值的数组。
     * 这仅适用于查询命令。
     */
    uint8_t* getValue();

    /**
     * 返回命令值数组的长度。
     */
    uint8_t  getValueLength();

    /**
     * 如果状态等于AT_OK，则返回true
     */
    bool     isOk();

    static const uint8_t API_ID = AT_COMMAND_RESPONSE;
};

/**
 * 表示远程AT命令RX数据包
 */
class RemoteAtCommandResponse : public AtCommandResponse {
public:

    RemoteAtCommandResponse();

    /**
     * 返回包含两个字符的命令的数组
     */
    uint8_t      * getCommand();

    /**
     * 返回命令状态码。
     * 零表示成功的命令
     */
    uint8_t        getStatus();

    /**
     * 返回包含命令值的数组。
     * 这仅适用于查询命令。
     */
    uint8_t      * getValue();

    /**
     * 返回命令值数组的长度。
     */
    uint8_t        getValueLength();

    /**
     * 返回远程无线电的16位地址
     */
    uint16_t       getRemoteAddress16();

    /**
     * 返回远程无线电的64位地址
     */
    XBeeAddress64& getRemoteAddress64();

    /**
     * 如果命令成功，则返回true
     */
    bool           isOk();

    static const uint8_t API_ID = REMOTE_AT_COMMAND_RESPONSE;

private:

    XBeeAddress64 _remoteAddress64;
};

/**+++++++++++++++++RxDataResponse++++++++++++++++++++**/

/**
 * Series 1和2数据RX数据包的通用功能
 */
class RxDataResponse : public XBeeResponse {
public:

    RxDataResponse();

    /**
     * 返回有效负载的指定索引。 索引可以是0到getDataLength（）-1
     * 不推荐使用此方法； 使用uint8_t * getData（）
     */
    uint8_t         getData(int index);

    /**
     * 返回有效载荷数组。 可以从索引0到getDataLength（）-1进行访问
     */
    uint8_t       * getData();

    /**
     * 返回有效载荷的长度
     */
    virtual uint8_t getDataLength() = 0;

    /**
     * 返回帧数据中数据开始的位置
     */
    virtual uint8_t getDataOffset() = 0;
};

#ifdef SERIES_2

/**
 * 表示2系列RX数据包
 */
class ZBRxResponse : public RxDataResponse {
public:

    ZBRxResponse();
    XBeeAddress64& getRemoteAddress64();
    uint16_t       getRemoteAddress16();
    uint8_t        getOption90();
    uint8_t        getOption91();
    uint8_t        getDataLength();

    // 数据开始的帧位置
    uint8_t        getDataOffset();

    static const uint8_t API_ID = ZB_RX_RESPONSE;

private:

    XBeeAddress64 _remoteAddress64;
};

/**
 * 表示2系列显式RX数据包
 *
 * 注意：要接收这些响应，请设置AO = 1。 在默认的情况下AO = 0，
 * 您将收到ZBRxResponses，但不知道确切的详细信息。
 */
class ZBExplicitRxResponse : public ZBRxResponse {
public:

    ZBExplicitRxResponse();
    uint8_t  getSrcEndpoint();
    uint8_t  getDstEndpoint();
    uint16_t getClusterId();
    uint16_t getProfileId();
    uint8_t  getOption();
    uint8_t  getDataLength();

    // 数据开始的帧位置
    uint8_t  getDataOffset();

    static const uint8_t API_ID = ZB_EXPLICIT_RX_RESPONSE;
};

/**
 * 表示2系列RX I / O样本数据包
 */
class ZBRxIoSampleResponse : public ZBRxResponse {
public:

    ZBRxIoSampleResponse();
    bool     containsAnalog();
    bool     containsDigital();

    /**
     * 如果启用了引脚，则返回true
     */
    bool     isAnalogEnabled(uint8_t pin);

    /**
     * 如果启用了引脚，则返回true
     */
    bool     isDigitalEnabled(uint8_t pin);

    /**
     * 返回指定引脚的10位模拟读数。
     * 有效引脚包括ADC：xxx。
     */
    uint16_t getAnalog(uint8_t pin);

    /**
     * 如果指定的引脚为高/高，则返回true。
     * 有效的引脚包括DIO：xxx。
     */
    bool     isDigitalOn(uint8_t pin);
    uint8_t  getDigitalMaskMsb();
    uint8_t  getDigitalMaskLsb();
    uint8_t  getAnalogMask();

    static const uint8_t API_ID = ZB_IO_SAMPLE_RESPONSE;
};
#endif // ifdef SERIES_2

#ifdef SERIES_1

/**
 * 表示Series 1 RX数据包
 */
class RxResponse : public RxDataResponse {
public:

    RxResponse();

    // 记住RSSI是负数，但是这是无符号字节，所以它由你来转换
    uint8_t         getRssi();
    uint8_t         getOption();
    bool            isAddressBroadcast();
    bool            isPanBroadcast();
    uint8_t         getDataLength();
    uint8_t         getDataOffset();
    virtual uint8_t getRssiOffset() = 0;
};

/**
 * 表示Series 1 16位地址RX数据包
 */
class Rx16Response : public RxResponse {
public:

    Rx16Response();
    uint8_t  getRssiOffset();
    uint16_t getRemoteAddress16();

    static const uint8_t API_ID = RX_16_RESPONSE;

protected:

    uint16_t _remoteAddress;
};

/**
 * 表示Series 1 64位地址RX数据包
 */
class Rx64Response : public RxResponse {
public:

    Rx64Response();
    uint8_t        getRssiOffset();
    XBeeAddress64& getRemoteAddress64();

    static const uint8_t API_ID = RX_64_RESPONSE;

private:

    XBeeAddress64 _remoteAddress;
};

/**
 * 表示1系列RX I / O样本数据包
 */
class RxIoSampleBaseResponse : public RxResponse {
public:

    RxIoSampleBaseResponse();

    /**
     * Returns the number of samples in this packet
     */
    uint8_t  getSampleSize();
    bool     containsAnalog();
    bool     containsDigital();

    /**
     * Returns true if the specified analog pin is enabled
     */
    bool     isAnalogEnabled(uint8_t pin);

    /**
     * Returns true if the specified digital pin is enabled
     */
    bool     isDigitalEnabled(uint8_t pin);

    /**
     * Returns the 10-bit analog reading of the specified pin.
     * Valid pins include ADC:0-5.  Sample index starts at 0
     */
    uint16_t getAnalog(uint8_t pin,
                       uint8_t sample);

    /**
     * Returns true if the specified pin is high/on.
     * Valid pins include DIO:0-8.  Sample index starts at 0
     */
    bool    isDigitalOn(uint8_t pin,
                        uint8_t sample);
    uint8_t getSampleOffset();

    /**
     * Gets the offset of the start of the given sample.
     */
    uint8_t getSampleStart(uint8_t sample);

private:
};
#endif // ifdef SERIES_1

/**
 * 代表调制解调器状态RX数据包
 */
class ModemStatusResponse : public XBeeResponse {
public:

    ModemStatusResponse();
    uint8_t getStatus();

    static const uint8_t API_ID = MODEM_STATUS_RESPONSE;
};

#endif // ifndef __XBEE_RESPONSE_H__
