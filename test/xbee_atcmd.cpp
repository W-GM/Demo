#include "xbee.h"

void framePrint(AtCommandResponse& atResponse)
{
    printf("frame packet is : 7e ");
    printf("%x ", atResponse.getMsbLength());
    printf("%x ", atResponse.getLsbLength());
    printf("%x ", atResponse.getApiId());


    for (int i = 0; i < atResponse.getFrameDataLength(); i++)
    {
        printf("%x ", atResponse.getFrameData()[i]);
    }
    printf("%x\n", atResponse.getChecksum());
}

/**
 * @brief 发送AT 命令函数
 * 
 * @param xbee XBee的实例化对象
 * @param atCmd AT命令
 * @param setVale AT命令值
 * @param valeLength AT命令值长度
 */
void xbeeAtCmd(XBee& xbee, uint8_t* atCmd, uint8_t* setVale, uint8_t valeLength)
{
    // 存放AT Command命令和参数的Tx数据包 的对象
    AtCommandRequest atRequest =
        AtCommandRequest(atCmd, setVale, valeLength);

    // 存储响应的对象
    AtCommandResponse atResponse = AtCommandResponse();

    debug("%s:%s:%d>> Sending command to the XBee\n", __FILE__, __func__, __LINE__);

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
                framePrint(atResponse);

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


