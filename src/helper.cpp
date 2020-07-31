#include "helper.h"

/* crc 计算 */
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
