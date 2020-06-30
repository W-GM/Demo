#include "xbee.h"
#include <stdio.h>

/**
 * xbee构造函数定义及初始化
 */
XBee::XBee() : _response(XBeeResponse())
{
    _pos = 0;
    _escape = false;
    _checksumTotal = 0;
    _nextFrameId = 0; // ???

    _response.init();

    // 为Rx数据包创建了一个存储空间 _responseFrameData
    _response.setFrameData(_responseFrameData);
}

/**
 * 读取所有可用的串行字节，直到数据包被解析，或发生错误或缓冲区为空。
 */
int XBee::readPacket()
{
    int ret = 0;

    // 重置先前的响应
    if (_response.isAvailable() || _response.isError())
    {
        // 丢弃先前的数据包并重新开始
        resetResponse();
    }

    // read()判断是否读到值，没有读到返回false
    // 并将读到的值保存在_b

    while (1)
    {
        ret = xbee_read();

        if (ret <= 0)
        {
            return ret;
        }

        while (ret)
        {
#ifdef ATAP_2

            if ((_pos > 0) && (b == START_BYTE) && (ATAP == 2))
            {
                // 在前一个包完成之前，新的包开始（丢弃前一个包，重新开始）
                _response.setErrorCode(UNEXPECTED_START_BYTE);
                return;
            }

            if ((_pos > 0) && (b == ESCAPE))
            {
                if (available())
                {
                    b = read();
                    b = 0x20 ^ b;
                }
                else
                {
                    // 转义字节。 下一个字节将是
                    // ？？？
                    _escape = true;
                    continue;
                }
            }

            // ？？？
            if (_escape == true)
            {
                b = 0x20 ^ b;
                _escape = false;
            }
#endif // ifdef ATAP_2

            // 校验和（包括以api id开头的所有字节）
            if (_pos >= API_ID_INDEX)
            {
                _checksumTotal += _b;
            }

            switch (_pos)
            {
            case 0:

                if (_b == START_BYTE)
                {
                    _pos++;
                }

                break;

            case 1:

                // 高字节长度
                _response.setMsbLength(_b);
                _pos++;

                break;

            case 2:

                // 低字节长度
                _response.setLsbLength(_b);
                _pos++;

                break;

            case 3:
                _response.setApiId(_b);
                _pos++;

                break;

            default:

                // 从第5字节开始

                if (_pos > MAX_FRAME_DATA_SIZE)
                {
                    // 超过最大大小。 出错
                    _response.setErrorCode(PACKET_EXCEEDS_BYTE_ARRAY_LENGTH);
                    return -1;
                }

                // 检查是否在数据包的末尾
                // 此时_pos在校验字节位
                if (_pos == (_response.getPacketLength() + 3))
                {
                    // 验证校验和
                    if ((_checksumTotal & 0xff) == 0xff)
                    {
                        _response.setChecksum(_b);
                        _response.setAvailable(true);

                        _response.setErrorCode(NO_ERROR);
                    }
                    else
                    {
                        // 校验失败
                        _response.setErrorCode(CHECKSUM_FAILURE);
                    }

                    // 减去4，因为帧数据长度不包括start，msb，lsb，校验和
                    _response.setFrameLength(_pos - 4); // 从API ID(不包含)之后到校验和(不包含)之前

                    // 重置vars状态
                    _pos = 0;

                    return -1;
                }
                else
                {
                    // 添加到数据包数组，从API ID之后开始
                    _response.getFrameData()[_pos - 4] = _b;
                    _pos++;
                }
            }
            break;
        }
    }
    return 1;
}

bool XBee::readPacket(int timeout)
{
    if (timeout < 0)
    {
        return false;
    }

    unsigned long start = millis();

    debug("%s:%s:%d  millis = %ld\n", __FILE__, __func__, __LINE__, start);

    while ((((long)millis() - (long)start)) < timeout)
    {
        debug("%s:%s:%d  millis = %ld\n", __FILE__, __func__, __LINE__,
              (millis() - start));
        readPacket();

        if (getResponse().isAvailable())
        {
            return true;
        }
        else if (getResponse().isError())
        {
            return false;
        }
    }
    debug("%s:%s:%d  time out :%ld\n", __FILE__, __func__, __LINE__,
          (millis() - start));

    // timed out
    return false;
}

