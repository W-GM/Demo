#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

// #define SERIES_1
#define SERIES_2

// 设置为XBee的ATAP值。 推荐AP = 2
// #define ATAP 2

#define START_BYTE 0x7e
#define ESCAPE 0x7d
#define XON 0x11
#define XOFF 0x13

// 此值确定用于接收RX数据包的字节数组的大小
// 大多数用户不会处理这么大的数据包，因此您可以对此进行调整
// 减少内存消耗的值。 但是请记住
// 如果RX数据包超出此大小，则无法解析！

// 此值由1系列无线电的最大数据包大小（100字节有效负载+ 64位地址+选项字节和rssi字节）确定
#define MAX_FRAME_DATA_SIZE 110

#define BROADCAST_ADDRESS 0xffff
#define ZB_BROADCAST_ADDRESS 0xfffe

// 帧数据的不变长度（不包括帧ID或api ID或可变数据大小（例如有效载荷，处于命令设置值）
#define ZB_TX_API_LENGTH 12
#define ZB_EXPLICIT_TX_API_LENGTH 18
#define TX_16_API_LENGTH 3
#define TX_64_API_LENGTH 9
#define AT_COMMAND_API_LENGTH 2
#define REMOTE_AT_COMMAND_API_LENGTH 13

// start/length(2)/api/frameid/checksum bytes
#define PACKET_OVERHEAD_LENGTH 6

// api始终是数据包中的第三个字节
#define API_ID_INDEX 3

// rssi字节的帧位置
#define RX_16_RSSI_OFFSET 2
#define RX_64_RSSI_OFFSET 8

#define DEFAULT_FRAME_ID 1
#define NO_RESPONSE_FRAME_ID 0

// 这些是XBee ZB模块在执行
// 常规的“ ZB TX请求”。
#define DEFAULT_ENDPOINT 232
#define DEFAULT_CLUSTER_ID 0x0011
//#define DEFAULT_PROFILE_ID 0xc105
#define DEFAULT_PROFILE_ID 0x1857

// TODO 放入tx16 class
#define ACK_OPTION 0
#define DISABLE_ACK_OPTION 1
#define BROADCAST_OPTION 4

// RX options 0x90 0x91
/*协调器向终端节点发送数据，Receive options的值，表示为匹配地址确认数据包*/
#define ZB_PACKET_ACKNOWLEDGED 0x01 
/*协调器向终端节点发送数据，Receive options的值，表示为广播地址确认数据包*/
#define ZB_BROADCAST_PACKET 0x02 
/*终端节点向协调器发送数据，Receive options的值，表示为匹配地址确认数据包*/
#define ZB_PACKET_ACKNOWLEDGED_FROM_TERMINAL 0x41
/*终端节点向协调器发送数据，Receive options的值，表示为广播地址确认数据包*/
#define ZB_BROADCAST_PACKET_FROM_TERMINAL 0x42

/**
 * Api Id 常数
 */
#define TX_64_REQUEST 0x0
#define TX_16_REQUEST 0x1
#define AT_COMMAND_REQUEST 0x08
#define AT_COMMAND_QUEUE_REQUEST 0x09
#define REMOTE_AT_REQUEST 0x17
#define ZB_TX_REQUEST 0x10
#define ZB_EXPLICIT_TX_REQUEST 0x11
#define RX_64_RESPONSE 0x80
#define RX_16_RESPONSE 0x81
#define RX_64_IO_RESPONSE 0x82
#define RX_16_IO_RESPONSE 0x83
#define AT_RESPONSE 0x88
#define TX_STATUS_RESPONSE 0x89
#define MODEM_STATUS_RESPONSE 0x8a
#define ZB_RX_RESPONSE 0x90
#define ZB_EXPLICIT_RX_RESPONSE 0x91
#define ZB_TX_STATUS_RESPONSE 0x8b
#define ZB_IO_SAMPLE_RESPONSE 0x92
#define ZB_IO_NODE_IDENTIFIER_RESPONSE 0x95
#define AT_COMMAND_RESPONSE 0x88
#define REMOTE_AT_COMMAND_RESPONSE 0x97

/**
 * TX STATUS 常数
 */
#define SUCCESS 0x0
#define CCA_FAILURE 0x2
#define INVALID_DESTINATION_ENDPOINT_SUCCESS 0x15
#define NETWORK_ACK_FAILURE 0x21
#define NOT_JOINED_TO_NETWORK 0x22
#define SELF_ADDRESSED 0x23
#define ADDRESS_NOT_FOUND 0x24
#define ROUTE_NOT_FOUND 0x25
#define PAYLOAD_TOO_LARGE 0x74

// XBeeWithCallbacks :: waitForStatus在超时时返回
#define XBEE_WAIT_TIMEOUT 0xff

// modem status    0x8a
#define HARDWARE_RESET 0          // 硬件复位
#define WATCHDOG_TIMER_RESET 1
#define ASSOCIATED 2              // 加入的网络(路由器和终端设备)
#define DISASSOCIATED 3           // 已解除关联
#define SYNCHRONIZATION_LOST 4    //
#define COORDINATOR_REALIGNMENT 5 //
#define COORDINATOR_STARTED 6     // 协调器已启动

#define ZB_BROADCAST_RADIUS_MAX_HOPS 0

#define ZB_TX_UNICAST 0
#define ZB_TX_BROADCAST 8

#define AT_OK 0
#define AT_ERROR 1
#define AT_INVALID_COMMAND 2
#define AT_INVALID_PARAMETER 3
#define AT_NO_RESPONSE 4

#define NO_ERROR 0
#define CHECKSUM_FAILURE 1
#define PACKET_EXCEEDS_BYTE_ARRAY_LENGTH 2
#define UNEXPECTED_START_BYTE 3

#ifdef DEBUG
# define debug(fmt, ...)                   \
    do                                     \
    {                                      \
        fprintf(stdout, fmt, __VA_ARGS__); \
    } while (0)
#else // ifdef DEBUG
# define debug(fmt, ...)
#endif // ifdef DEBUG

#endif // ifndef __CONSTANTS_H__
