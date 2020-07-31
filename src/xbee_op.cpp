#include "xbee.h"
#include "xbee_op.h"

/**
 * @brief 发送AT 命令函数
 *
 * @param xbee XBee的实例化对象
 * @param atCmd AT命令
 * @param setVale AT命令值
 * @param valeLength AT命令值长度
 */
void xbeeAtCmd(XBee& xbee, uint8_t *atCmd, uint8_t *setVale, uint8_t valeLength)
{
    // 存放AT Command命令和参数的Tx数据包 的对象
    AtCommandRequest atRequest =
        AtCommandRequest(atCmd, setVale, valeLength);

    // 存储响应的对象
    AtCommandResponse atResponse = AtCommandResponse();

    debug("%s:%s:%d>> Sending command to the XBee\n", __FILE__, __func__,
          __LINE__);

    // 发送
    xbee.send(atRequest);

    // 等待长达5秒钟的状态响应
    if (xbee.readPacket(10))
    {
        debug("%s:%s:%d  get a response\n", __FILE__, __func__, __LINE__);

        if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE)
        {
            // 获得一个AT Command 响应
            xbee.getResponse().getAtCommandResponse(atResponse);

            if (atResponse.isOk())
            {
                printf("Command [");
                printf("%c", atResponse.getCommand()[0]);
                printf("%c", atResponse.getCommand()[1]);
                printf("] was successful!\n");

                // 打印帧数据


                printf("frame packet is : 7e ");
                printf("%x ", atResponse.getMsbLength());
                printf("%x ", atResponse.getLsbLength());
                printf("%x ", atResponse.getApiId());


                for (int i = 0; i < atResponse.getFrameDataLength(); i++)
                {
                    printf("%x ", atResponse.getFrameData()[i]);
                }
                printf("%x\n", atResponse.getChecksum());

                if (atResponse.getValueLength() > 0)
                {
                    printf("command value length is: ");
                    printf("%d\n", atResponse.getValueLength());

                    printf("Command value: ");

                    for (int i = 0; i < atResponse.getValueLength(); i++)
                    {
                        printf("%x", atResponse.getValue()[i]);
                        printf(" ");
                    }

                    printf(" \n");
                }
            }
            else
            {
                printf("Command return error code: ");
                printf("%x\n", atResponse.getStatus());
            }
        }
        else
        {
            printf("Return an error AT response: ");
            printf("%x\n", xbee.getResponse().getApiId());
        }
    }
    else
    {
        debug("%s:%s:%d  response time out\n", __FILE__, __func__, __LINE__);

        if (xbee.getResponse().isError())
        {
            printf("Error reading packet.  Error code: ");
            printf("%x\n", xbee.getResponse().getErrorCode());
        }
        else
        {
            printf("No response from radio\n");
        }
    }
}

/**
 * @brief 发送远程AT命令函数
 *
 * @param xbee XBee的实例化对象
 * @param XBeeAddr 64位远程设备地址
 * @param atCmd 远程AT命令
 * @param setVale 远程AT命令值
 * @param valeLength 远程AT命令值长度
 */