void XBee::readPacketUntilAvailable()
{
    while (!(getResponse().isAvailable() || getResponse().isError()))
    {
        // read some more
        readPacket();
    }
}

void XBee::getResponse(XBeeResponse& response)
{
    response.setMsbLength(_response.getMsbLength());
    response.setLsbLength(_response.getLsbLength());
    response.setApiId(_response.getApiId());
    response.setFrameLength(_response.getFrameDataLength());

    response.setFrameData(_response.getFrameData());
}

XBeeResponse& XBee::getResponse()
{
    return _response;
}

/**
 * 从串行端口发送XBeeRequest（TX数据包）
 */
void XBee::send(XBeeRequest& request)
{
    // 发送0x7e
    sendByte(START_BYTE, false);

    // 发送length
    uint8_t msbLen = ((request.getFrameDataLength() + 2) >> 8) & 0xff; // 虚函数，须在子类中定义
    uint8_t lsbLen = (request.getFrameDataLength() + 2) & 0xff;        //同上

    sendByte(msbLen,               false);
    sendByte(lsbLen,               false);

    // 发送api id
    sendByte(request.getApiId(),   false);
    sendByte(request.getFrameId(), false);

    uint8_t checksum = 0;

    // 计算校验和，从api id开始
    checksum += request.getApiId();
    checksum += request.getFrameId();

    for (int i = 0; i < request.getFrameDataLength(); i++)
    {
        // 发送FrameDate(不包括apiId,FrameId)
        sendByte(request.getFrameData(i), false);
        checksum += request.getFrameData(i);
    }

    checksum = 0xff - checksum;

    // 发送 checksum
    sendByte(checksum, false);
}

// ？？？
uint8_t XBee::getNextFrameId()
{
    _nextFrameId++;

    if (_nextFrameId == 0)
    {
        // 无法发送0，因为这会禁用状态响应
        _nextFrameId = 1;
    }

    return _nextFrameId;
}

void XBee::setSerial(uart *serial)
{
    _serial = serial;
}

/**
 * 复位响应：丢弃先前的数据包并重新开始
 */
void XBee::resetResponse()
{
    _pos = 0;
    _escape = false;
    _checksumTotal = 0;
    _response.reset();
}

/**
 * 获取串口中是否可以读到数据（如果没有读到数据，为false）
 * 
 *
 */

/**
 * @brief 读取串口中的数据
 * 
 * @return int 大于0：返回读取到的长度；等于0：超时；小于0：出错；
 */
int XBee::xbee_read()
{
    uint8_t buffer = 0;
    int     ret = 0;

    ret = _serial->Read(&buffer, sizeof(buffer), 2000);

    // 小于零表示出错，
    if (ret < 0)
    {
        fprintf(stderr,
                "%s:%s:%d>> serial_read(): %s\n",
                __FILE__,
                __func__,
                __LINE__,
                _serial->errmsg());
        return ret;
    }
    else if (ret == 0)
    {
        debug("%s:%s:%d>>timeout\n", __FILE__, __func__, __LINE__);
        return ret;
    }
    _b = buffer & 0xff;
    debug("%02x ", buffer);
    return ret;
}

/**
 * 发送数据
 */
void XBee::write(uint8_t val)
{
    if (_serial->Write(&val, sizeof(val)) < 0)
    {
        debug("%s:%s:%d>> write to serial faild\n", __FILE__, __func__, __LINE__);
    }
}

/**
 * 判断是否为模式2（AP = 2），并发送字节和转义字符
 */
void XBee::sendByte(uint8_t b, bool escape)
{
    if (escape &&
        ((b == START_BYTE) || (b == ESCAPE) || (b == XON) || (b == XOFF)))
    {
        write(ESCAPE);
        write(b ^ 0x20);
    }
    else
    {
        write(b);
        debug("%02x ", b);
    }
}
