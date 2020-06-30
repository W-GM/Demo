/*
 * c-periphery
 * https://github.com/vsergeev/c-periphery
 * License: MIT
 */

#ifndef _PERIPHERY_SPI_H
# define _PERIPHERY_SPI_H

# ifdef __cplusplus
extern "C" {
# endif // ifdef __cplusplus

# include <stddef.h>
# include <stdint.h>

enum spi_error_code {
    SPI_ERROR_ARG       = -1, /* Invalid arguments */
    SPI_ERROR_OPEN      = -2, /* Opening SPI device */
    SPI_ERROR_QUERY     = -3, /* Querying SPI device attributes */
    SPI_ERROR_CONFIGURE = -4, /* Configuring SPI device attributes */
    SPI_ERROR_TRANSFER  = -5, /* SPI transfer */
    SPI_ERROR_CLOSE     = -6, /* Closing SPI device */
};

typedef enum spi_bit_order {
    MSB_FIRST,
    LSB_FIRST,
} spi_bit_order_t;

typedef struct spi_handle spi_t;

/* Primary Functions */
spi_t* spi_new(void);
int    spi_open(spi_t       *spi,
                const char  *path,
                unsigned int mode,
                uint32_t     max_speed);
int spi_open_advanced(spi_t          *spi,
                      const char     *path,
                      unsigned int    mode,
                      uint32_t        max_speed,
                      spi_bit_order_t bit_order,
                      uint8_t         bits_per_word,
                      uint8_t         extra_flags);
int spi_transfer(spi_t         *spi,
                 const uint8_t *txbuf,
                 uint8_t       *rxbuf,
                 size_t         len);
int  spi_close(spi_t *spi);
void spi_free(spi_t *spi);

/* Getters */
int  spi_get_mode(spi_t        *spi,
                  unsigned int *mode);
int  spi_get_max_speed(spi_t    *spi,
                       uint32_t *max_speed);
int  spi_get_bit_order(spi_t           *spi,
                       spi_bit_order_t *bit_order);
int  spi_get_bits_per_word(spi_t   *spi,
                           uint8_t *bits_per_word);
int  spi_get_extra_flags(spi_t   *spi,
                         uint8_t *extra_flags);

/* Setters */
int spi_set_mode(spi_t       *spi,
                 unsigned int mode);
int spi_set_max_speed(spi_t   *spi,
                      uint32_t max_speed);
int spi_set_bit_order(spi_t          *spi,
                      spi_bit_order_t bit_order);
int spi_set_bits_per_word(spi_t  *spi,
                          uint8_t bits_per_word);
int spi_set_extra_flags(spi_t  *spi,
                        uint8_t extra_flags);

/* Miscellaneous */
int spi_fd(spi_t *spi);
int spi_tostring(spi_t *spi,
                 char  *str,
                 size_t len);

/* Error Handling */
int         spi_errno(spi_t *spi);
const char* spi_errmsg(spi_t *spi);

/*********** custom function [wgm-2020-05-11] *************/
/* 保存发送给控制寄存器的数据，用于按指定位单独采集 */
const uint8_t spi_buf_tx[][2] = { 0x83, 0x30,
                                  0x87, 0x30,
                                  0x8b, 0x30,
                                  0x8f, 0x30,
                                  0x93, 0x30,
                                  0x97, 0x30,
                                  0x9b, 0x30,
                                  0x9f, 0x30,
                                  0xa3, 0x30,
                                  0xa7, 0x30 };

/**
 * @brief 用于管理spi对象
 *
 */
struct spi_handler
{
    spi_t  *spi;                 /* 指向spi_new成功的指针 */
    uint8_t buf_rx[2];           /* 用于保存接收到的值，前4位代表地址，后12位代表数据 */
    const   uint8_t(*buf_tx)[2]; /* 指向保存发送给控制寄存器的数据，用于按指定位单独采集 */
    uint8_t val[16];              /* 存储转化后要发给上位机的对应数值 */
};
spi_t* spi_init(const char *path);

# ifdef __cplusplus
}
# endif // ifdef __cplusplus

#endif  // ifndef _PERIPHERY_SPI_H
