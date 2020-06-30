/**

 */
#include <stdio.h>
#include "xbee_response.h"

// 默认构造函数
XBeeResponse::XBeeResponse()
{}

// 获取响应的api的id值(frame id)
uint8_t XBeeResponse::getApiId()
{
    return _apiId;
}

void XBeeResponse::setApiId(uint8_t apiId)
{
    _apiId = apiId;
}

// 获取数据包的高字节长度
uint8_t XBeeResponse::getMsbLength()
{
    return _msbLength;
}

void XBeeResponse::setMsbLength(uint8_t msbLength)
{
    _msbLength = msbLength;
}

// 获取数据包的低字节长度
uint8_t XBeeResponse::getLsbLength()
{
    return _lsbLength;
}

void XBeeResponse::setLsbLength(uint8_t lsbLength)
{
    _lsbLength = lsbLength;
}

// 获取数据包的校验和
uint8_t XBeeResponse::getChecksum()
{
    return _checksum;
}

void XBeeResponse::setChecksum(uint8_t checksum)
{
    _checksum = checksum;
}

// 获取帧数据的长度：api id(包含)之后和校验和之前的所有字节
uint8_t XBeeResponse::getFrameDataLength()
{
    return _frameLength;
}

/**
 * 设置frame data缓冲区
 * 以跟在API ID之后的字节开头，包括校验和之前的所有字节
 */
void XBeeResponse::setFrameData(uint8_t *frameDataPtr)
{
    _frameDataPtr = frameDataPtr;
}

uint8_t * XBeeResponse::getFrameData()
{
    return _frameDataPtr; // 在XBee的构造函数中定义，分配存储空间
}

void XBeeResponse::setFrameLength(uint8_t frameLength)
{
    _frameLength = frameLength;
}

/**
 * 返回数据包的长度
 */
uint16_t XBeeResponse::getPacketLength()
{
    return ((_msbLength << 8) & 0xff00) + (_lsbLength & 0xff); // [已改]
}

void XBeeResponse::reset()
{
    init();
    _apiId = 0;
    _msbLength = 0;
    _lsbLength = 0;
    _checksum = 0;
    _frameLength = 0;

    _errorCode = NO_ERROR;
}

/**
 * 初始化响应
 */
void XBeeResponse::init()
{
    _complete = false;
    _errorCode = NO_ERROR;
    _checksum = 0;
}

#ifdef SERIES_2
void XBeeResponse::getZBTxStatusResponse(XBeeResponse& zbXBeeResponse)
{
    // 离开吗？
    ZBTxStatusResponse *zb = static_cast<ZBTxStatusResponse *>(&zbXBeeResponse);

    // 将指针数组传递给子类
    zb->setFrameData(getFrameData());
    setCommon(zbXBeeResponse);
}

void XBeeResponse::getZBRxResponse(XBeeResponse& rxResponse)
{
    ZBRxResponse *zb = static_cast<ZBRxResponse *>(&rxResponse);

    // TODO： 验证响应API ID是否与此响应的API匹配

    // 将指针数组传递给子类
    zb->setFrameData(getFrameData());
    setCommon(rxResponse);

    zb->getRemoteAddress64().setMsb((uint32_t(getFrameData()[0]) << 24) +
                                    (uint32_t(getFrameData()[1]) << 16) +
                                    (uint16_t(
                                         getFrameData()
                                         [2])
                                     << 8) +
                                         getFrameData()[3]);
    zb->getRemoteAddress64().setLsb((uint32_t(getFrameData()[4]) << 24) +
                                    (uint32_t(getFrameData()[5]) << 16) +
                                    (uint16_t(getFrameData()[6]) << 8) +
                                    (getFrameData()[7]));
}

void XBeeResponse::getZBExplicitRxResponse(XBeeResponse& rxResponse)
{
    // 没有补充
    getZBRxResponse(rxResponse);
}

void XBeeResponse::getZBRxIoSampleResponse(XBeeResponse& response)
{
    ZBRxIoSampleResponse *zb = static_cast<ZBRxIoSampleResponse *>(&response);

    // 将指针数组传递给子类
    zb->setFrameData(getFrameData());
    setCommon(response);

    zb->getRemoteAddress64().setMsb((uint32_t(getFrameData()[0]) << 24) +
                                    (uint32_t(getFrameData()[1]) << 16) +
                                    (uint16_t(
                                         getFrameData()
                                         [2]) << 8) + getFrameData()[3]);
    zb->getRemoteAddress64().setLsb((uint32_t(getFrameData()[4]) << 24) +
                                    (uint32_t(getFrameData()[5]) << 16) +
                                    (uint16_t(getFrameData()[6]) << 8) +
                                    (getFrameData()[7]));
}

