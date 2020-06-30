#include "xbee.h"

void framePrint(RemoteAtCommandResponse& remoteAtResponse)
{
    printf("frame packet is : 7e ");
    printf("%x ", remoteAtResponse.getMsbLength());
    printf("%x ", remoteAtResponse.getLsbLength());
    printf("%x ", remoteAtResponse.getApiId());


    for (int i = 0; i < remoteAtResponse.getFrameDataLength(); i++)
    {
        printf("%x ", remoteAtResponse.getFrameData()[i]);
    }
    printf("%x\n", remoteAtResponse.getChecksum());
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
                framePrint(remoteAtResponse);

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
