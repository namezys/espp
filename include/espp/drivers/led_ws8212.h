#pragma once

#include "freertos/FreeRTOS.h"

#include "espp/gpio.h"
#include "espp/utils/test.h"

#include <array>

namespace espp {
void IRAM_ATTR SendLed(uint32_t mask, uint32_t* buffer, uint32_t size);

struct LedColor{
    uint8_t green;
    uint8_t red;
    uint8_t blue;

    LedColor():
        green(0), red(0), blue(0)
    {}

    LedColor(uint8_t R, uint8_t G, uint8_t B):
        green(G), red(R), blue(B)
    {}

    static
    uint8_t _Truncate(int32_t c)
    {
        return std::max(std::min(c, 0xFF), 0);
    }

    template<class T>
    LedColor& operator=(const T& other)
    {
        green = other.green;
        red = other.red;
        blue = other.blue;
        return *this;
    }
};

inline
LedColor operator*(const LedColor& color, uint32_t c)
{
    assert(c < 0xFFu);
    return {
        static_cast<uint8_t>(color.red * c / 0xFFu),
        static_cast<uint8_t>(color.green * c / 0xFFu),
        static_cast<uint8_t>(color.blue * c / 0xFFu)
    };
}

inline
LedColor operator*(uint32_t c, const LedColor& color)
{
    return (color * c);
}

inline
LedColor operator+(const LedColor& color, int32_t c)
{
    return {
        LedColor::_Truncate(color.red + c),
        LedColor::_Truncate(color.green + c),
        LedColor::_Truncate(color.blue + c),
    };
}

inline
LedColor operator-(const LedColor& color, int32_t c)
{
    return color + (-c);
}

template<std::size_t length>
class LedStrip{
public:
    explicit
    LedStrip(GpioPin& pin): _gpio_pin(pin), _buffer(), _colors(reinterpret_cast<uint8_t*>(_buffer.data()))
    {
        reset();
    }

    UBaseType_t size() const
    {
        return length;
    }

    inline __attribute__((always_inline))
    LedColor& operator[](UBaseType_t idx)
    {
        return *reinterpret_cast<LedColor*>(_colors + 3 * idx);
    }

    inline __attribute__((always_inline))
    const LedColor operator[](UBaseType_t idx) const
    {
        return *reinterpret_cast<const LedColor*>(_colors + 3 * idx);
    }

    void reset()
    {
        _buffer.fill(0);
    }

    void send()
    {
        SendLed(_gpio_pin.mask(), ledBuffer(), ledBufferSize());
    }


private:
    const static std::size_t _bufferSize = (3 * length + 3) / 4;
    GpioPin _gpio_pin;
    std::array<uint32_t, _bufferSize> _buffer;
    uint8_t* const _colors;

    uint32_t* ledBuffer()
    {
        return _buffer.data();
    }

    UBaseType_t ledBufferSize()
    {
        return _bufferSize;
    }

};

#ifdef ENABLE_TEST
    namespace testing {
        void testWaitConstResult(uint32_t constDelays[20]);
        void testWaitVarResult(uint32_t vars[20]);
        uint32_t testPut32BitsResult();
        uint32_t testSendDataResult();
    }
#endif

}

