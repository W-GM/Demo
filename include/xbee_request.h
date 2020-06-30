#ifndef __XBEE_REQUEST_H__
#define __XBEE_REQUEST_H__

#include <stdint.h>
#include "xbee_address.h"
#include "constants.h"

/**
 * 所有XBee请求的基类(TX包)
 * 建议重用该类的子类来保存内存
 * 该类为其分配一个缓冲区
 */
class XBeeRequest {
public:

    /**
     * 构造函数
     * TODO 使受保护
     */
    XBeeRequest(uint8_t apiId,
                uint8_t frameId);

    /**
     * 设置frame id。必须在1到255之间(包括255)才能获得TX状态响应。
     */
    void    setFrameId(uint8_t frameId);

    /**
     * 返回FrameId
     */
    uint8_t getFrameId();

    /**
     * 返回 API id
     */
    uint8_t getApiId();

    // 设置其 "= 0" 使它成为一个纯虚函数，这意味着子类必须实现

    /**
     * frame ID之后开始（pos = 0），直到校验和（不包括）之前结束
     * 注意：与Digi对帧数据的定义不同，它不是以API ID开头。
     * 原因是API ID和Frame ID对所有请求都是通用的，而我对框架数据的定义只是API特定的数据。
     */
    virtual uint8_t getFrameData(uint8_t pos) = 0;

    /**
     * 返回API frame的大小（不包括FrameID或API ID或校验和）。
     */
    virtual uint8_t getFrameDataLength() = 0;

    // void reset();

protected:

    void setApiId(uint8_t apiId);

private:

    uint8_t _apiId;
    uint8_t _frameId;
}; // << XBeeRequest end >>

/**
 * 所有支持有效负载的TX数据包都扩展了此类
 */
class PayloadRequest : public XBeeRequest {
public:

    PayloadRequest(uint8_t  apiId,
                   uint8_t  frameId,
                   uint8_t *payload,
                   uint8_t  payloadLength);

    /**
     * 如果不为null，返回数据包的有效载荷
     */
    uint8_t* getPayload();

    /**
     * 设置有效载荷数组
     */
    void     setPayload(uint8_t *payloadPtr);

    /*
     * 一次调用即可设置有效载荷及其长度。
     */
    void     setPayload(uint8_t *payloadPtr, uint8_t payloadLength)
    {
        setPayload(payloadPtr);
        setPayloadLength(payloadLength);
    }

    /**
     * 返回用户指定的有效载荷数组的长度。
     */
    uint8_t getPayloadLength();

    /**
     * 设置要包含在请求中的有效负载的长度。
     * 例如，如果有效载荷数组为50个字节，而您只希望包中包含前10个，则将长度设置为10。
     * 长度必须小于等于数组长度。
     */
    void    setPayloadLength(uint8_t payloadLength);

private:

    uint8_t *_payloadPtr;
    uint8_t _payloadLength;
};

#ifdef SERIES_1

/**
 * 表示与Api ID相对应的Series 1 TX数据包：TX_16_REQUEST
 * <p/>
 * Be careful not to send a data array larger than the max packet size of your
 *radio.
 * This class does not perform any validation of packet size and there will be
 *no indication
 * if the packet is too large, other than you will not get a TX Status response.
 * The datasheet says 100 bytes is the maximum, although that could change in
 *future firmware.
 */
class Tx16Request : public PayloadRequest {
public:

    Tx16Request(uint16_t addr16,
                uint8_t  option,
                uint8_t *payload,
                uint8_t  payloadLength,
                uint8_t  frameId);

    /**
     * Creates a Unicast Tx16Request with the ACK option and DEFAULT_FRAME_ID
     */
    Tx16Request(uint16_t addr16,
                uint8_t *payload,
                uint8_t  payloadLength);

    /**
     * Creates a default instance of this class.  At a minimum you must specify
     * a payload, payload length and a destination address before sending this
     *request.
     */
    Tx16Request();
    uint16_t getAddress16();
    void     setAddress16(uint16_t addr16);
    uint8_t  getOption();
    void     setOption(uint8_t option);
    uint8_t  getFrameData(uint8_t pos);
    uint8_t  getFrameDataLength();

protected:

private:

    uint16_t _addr16;
    uint8_t _option;
};

