#pragma once

#include "freertos/FreeRTOS.h"

#include "esp8266/gpio_struct.h"
#include "driver/gpio.h"

#include "espp/log.h"

namespace espp {

/**
 * Save current state of GPIO pin. Used for debugging.
 */
struct GpioState{
    gpio_num_t pin;     ///< pin number
    char level;         ///< H in case of high level or L in case of low
};

inline const Log& operator<<(const Log& log, const GpioState& state)
{
    return log << "gpio" << state.pin << "level" << state.level;
}

/**
 * Define GPIO pin
 */
class GpioPin{
private:
    const gpio_num_t _pin_num;
    const uint32_t _pin_mask;

public:
    explicit
    GpioPin(gpio_num_t pin_num):
        _pin_num(pin_num),
        _pin_mask(0x1u << _pin_num)
    {
        assert(GPIO_IS_VALID_GPIO(_pin_num) && !RTC_GPIO_IS_VALID_GPIO(_pin_num));
    }

    gpio_num_t pin() const
    {
        return _pin_num;
    }

    uint32_t mask() const
    {
        return _pin_mask;
    }

    bool HasLowLevel()
    {
        return gpio_get_level(_pin_num) == 0;
    }

    bool HasHighLevel()
    {
        return !HasLowLevel();
    }

    void SetLowLevel() const __attribute__((always_inline))
    {
        GPIO.out_w1tc = _pin_mask;
    }

    void SetHighLevel() const __attribute__((always_inline))
    {
        GPIO.out_w1ts = _pin_mask;
    }

    void Disable()
    {
        gpio_set_direction(_pin_num, GPIO_MODE_DISABLE);
    }

    void EnableInput()
    {
        gpio_set_direction(_pin_num, GPIO_MODE_INPUT);
    }

    void EnableOutput()
    {
        gpio_set_direction(_pin_num, GPIO_MODE_OUTPUT);
    }

    void EnablePullUpOnly()
    {
        gpio_set_pull_mode(_pin_num, GPIO_PULLUP_ONLY);
    }

    void EnablePullDownOnly()
    {
        gpio_set_pull_mode(_pin_num, GPIO_PULLDOWN_ONLY);
    }

    GpioState state()
    {
        return {.pin = _pin_num, .level = HasLowLevel() ? 'L' : 'H'};
    }
};

/**
 * Base class for interruption
 */
class GpioInterrupt{
protected:
    GpioPin& _pin;

public:
    GpioInterrupt(GpioPin& pin):
            _pin(pin)
    {
    }

    GpioPin& pin()
    {
        return _pin;
    }
};

/**
 * Register interrupt handler with method run.
 *
 * @tparam Handler class derived from GpioInterrupt
 * @tparam autoReg true if constructor has to register interrupt
 */
template<class Handler, gpio_int_type_t intType = GPIO_INTR_DISABLE, bool autoReg = false>
class GpioInterruptRegister{
protected:
    bool _registered = false;
    Handler _handler;

public:
    GpioInterruptRegister(GpioPin& pin):
            _handler(pin)
    {
        if(autoReg) {
            Register();
        }
    }

    ~GpioInterruptRegister()
    {
        if(_registered || autoReg) {
            Unregister();
        }
    }

    void Register()
    {
        assert(!_registered);
        gpio_isr_handler_add(_handler.pin(), GpioInterruptRegister::handler, reinterpret_cast<void*>(&_handler));
    }

    void Unregister()
    {
        assert(_registered);
        gpio_isr_handler_remove(_handler.pin());
    }

    static void handler(void* arg)
    {
        Handler* handler = reinterpret_cast<Handler*>(arg);
        handler->run();
    }
};

template<class Handler, bool autoReg = false>
class GpioRisingEdgeInterruptRegister: public GpioInterruptRegister<Handler, GPIO_INTR_POSEDGE, autoReg>{
public:
    using GpioInterruptRegister<Handler, GPIO_INTR_POSEDGE, autoReg>::GpioInterruptRegister;
};

template<class Handler, bool autoReg = false>
class GpioFallingEdgeInterruptRegister: public GpioInterruptRegister<Handler, GPIO_INTR_NEGEDGE, autoReg>{
public:
    using GpioInterruptRegister<Handler, GPIO_INTR_NEGEDGE, autoReg>::GpioInterruptRegister;
};

template<class Handler, bool autoReg = false>
class GpioAnyEdgeInterruptRegister: public GpioInterruptRegister<Handler, GPIO_INTR_ANYEDGE, autoReg>{
public:
    using GpioInterruptRegister<Handler, GPIO_INTR_ANYEDGE, autoReg>::GpioInterruptRegister;
};

template<class Handler, bool autoReg = false>
class GpioLowLevelInterruptRegister: public GpioInterruptRegister<Handler, GPIO_INTR_LOW_LEVEL, autoReg>{
public:
    using GpioInterruptRegister<Handler, GPIO_INTR_LOW_LEVEL, autoReg>::GpioInterruptRegister;
};

template<class Handler, bool autoReg = false>
class GpioHighLevelInterruptRegister: public GpioInterruptRegister<Handler, GPIO_INTR_HIGH_LEVEL, autoReg>{
public:
    using GpioInterruptRegister<Handler, GPIO_INTR_HIGH_LEVEL, autoReg>::GpioInterruptRegister;
};

}