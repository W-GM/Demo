#include "xbee.h"

/**
 * @brief 发送帧数据
 *
 * @param xbee 初始化一个XBee的对象
 * @param payload 要发送的数据
 * @param payloadlen 数据长度
 */
void xbeeTx(XBee& xbee, uint8_t *payload, uint8_t payloadlen,
            XBeeAddress64 addr64)
{
    // SH + SL Address of receiving XBee
    ZBTxRequest zbTx = ZBTxRequest(addr64, payload, payloadlen);
    ZBTxStatusResponse txStatus = ZBTxStatusResponse();

    xbee.send(zbTx);
    debug("%s:%s:%d:请求(数据)已发送\n", __FILE__, __func__, __LINE__);

    // 在发送tx请求后，我们期望一个状态响应
    // 等待最多半秒钟的状态响应
    if (xbee.readPacket(500))
    {
        if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
        {
            xbee.getResponse().getZBTxStatusResponse(txStatus);

            // 获取发送(交付)状态，在第五个字节
            if (txStatus.getDeliveryStatus() == SUCCESS)
            {
                printf("响应成功\n");
            }
            else
            {
                printf("响应失败\n");
            }
        }
    }
    else if (xbee.getResponse().isError())
    {
        // nss.print("Error reading packet.  Error code: ");
        // nss.println(xbee.getResponse().getErrorCode());
        printf("响应包含错误\n");
    }
    else
    {
        printf("本地XBee没有提供及时的TX状态响应\n");
    }
}
