#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace espp {

template<class Lockable>
class LockGuard {
private:
    Lockable& _lockable;

public:
    explicit
    LockGuard(Lockable& lockable): _lockable(lockable)
    {
        _lockable.Lock();
    }

    ~LockGuard()
    {
        _lockable.Unlock();
    }
};

class Mutex {
private:
    const SemaphoreHandle_t _handle;

public:
    using LockGuard = ::espp::LockGuard<Mutex>;

    Mutex(): _handle(xSemaphoreCreateMutex())
    {
    }

    void Lock()
    {
        xSemaphoreTake(_handle, portMAX_DELAY);
    }

    inline
    bool TryLock(TickType_t ticKNumber)
    {
        return pdTRUE == xSemaphoreTake(_handle, ticKNumber);
    }

    inline
    bool TryLockMs(uint32_t ms)
    {
        return TryLock(pdMS_TO_TICKS(ms));
    }

    inline
    void Unlock()
    {
        xSemaphoreGive(_handle);
    }
};


}