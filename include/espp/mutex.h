#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "espp/utils/macros.h"



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

template<TickType_t blockTime = pdMS_TO_TICKS(1000)>
class Mutex {
private:
    const SemaphoreHandle_t _handle;

public:
    using LockGuard = ::espp::LockGuard<Mutex<blockTime>>;

    Mutex(): _handle(xSemaphoreCreateMutex())
    {
    }

    void Lock()
    {
        VERBOSE << "LOCK MUTEX" << _handle;
        ESPP_CHECK(xSemaphoreTake(_handle, blockTime) == pdPASS);
        VERBOSE << "LOCKED";
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
        VERBOSE << "UNLOCK MUTEX" << _handle;
    }
};

}