#endif // ifdef SERIES_2

#ifdef SERIES_1
void XBeeResponse::getTxStatusResponse(XBeeResponse& txResponse)
{
    TxStatusResponse *txStatus = static_cast<TxStatusResponse *>(&txResponse);

    // 将指针数组传递给子类
    txStatus->setFrameData(getFrameData());
    setCommon(txResponse);
}

void XBeeResponse::getRx16Response(XBeeResponse& rx16Response)
{
    Rx16Response *rx16 = static_cast<Rx16Response *>(&rx16Response);

    // 将指针数组传递给子类
    rx16->setFrameData(getFrameData());
    setCommon(rx16Response);
}

void XBeeResponse::getRx64Response(XBeeResponse& rx64Response)
{
    Rx64Response *rx64 = static_cast<Rx64Response *>(&rx64Response);

    // 将指针数组传递给子类
    rx64->setFrameData(getFrameData());
    setCommon(rx64Response);

    rx64->getRemoteAddress64().setMsb((uint32_t(getFrameData()[0]) << 24) +
                                      (uint32_t(getFrameData()[1]) << 16) +
                                      (uint16_t(
                                           getFrameData()
                                           [2])
                                       << 8) +
                                           getFrameData()[3]);
    rx64->getRemoteAddress64().setLsb((uint32_t(getFrameData()[4]) << 24) +
                                      (uint32_t(getFrameData()[5]) << 16) +
                                      (uint16_t(
                                           getFrameData()
                                           [6])
                                       << 8) +
                                           getFrameData()[7]);
}

#endif // ifdef SERIES_1

void XBeeResponse::getAtCommandResponse(XBeeResponse& atCommandResponse)
{
    AtCommandResponse *at = static_cast<AtCommandResponse *>(&atCommandResponse);

    // 将指针数组传递给子类
    at->setFrameData(getFrameData());
    setCommon(atCommandResponse);
}

void XBeeResponse::getRemoteAtCommandResponse(XBeeResponse& response)
{
    // TODO：没有真正的必要。 更改arg以匹配期望的类
    RemoteAtCommandResponse *at =
        static_cast<RemoteAtCommandResponse *>(&response);

    // 将指针数组传递给子类
    at->setFrameData(getFrameData());
    setCommon(response);

    at->getRemoteAddress64().setMsb((uint32_t(getFrameData()[1]) << 24) +
                                    (uint32_t(getFrameData()[2]) << 16) +
                                    (uint16_t(
                                         getFrameData()
                                         [3]) << 8) + getFrameData()[4]);
    at->getRemoteAddress64().setLsb((uint32_t(getFrameData()[5]) << 24) +
                                    (uint32_t(getFrameData()[6]) << 16) +
                                    (uint16_t(getFrameData()[7]) << 8) +
                                    (getFrameData()[8]));
}

void XBeeResponse::getModemStatusResponse(XBeeResponse& modemStatusResponse)
{
    ModemStatusResponse *modem =
        static_cast<ModemStatusResponse *>(&modemStatusResponse);

    // pass pointer array to subclass
    modem->setFrameData(getFrameData());
    setCommon(modemStatusResponse);
}

/**
 * 如果响应已成功解析并且完整且可以使用，则返回true
 */
bool XBeeResponse::isAvailable()
{
    return _complete;
}

void XBeeResponse::setAvailable(bool complete)
{
    _complete = complete;
}

/**
 * 如果响应包含错误，则返回true
 */
bool XBeeResponse::isError()
{
    return _errorCode > 0;
}

// 返回错误代码，如果成功，则返回零。
// 错误代码(包括：CHECKSUM_FAILURE，PACKET_EXCEEDS_BYTE_ARRAY_LENGTH，UNEXPECTED_START_BYTE)
uint8_t XBeeResponse::getErrorCode()
{
    return _errorCode;
}

void XBeeResponse::setErrorCode(uint8_t errorCode)
{
    _errorCode = errorCode;
}

// 将常见字段从xbee响应复制到目标响应
void XBeeResponse::setCommon(XBeeResponse& target)
{
    target.setApiId(getApiId());
    target.setAvailable(isAvailable());
    target.setChecksum(getChecksum());
    target.setErrorCode(getErrorCode());
    target.setFrameLength(getFrameDataLength());
    target.setMsbLength(getMsbLength());
    target.setLsbLength(getLsbLength());
}

/*+++++++++++++++++++FrameIdResponse+++++++++++++++++++++*/
FrameIdResponse::FrameIdResponse()
{}

uint8_t FrameIdResponse::getFrameId()
{
    return getFrameData()[0];
}