void xbeeRemoteAtCmd(XBee         & xbee,
                     XBeeAddress64& remoteAddr,
                     uint8_t       *atCmd,
                     uint8_t       *setVale,
                     uint8_t        valeLength)
{
    // SH + SL 64位远程地址
    // XBeeAddress64 remoteAddress = XBeeAddress64(0x0013a200, 0x400a3e02);

    // 使用IR命令创建远程AT请求
    RemoteAtCommandRequest remoteAtRequest = RemoteAtCommandRequest(remoteAddr,
                                                                    atCmd,
                                                                    setVale,
                                                                    valeLength);

    // 创建一个远程AT响应对象
    RemoteAtCommandResponse remoteAtResponse = RemoteAtCommandResponse();

    // 发送命令
    xbee.send(remoteAtRequest);

    // 等待长达5秒钟的状态响应
    if (xbee.readPacket(10))
    {
        debug("%s:%s:%d  get a response\n", __FILE__, __func__, __LINE__);

        if (xbee.getResponse().getApiId() == REMOTE_AT_COMMAND_RESPONSE)
        {
            xbee.getResponse().getRemoteAtCommandResponse(remoteAtResponse);

            if (remoteAtResponse.isOk())
            {
                printf("Command [");
                printf("%c", remoteAtResponse.getCommand()[0]);
                printf("%c", remoteAtResponse.getCommand()[1]);
                printf("] was successful!\n");

                // 打印帧数据

                printf("frame packet is : 7e ");
                printf("%x ", remoteAtResponse.getMsbLength());
                printf("%x ", remoteAtResponse.getLsbLength());
                printf("%x ", remoteAtResponse.getApiId());


                for (int i = 0; i < remoteAtResponse.getFrameDataLength(); i++)
                {
                    printf("%x ", remoteAtResponse.getFrameData()[i]);
                }
                printf("%x\n", remoteAtResponse.getChecksum());

                if (remoteAtResponse.getValueLength() > 0)
                {
                    printf("command value length is: ");
                    printf("%d\n", remoteAtResponse.getValueLength());

                    printf("Command value: ");

                    for (int i = 0; i < remoteAtResponse.getValueLength(); i++)
                    {
                        printf("%x", remoteAtResponse.getValue()[i]);
                        printf(" ");
                    }

                    printf(" \n");
                }
            }
            else
            {
                printf("Command return error code: ");
                printf("%x\n", remoteAtResponse.getStatus());
            }
        }
        else
        {
            printf("Expected Remote AT response but got ");
            printf("%x\n", xbee.getResponse().getApiId());
        }
    }
    else if (xbee.getResponse().isError())
    {
        printf("Error reading packet.  Error code: ");
        printf("%x\n", xbee.getResponse().getErrorCode());
    }
    else
    {
        printf("No response from radio\n");
    }
}

/**
 * @brief 发送帧数据
 * 
 * @param xbee 初始化一个XBee的对象
 * @param payload 要发送的数据
 * @param payloadlen 数据长度
 * @param addr64 要发送的目标的64位地址
 */
void xbeeTx(XBee& xbee, uint8_t *payload, uint8_t payloadlen,
            XBeeAddress64 addr64, uint8_t frameID)
{
    /* SH + SL Address of receiving XBee */
    
    // ZBTxRequest zbTx = ZBTxRequest(addr64, payload, payloadlen); /* 0x10 */
    ZBTxRequest zbTx = ZBTxRequest(addr64, ZB_BROADCAST_ADDRESS, 
                        ZB_BROADCAST_RADIUS_MAX_HOPS, ZB_TX_UNICAST, 
                       payload, payloadlen, frameID); /* 0x10 */
    //ZBExplicitTxRequest zbTx = ZBExplicitTxRequest(addr64, ZB_BROADCAST_ADDRESS, payload, payloadlen); /* 0x11 */

    xbee.send(zbTx);
}

/**
 * @brief 接收ZigBee的数据
 * 
 * @param xbee xbee handler
 * @param data 返回给调用者的数据
 * @param len 返回给调用者的数据长度
 * @param slave_addr 返回给调用者的井口64位地址
 * @return int 成功：1；失败：-1；超时：0
 */
