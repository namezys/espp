#pragma once

#include "freertos/FreeRTOS.h"

#include "espp/gpio.h"
#include "espp/utils/test.h"

#include <array>

namespace espp {
void IRAM_ATTR SendLed(uint32_t mask, uint32_t* buffer, uint32_t size);

template<std::size_t length>
class LedStrip{
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

public:
    struct LedColor{
        uint8_t green;
        uint8_t red;
        uint8_t blue;
    };

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