/**************ZBTxStatusResponse*******************/

#ifdef SERIES_2

// 2系列TX状态数据包默认构造函数？
ZBTxStatusResponse::ZBTxStatusResponse() : FrameIdResponse()
{}

// 获取远程地址
uint16_t ZBTxStatusResponse::getRemoteAddress()
{
    return (getFrameData()[1] << 8) + getFrameData()[2];
}

// 获取重试次数
uint8_t ZBTxStatusResponse::getTxRetryCount()
{
    return getFrameData()[3];
}

// 获取交付(发送)状态？
uint8_t ZBTxStatusResponse::getDeliveryStatus()
{
    return getFrameData()[4];
}

// 获取发现状态？
uint8_t ZBTxStatusResponse::getDiscoveryStatus()
{
    return getFrameData()[5];
}

//
bool ZBTxStatusResponse::isSuccess()
{
    return getDeliveryStatus() == SUCCESS;
}

#endif // ifdef SERIES_2

/***********AtCommandResponse*************/
AtCommandResponse::AtCommandResponse()
{}

uint8_t * AtCommandResponse::getCommand()
{
    return getFrameData() + 1;
}

uint8_t AtCommandResponse::getStatus()
{
    return getFrameData()[3];
}

uint8_t * AtCommandResponse::getValue()
{
    if (getValueLength() > 0)
    {
        // 值仅包含在查询命令中。 设置命令不返回值
        return getFrameData() + 4;
    }

    return nullptr;
}

uint8_t AtCommandResponse::getValueLength()
{
    return getFrameDataLength() - 4;
}

bool AtCommandResponse::isOk()
{
    return getStatus() == AT_OK;
}

/***********RemoteAtCommandResponse************/
RemoteAtCommandResponse::RemoteAtCommandResponse() : AtCommandResponse()
{}

uint8_t * RemoteAtCommandResponse::getCommand()
{
    return getFrameData() + 11;
}

uint8_t RemoteAtCommandResponse::getStatus()
{
    return getFrameData()[13];
}

uint8_t * RemoteAtCommandResponse::getValue()
{
    if (getValueLength() > 0)
    {
        // 值仅包含在查询命令中。 设置命令不返回值
        return getFrameData() + 14;
    }

    return nullptr;
}

uint8_t RemoteAtCommandResponse::getValueLength()
{
    return getFrameDataLength() - 14;
}

uint16_t RemoteAtCommandResponse::getRemoteAddress16()
{
    return uint16_t((getFrameData()[9] << 8) + getFrameData()[10]);
}

XBeeAddress64& RemoteAtCommandResponse::getRemoteAddress64()
{
    return _remoteAddress64;
}

bool RemoteAtCommandResponse::isOk()
{
    // 奇怪的C ++行为。 不使用此方法，它将调用AtCommandResponse :: isOk（），
    // 后者将调用AtCommandResponse :: getStatus，而不是this.getStatus ！！！
    return getStatus() == AT_OK;
}

/*+++++++++++++++++RxDataResponse++++++++++++++++++++*/

RxDataResponse::RxDataResponse() : XBeeResponse()
{}

uint8_t RxDataResponse::getData(int index)
{
    return getFrameData()[getDataOffset() + index];
}

uint8_t * RxDataResponse::getData()
{
    return getFrameData() + getDataOffset();
}

#ifdef SERIES_2

/***********ZBRxResponse************/
ZBRxResponse::ZBRxResponse() : RxDataResponse()
{
    _remoteAddress64 = XBeeAddress64();
}

uint16_t ZBRxResponse::getRemoteAddress16()
{
    return (getFrameData()[8] << 8) + getFrameData()[9];
}

uint8_t ZBRxResponse::getOption90()
{
    return getFrameData()[10];
}
uint8_t ZBRxResponse::getOption91()
{
    debug("%s:%s:%d>> %02x\n", __FILE__, __func__, __LINE__, getFrameData()[16]);
    return getFrameData()[16];
}

// 标记以从数据包阵列读取数据。 这是索引，所以是数组中的第12个项目
uint8_t ZBRxResponse::getDataOffset()
{
    return 11;
}

uint8_t ZBRxResponse::getDataLength()
{
    return getPacketLength() - getDataOffset() - 1;
}

XBeeAddress64& ZBRxResponse::getRemoteAddress64()
{
    return _remoteAddress64;
}

/***********ZBExplicitRxResponse************/
ZBExplicitRxResponse::ZBExplicitRxResponse() : ZBRxResponse()
{}

uint8_t ZBExplicitRxResponse::getSrcEndpoint()
{
    return getFrameData()[10];
}

