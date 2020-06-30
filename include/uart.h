#ifndef __UART_H__
#define __UART_H__

#include <string>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

using namespace std;

enum serial_error_code
{
    SERIAL_ERROR_ARG       = -1, /* Invalid arguments */
    SERIAL_ERROR_OPEN      = -2, /* Opening serial port */
    SERIAL_ERROR_QUERY     = -3, /* Querying serial port attributes */
    SERIAL_ERROR_CONFIGURE = -4, /* Configuring serial port attributes */
    SERIAL_ERROR_IO        = -5, /* Reading/writing serial port */
    SERIAL_ERROR_CLOSE     = -6, /* Closing serial port */
};

typedef enum serial_parity
{
    PARITY_NONE,
    PARITY_ODD,
    PARITY_EVEN,
} serial_parity_t;

// 异常
class Error_t
{
    public:
    int  c_errno;
    char errmsg[96];
};

class uart {
    int _port_fd;

    Error_t _error;

public:

    uart();

    int Open(
        const char *path,
        uint32_t    baudrate);
    int Read(
        uint8_t *buf,
        size_t   len,
        int      timeout_ms);
    int Write(
        const uint8_t *buf,
        size_t         len);
    int Flush();
    int input_waiting(
        unsigned int *count);
    int output_waiting(
        unsigned int *count);
    int Poll(
        int timeout_ms);
    int Close();

    int error(
        int         code,
        int         c_errno,
        const char *fmt,
        ...);

    int open_advanced(
        const char     *path,
        uint32_t        baudrate,
        unsigned int    databits,
        serial_parity_t parity,
        unsigned int    stopbits,
        bool            xonxoff,
        bool            rtscts);

    /* Getters */
    int get_baudrate(
        uint32_t *baudrate);
    int get_databits(
        unsigned int *databits);
    int get_parity(
        serial_parity_t *parity);
    int get_stopbits(
        unsigned int *stopbits);
    int get_xonxoff(
        bool *xonxoff);
    int get_rtscts(
        bool *rtscts);

    /* Setters */
    int set_baudrate(
        uint32_t baudrate);
    int set_databits(
        unsigned int databits);
    int set_parity(
        enum serial_parity parity);
    int set_stopbits(
        unsigned int stopbits);
    int set_xonxoff(
        bool enabled);
    int set_rtscts(
        bool enabled);

    /* Miscellaneous */
    int fd();
    int uartostring(
        char  *str,
        size_t len);

    /* Error Handling */
    int         get_errno();
    const char* errmsg();
};

#endif // ifndef __UART_H__
