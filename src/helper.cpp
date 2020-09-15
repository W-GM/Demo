#include "helper.h"
#include "constants.h"

/**
 * @brief 用于计算某一帧的crc校验值
 * 
 * @param buffer 要校验的数据帧
 * @param buffer_length 要校验的数据帧长度
 * @return uint16_t 返回校验值
 */
uint16_t crc16(uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
    uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
    unsigned int i;        /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ table_crc_hi[i];
        crc_lo = table_crc_lo[i];
    }

    return crc_hi << 8 | crc_lo;
}

/**
 * @brief 用于校验modbus RTU数据帧
 * 
 * @param buffer 要校验的数据帧
 * @param buffer_len 数据帧长度
 * @return int 校验成功返回0；失败返回-1
 */
int crc16_check(uint8_t *buffer, uint16_t buffer_len)
{
    /* 用于保存经过计算后得到的crc校验结果 */
    uint16_t crc_result = crc16(buffer, buffer_len - 2);
    /* 用于保存当前帧自带的crc校验值 */
    uint16_t crc_original = (buffer[buffer_len - 2] << 8) | 
                            (buffer[buffer_len - 1] & 0xff);
    if(crc_result == crc_original)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
 * @brief 制作自定义格式的 自定义/当前 时间
 * 
 * @param custom_time 指向自定义时间，当为nullptr时，获取当前时间
 * @return std::string 自定义格式的 自定义/当前 时间
 */
std::string get_time(time_t *custom_time)
{
    struct tm *t;
    time_t tt = 0;
    if(custom_time == nullptr)
    {
        time(&tt);
    }
    else
    {
        tt = *custom_time;
    }
    
    t = localtime(&tt);
    char temp[20];
    sprintf(temp, "%4d-%02d-%02d-%02d:%02d:%02d", t->tm_year + 1900,
            t->tm_mon + 1,
            t->tm_mday,
            t->tm_hour,
            t->tm_min,
            t->tm_sec);

    return std::string(temp);
}

/**
 * @brief Get the current time object
 * 
 * @return struct currtime 
 */
struct currtime get_current_time()
{
    struct tm *t;
    struct currtime currtime;
    
    time_t tt = 0;
    time(&tt);
    t = localtime(&tt);

    currtime.year = t->tm_year + 1900;
    currtime.month = t->tm_mon + 1;
    currtime.day = t->tm_mday;
    currtime.hour = t->tm_hour;
    currtime.minute = t->tm_min;
    currtime.second = t->tm_sec;

    return currtime;
}

/**
 * @brief 自定义debug打印语句
 * 
 * @param time 要打印的数据长度
 * @param data 要打印的数据
 * @param explain 解释说明
 */
void debug_custom(const char *explain, uint8_t *data, int time)
{
    if(explain != nullptr)
    {
        debug("%s\n", explain);
    }

    if(data != nullptr)
    {
        for (int i = 0; i < time; i++)
        {
            debug("%02x ", data[i]);
        }
        
        debug("%s\n", "");
    }
    
    
}