uint8_t ZBExplicitRxResponse::getDstEndpoint()
{
    return getFrameData()[11];
}

uint16_t ZBExplicitRxResponse::getClusterId()
{
    return (uint16_t)(getFrameData()[12]) << 8 | getFrameData()[13];
}

uint16_t ZBExplicitRxResponse::getProfileId()
{
    return (uint16_t)(getFrameData()[14]) << 8 | getFrameData()[15];
}

uint8_t ZBExplicitRxResponse::getOption()
{
    return getFrameData()[16];
}

// 标记以从数据包阵列读取数据。
uint8_t ZBExplicitRxResponse::getDataOffset()
{
    return 17;
}

uint8_t ZBExplicitRxResponse::getDataLength()
{
    return getPacketLength() - getDataOffset() - 1;
}

/***********ZBExplicitRxResponse************/
ZBRxIoSampleResponse::ZBRxIoSampleResponse() : ZBRxResponse()
{}

// 64 + 16个地址，样本量，选项= 12（索引11），因此从12开始
uint8_t ZBRxIoSampleResponse::getDigitalMaskMsb()
{
    return getFrameData()[12] & 0x1c;
}

uint8_t ZBRxIoSampleResponse::getDigitalMaskLsb()
{
    return getFrameData()[13];
}

uint8_t ZBRxIoSampleResponse::getAnalogMask()
{
    return getFrameData()[14] & 0x8f;
}

bool ZBRxIoSampleResponse::containsAnalog()
{
    return getAnalogMask() > 0;
}

bool ZBRxIoSampleResponse::containsDigital()
{
    return getDigitalMaskMsb() > 0 || getDigitalMaskLsb() > 0;
}

bool ZBRxIoSampleResponse::isAnalogEnabled(uint8_t pin)
{
    return ((getAnalogMask() >> pin) & 1) == 1;
}

bool ZBRxIoSampleResponse::isDigitalEnabled(uint8_t pin)
{
    if (pin <= 7)
    {
        // 添加了额外的括号以使AVR编译器平静
        return ((getDigitalMaskLsb() >> pin) & 1) == 1;
    }
    else
    {
        return ((getDigitalMaskMsb() >> (pin - 8)) & 1) == 1;
    }
}

uint16_t ZBRxIoSampleResponse::getAnalog(uint8_t pin)
{
    // 如果未启用dio，则模拟量将在样本大小后开始13个字节
    uint8_t start = 15;

    if (containsDigital())
    {
        // 为数字I / O腾出空间
        start += 2;
    }

    // 启动取决于启用此引脚之前有多少个引脚
    for (int i = 0; i < pin; i++)
    {
        if (isAnalogEnabled(i))
        {
            start += 2;
        }
    }

    return (uint16_t)((getFrameData()[start] << 8) + getFrameData()[start + 1]);
}

bool ZBRxIoSampleResponse::isDigitalOn(uint8_t pin)
{
    if (pin <= 7)
    {
        // D0-7
        // DIO LSB is index 5
        return ((getFrameData()[16] >> pin) & 1) == 1;
    }
    else
    {
        // D10-12
        // DIO MSB is index 4
        return ((getFrameData()[15] >> (pin - 8)) & 1) == 1;
    }
}

#endif // ifdef SERIES_2

#ifdef SERIES_1

/****************RxResponse*****************/

RxResponse::RxResponse() : RxDataResponse()
{}

uint8_t RxResponse::getRssi()
{
    return getFrameData()[getRssiOffset()];
}

uint8_t RxResponse::getOption()
{
    return getFrameData()[getRssiOffset() + 1];
}

bool RxResponse::isAddressBroadcast()
{
    return (getOption() & 2) == 2;
}

bool RxResponse::isPanBroadcast()
{
    return (getOption() & 4) == 4;
}

uint8_t RxResponse::getDataOffset()
{
    return getRssiOffset() + 2;
}

uint8_t RxResponse::getDataOffset()
{
    return getRssiOffset() + 2;
}

/**************Rx16Response**************/

Rx16Response::Rx16Response() : RxResponse() {}

uint8_t Rx16Response::getRssiOffset()
{
    return RX_16_RSSI_OFFSET;
}

uint16_t Rx16Response::getRemoteAddress16()
{
    return (getFrameData()[0] << 8) + getFrameData()[1];
}

/*************Rx64Response******************/

Rx64Response::Rx64Response() : RxResponse()
{
    _remoteAddress = XBeeAddress64();
}

#endif // ifdef SERIES_1

/***************ModemStatusResponse****************/
ModemStatusResponse::ModemStatusResponse()
{}

uint8_t ModemStatusResponse::getStatus()
{
    return getFrameData()[0];
}
