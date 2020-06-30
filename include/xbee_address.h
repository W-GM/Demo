#ifndef __XBEE_ADDRESS_H__
#define __XBEE_ADDRESS_H__

#include <stdint.h>

/**
 * C ++ 11引入了constexpr，以向编译器提示可以在编译时评估事物。
 * 这可以帮助删除全局对象的启动代码，或者以其他方式帮助编译器进行优化。
 * 由于关键字是C ++ 11中引入的，所以支持较早的编译器只需删除关键字即可，为此，我们使用了宏。
 */
#if __cplusplus >= 201103L
# define CONSTEXPR constexpr
#else // if __cplusplus >= 201103L
# define CONSTEXPR
#endif // if __cplusplus >= 201103L

class XBeeAddress {
public:

    CONSTEXPR XBeeAddress() {}
};

/**
 * 表示64位XBee地址
 *
 * 请注意，从4.9版开始的avr-gcc不能很好地优化uint64_t，
 * 因此对于最小和最快的代码，请分别使用msb和lsb。
 * 参见https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66511
 */
class XBeeAddress64 : public XBeeAddress {
public:

    CONSTEXPR XBeeAddress64(uint64_t addr) : _msb(addr >> 32), _lsb(addr) {}

    CONSTEXPR XBeeAddress64(uint32_t msb, uint32_t lsb) : _msb(msb), _lsb(lsb) {}

    CONSTEXPR XBeeAddress64() : _msb(0), _lsb(0) {}

    uint32_t getMsb()
    {
        return _msb;
    }

    uint32_t getLsb()
    {
        return _lsb;
    }

    uint64_t get()
    {
        return (static_cast<uint64_t>(_msb) << 32) | _lsb;
    }

    operator uint64_t()
    {
        return get();
    }
    void setMsb(uint32_t msb)
    {
        _msb = msb;
    }

    void setLsb(uint32_t lsb)
    {
        _lsb = lsb;
    }

    void set(uint64_t addr)
    {
        _msb = addr >> 32;
        _lsb = addr;
    }

private:

    // 修复https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66511之后，
    // 将它们合并到uint64_t中可能是有意义的。
    uint32_t _msb;
    uint32_t _lsb;
};

#endif // ifndef __XBEE_ADDRESS_H__
