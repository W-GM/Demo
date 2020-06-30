#include "xbee.h"
#include "screen.h"
#include <vector>

std::vector<unsigned char> Data={'d','a','t','a',',','r','e','c','e','i','v','e','d'};

// 全局坐标
struct Point poi = Point(0x00, 0x22);

/**
 * @brief 接收帧数据包
 * 
 * @param xbee XBee 的实例化对象
 */
void xbeeRx(XBee& xbee, ZBRxIoSampleResponse &rxios, Screen *scr)
{
    XBeeResponse response = XBeeResponse();
    
    ZBRxResponse rx = ZBRxResponse();  // 90
    ModemStatusResponse msr = ModemStatusResponse();  // 8a
    rxios = ZBRxIoSampleResponse();  // 91

    while (1)
    {
        xbee.readPacket();

        if (xbee.getResponse().isAvailable())
        {
            if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
            {
                xbee.getResponse().getZBRxResponse(rx);

                if (rx.getOption90() == ZB_PACKET_ACKNOWLEDGED)
                {
                    printf("数据包已确认\n");
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
                }

                // 打印接收到的数据包
                printf("frame packet is : 7e ");
                printf("%x ", rx.getMsbLength());
                printf("%x ", rx.getLsbLength());
                printf("%x ", rx.getApiId());

                debug("%s:%s:%d---------------------\n", __FILE__, __func__, __LINE__);

                for (int i = 0; i < rx.getFrameDataLength(); i++)
                {
                    printf("%x ", rx.getFrameData()[i]);
                }

                printf("%x\n", rx.getChecksum());
            }
            else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE)
            {
                xbee.getResponse().getModemStatusResponse(msr);

                // 本地XBee在某些事件上发送此响应，例如
                // 关联/解除关联

                if (msr.getStatus() == ASSOCIATED)
                {
                    printf("已成功加入网络\n");
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
            else if (xbee.getResponse().getApiId() == ZB_EXPLICIT_RX_RESPONSE)
            {
                xbee.getResponse().getZBRxIoSampleResponse(rxios);

                if ((rxios.getOption91() == ZB_PACKET_ACKNOWLEDGED) || (rxios.getOption91() == ZB_PACKET_ACKNOWLEDGED_FROM_TERMINAL))
                {
                    printf("数据包已确认\n");
                }
                else if ((rxios.getOption91() == ZB_BROADCAST_PACKET) || (rxios.getOption91() == ZB_BROADCAST_PACKET_FROM_TERMINAL))
                {
                    printf("数据包为广播数据包\n");
                }
                else
                {
                    printf("数据包是其他类型\n");
                }

                // 打印接收到的数据包
                printf("frame packet is : 7e ");
                printf("%x ", rxios.getMsbLength());
                printf("%x ", rxios.getLsbLength());
                printf("%x ", rxios.getApiId());

                debug("%s:%s:%d---------------------\n", __FILE__, __func__, __LINE__);

                for (int i = 0; i < rxios.getFrameDataLength(); i++)
                {
                    printf("%x ", rxios.getFrameData()[i]);
                }
                
                printf("%x\n", rxios.getChecksum());
                
            }
            else
            {
                printf("其他的API ID\n");
            }
        }
        else if (xbee.getResponse().isError())
        {
            // nss.print("Error reading packet.  Error code: ");
            // nss.println(xbee.getResponse().getErrorCode());
            printf("响应解析失败\n");
        }

        // 在屏幕显示字符，支持，asciii 和 GB2312 编码
        scr->DisplayString(0, 1, 0x02, 0xffff, 0x0000, poi, Data);
#if 0
        for (int i = 0; i < rxios.getFrameDataLength(); i++)
        {
            Data.push_back(rxios.getFrameData()[i]);
            printf("--%02x--\n", rxios.getFrameData()[i]);
        }
#endif

    }
}