/**
 * Represents a Series 1 TX packet that corresponds to Api Id: TX_64_REQUEST
 *
 * Be careful not to send a data array larger than the max packet size of your
 *radio.
 * This class does not perform any validation of packet size and there will be
 *no indication
 * if the packet is too large, other than you will not get a TX Status response.
 * The datasheet says 100 bytes is the maximum, although that could change in
 *future firmware.
 */
class Tx64Request : public PayloadRequest {
public:

    Tx64Request(XBeeAddress64& addr64,
                uint8_t        option,
                uint8_t       *payload,
                uint8_t        payloadLength,
                uint8_t        frameId);

    /**
     * Creates a unicast Tx64Request with the ACK option and DEFAULT_FRAME_ID
     */
    Tx64Request(XBeeAddress64& addr64,
                uint8_t       *payload,
                uint8_t        payloadLength);

    /**
     * Creates a default instance of this class.  At a minimum you must specify
     * a payload, payload length and a destination address before sending this
     *request.
     */
    Tx64Request();
    XBeeAddress64& getAddress64();
    void           setAddress64(XBeeAddress64& addr64);

    // TODO move option to superclass
    uint8_t        getOption();
    void           setOption(uint8_t option);
    uint8_t        getFrameData(uint8_t pos);
    uint8_t        getFrameDataLength();

private:

    XBeeAddress64 _addr64;
    uint8_t _option;
};
#endif // ifdef SERIES_1

/**
 * 表示与Api ID相对应的2系列TX数据包：ZB_TX_REQUEST
 *
 * 注意不要发送大于无线电最大数据包大小的数据数组。
 * 此类不会对数据包大小进行任何验证，并且不会指示该数据包是否太大，除非您不会收到TX Status响应。
 * 数据表说ZNet固件最大为72个字节，ZB Pro固件提供ATNP命令来获取最大支持的有效负载大小。
 * 此命令很有用，因为最大有效负载大小会根据某些设置（例如加密）而变化。
 * ZB Pro固件提供了PAYLOAD_TOO_LARGE，如果有效负载大小超过最大值，则将返回该值。
 */
class ZBTxRequest : public PayloadRequest {
public:

    /**
     * 使用ACK选项和DEFAULT_FRAME_ID创建单播ZBTxRequest
     */
    ZBTxRequest(const XBeeAddress64& addr64,
                uint8_t             *payload,
                uint8_t              payloadLength);
    ZBTxRequest(const XBeeAddress64& addr64,
                uint16_t             addr16,
                uint8_t              broadcastRadius,
                uint8_t              option,
                uint8_t             *payload,
                uint8_t              payloadLength,
                uint8_t              frameId);

    /**
     * 创建此类的默认实例。 发送此请求之前，您至少必须指定有效负载，有效负载长度和64位目标地址。
     */
    ZBTxRequest();
    XBeeAddress64& getAddress64();
    uint16_t       getAddress16();
    uint8_t        getBroadcastRadius();
    uint8_t        getOption();
    void           setAddress64(const XBeeAddress64& addr64);
    void           setAddress16(uint16_t addr16);
    void           setBroadcastRadius(uint8_t broadcastRadius);
    void           setOption(uint8_t option);

protected:

    // 声明虚函数
    uint8_t getFrameData(uint8_t pos);
    uint8_t getFrameDataLength();
    XBeeAddress64 _addr64;
    uint16_t _addr16;
    uint8_t _broadcastRadius;
    uint8_t _option;
};

/**
 * 表示与Api ID相对应的2系列TX数据包：ZB_EXPLICIT_TX_REQUEST
 *
 * 请参阅上面有关ZBTxRequest的最大数据包大小的警告，这可能也适用于此。
 *
 * 请注意，为了区分来自非XBee设备的答复数据包，
 * 将AO = 1设置为启用ZBExplicitRxResponse数据包的接收。
 */
class ZBExplicitTxRequest : public ZBTxRequest {
public:

    /**
     *使用ACK选项和DEFAULT_FRAME_ID创建单播ZBExplicitTxRequest。
     *
     * 它使用endpoints 232 and cluster 0x0011的Maxstream配置文件（0xc105），
     * 从而导致与普通ZBTxRequest发送的数据包相同。
     */
    ZBExplicitTxRequest(XBeeAddress64& addr64,
                        uint16_t       addr16,
                        uint8_t       *payload,
                        uint8_t        payloadLength);