int xbeeRx(XBee& xbee, uint8_t *data, int *len, uint64_t *slave_addr)
{
    ZBTxStatusResponse txStatus = ZBTxStatusResponse();  // 8b

    ZBRxResponse rx = ZBRxResponse();                    // 90
    ModemStatusResponse msr = ModemStatusResponse();     // 8a

    ZBRxIoSampleResponse rxios = ZBRxIoSampleResponse(); // 91

    uint64_t recv_len[8] = {0};

    int ret = xbee.readPacket();
    if(ret == 0)
    {
        return ret;
    }
    debug("%s\n", " ");

    if (xbee.getResponse().isAvailable())
    {
        /* 8a帧 */
        if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE)
        {
            xbee.getResponse().getModemStatusResponse(msr);

            // 本地XBee在某些事件上发送此响应，例如
            // 关联/解除关联

            if (msr.getStatus() == ASSOCIATED)
            {
                //printf("已成功加入网络\n");
            }
            else if (msr.getStatus() == DISASSOCIATED)
            {
                printf("已解除关联\n");
            }
            else
            {
                printf("其他状态响应\n");
            }
        }

        /* 状态响应（8b）帧 */
        else if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
        {
            xbee.getResponse().getZBTxStatusResponse(txStatus);

            // 获取发送(交付)状态，在第五个字节
            if (txStatus.getDeliveryStatus() == SUCCESS)
            {
                printf("响应成功\n");
            }
            else if (txStatus.getDeliveryStatus() == ADDRESS_NOT_FOUND)
            {
                printf("查找地址失败\n");
                return -1;
            }
            else if (txStatus.getDeliveryStatus() == NOT_JOINED_TO_NETWORK)
            {
                printf("设备未加入网络\n");
                return -1;
            }
            else
            {
                printf("响应失败（其他原因）\n");
                return -1;
            }
        }

        /* 90帧 */
        else if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
        {
            xbee.getResponse().getZBRxResponse(rx);

            if (rx.getOption90() == ZB_PACKET_ACKNOWLEDGED)
            {
                //printf("数据包（90帧）已确认\n");
            }

            // TODO：判断数据包是其他类型：
            // 如：广播数据包；使用APS加密的数据包；数据包是从终端设备发送的(如果已知)
            else if (rx.getOption90() == ZB_BROADCAST_PACKET)
            {
                printf("数据包为广播数据包\n");
            }
            else
            {
                printf("数据包是其他类型\n");
                return -1;
            }

            /* 打印接收到的数据包 */
            // debug("%s:%s:%d>>\n", __FILE__, __func__, __LINE__);
            // debug("%s",           "frame packet is >> 7e ");
            // debug("%x ",          rx.getMsbLength());
            // debug("%x ",          rx.getLsbLength());
            // debug("%x ",          rx.getApiId());

            for (int i = 0; i < rx.getFrameDataLength(); i++)
            {
                data[i] = rx.getFrameData()[i];
            }
            
            *len =  rx.getFrameDataLength();
            if(slave_addr != nullptr)
            {
                for(int i=0; i<8;i++)
                {
                    recv_len[i] = rx.getFrameData()[i];
                }

                *slave_addr = recv_len[0] << 56 | 
                          recv_len[1] << 48 | 
                          recv_len[2] << 40 | 
                          recv_len[3] << 32 | 
                          recv_len[4] << 24 |
                          recv_len[5] << 16 | 
                          recv_len[6] << 8 | 
                          recv_len[7] << 0;
            }
        }

        /* 91帧 */
        else if (xbee.getResponse().getApiId() == ZB_EXPLICIT_RX_RESPONSE)
        {
            xbee.getResponse().getZBRxIoSampleResponse(rxios);

            if ((rxios.getOption91() == ZB_PACKET_ACKNOWLEDGED) ||
                (rxios.getOption91() == ZB_PACKET_ACKNOWLEDGED_FROM_TERMINAL))
            {
                //printf("数据包（91帧）已确认\n");
            }
            else if ((rxios.getOption91() == ZB_BROADCAST_PACKET) ||
                     (rxios.getOption91() ==
                      ZB_BROADCAST_PACKET_FROM_TERMINAL))
            {
                printf("数据包为广播数据包\n");
            }
            else
            {
                printf("数据包是其他类型\n");
                return -1;
            }

            /* 打印接收到的数据包 */
            // debug("%s:%s:%d>> \n", __FILE__, __func__, __LINE__);
            // debug("%s",            "frame packet is >> 7e ");
            // debug("%x ",           rxios.getMsbLength());
            // debug("%x ",           rxios.getLsbLength());
            // debug("%x ",           rxios.getApiId());
// 
            /* 将收到的数据发送给调用者 */
            for (int i = 0; i < rxios.getFrameDataLength() - 17; i++)
            {
                data[i] = rxios.getFrameData()[i + 17];
            }
            *len =  rxios.getFrameDataLength() - 17;

            if(slave_addr)
            {
                for(int i=0; i<8;i++)
                {
                    recv_len[i] = rxios.getFrameData()[i];
                }

                *slave_addr = recv_len[0] << 56 | 
                          recv_len[1] << 48 | 
                          recv_len[2] << 40 | 
                          recv_len[3] << 32 | 
                          recv_len[4] << 24 |
                          recv_len[5] << 16 | 
                          recv_len[6] << 8 | 
                          recv_len[7] << 0;
            }
            
        }
        else
        {
            printf("其他的API ID >> 0x%02x\n", xbee.getResponse().getApiId());

            return -1;
        }
    }
    else if (xbee.getResponse().isError())
    {
        printf("响应解析失败\n");
        return -1;
    }
    else
    {
        printf("其他错误\n");
        return -1;
    }
    return 1;
}