    /**
     * 创建一个ZBExplicitTxRequest，指定所有字段。
     */
    ZBExplicitTxRequest(XBeeAddress64& addr64,
                        uint16_t       addr16,
                        uint8_t        broadcastRadius,
                        uint8_t        option,
                        uint8_t       *payload,
                        uint8_t        payloadLength,
                        uint8_t        frameId,
                        uint8_t        srcEndpoint,
                        uint8_t        dstEndpoint,
                        uint16_t       clusterId,
                        uint16_t       profileId);

    /**
     * 创建此类的默认实例。 发送此请求之前，您至少必须指定有效负载，有效负载长度和目标地址。
     *
     * 此外，它使用the Maxstream profile (0xc105), both endpoints 232 and cluster
     *0x0011，
     * 导致与正常ZBExplicitTxRequest发送的数据包相同。
     */
    ZBExplicitTxRequest();
    uint8_t  getSrcEndpoint();
    uint8_t  getDstEndpoint();
    uint16_t getClusterId();
    uint16_t getProfileId();
    void     setSrcEndpoint(uint8_t endpoint);
    void     setDstEndpoint(uint8_t endpoint);
    void     setClusterId(uint16_t clusterId);
    void     setProfileId(uint16_t profileId);

protected:

    // 声明虚函数
    uint8_t getFrameData(uint8_t pos);
    uint8_t getFrameDataLength();

private:

    uint8_t _srcEndpoint;
    uint8_t _dstEndpoint;
    uint16_t _profileId;
    uint16_t _clusterId;
};

/**
 * 表示一个AT Command TX数据包
 * 该命令用于配置串行连接的XBee无线电
 */
class AtCommandRequest : public XBeeRequest {
public:

    AtCommandRequest();
    AtCommandRequest(uint8_t *command);
    AtCommandRequest(uint8_t *command,
                     uint8_t *commandValue,
                     uint8_t  commandValueLength);
    uint8_t  getFrameData(uint8_t pos);
    uint8_t  getFrameDataLength();
    uint8_t* getCommand();
    void     setCommand(uint8_t *command);
    uint8_t* getCommandValue();
    void     setCommandValue(uint8_t *command);
    uint8_t  getCommandValueLength();
    void     setCommandValueLength(uint8_t length);

    /**
     * 清除可选的commandValue和commandValueLength，以便可以发送查询
     */
    void     clearCommandValue();

    // void reset();

private:

    uint8_t *_command;
    uint8_t *_commandValue;
    uint8_t _commandValueLength;
};

/**
 * 表示远程AT命令TX数据包
    * 该命令用于配置远程XBee无线电
 */
class RemoteAtCommandRequest : public AtCommandRequest {
public:

    RemoteAtCommandRequest();

    /**
     * 创建一个具有16位地址的RemoteAtCommandRequest来设置命令。
     * 默认为广播的64位地址，applyChanges为true。
     */
    RemoteAtCommandRequest(uint16_t remoteAddress16,
                           uint8_t *command,
                           uint8_t *commandValue,
                           uint8_t  commandValueLength);

    /**
     * 创建一个具有16位地址的RemoteAtCommandRequest来查询命令。
     * 默认为广播的64位地址，applyChanges为true。
     */
    RemoteAtCommandRequest(uint16_t remoteAddress16,
                           uint8_t *command);

    /**
     * 创建一个具有64位地址的RemoteAtCommandRequest来设置命令。
     * 16位地址默认为广播，applyChanges为true。
     */
    RemoteAtCommandRequest(XBeeAddress64& remoteAddress64,
                           uint8_t       *command,
                           uint8_t       *commandValue,
                           uint8_t        commandValueLength);

    /**
     * 创建一个具有16位地址的RemoteAtCommandRequest来查询命令。
     * 16位地址默认为广播，applyChanges为true。
     */
    RemoteAtCommandRequest(XBeeAddress64& remoteAddress64,
                           uint8_t       *command);
    uint16_t       getRemoteAddress16();
    void           setRemoteAddress16(uint16_t remoteAddress16);
    XBeeAddress64& getRemoteAddress64();
    void           setRemoteAddress64(XBeeAddress64& remoteAddress64);
    bool           getApplyChanges();
    void           setApplyChanges(bool applyChanges);
    uint8_t        getFrameData(uint8_t pos);
    uint8_t        getFrameDataLength();
    static XBeeAddress64 broadcastAddress64;

    //	静态uint16_t广播16地址;

private:

    XBeeAddress64 _remoteAddress64;
    uint16_t _remoteAddress16;
    bool _applyChanges;
};

#endif // ifndef __XBEE_REQUEST_H__
