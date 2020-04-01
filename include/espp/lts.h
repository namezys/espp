#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace espp {

/**
 * LTS IDS which is used by espp
 *
 * The first available LTS is USER
 */
enum class LTS{
    TASK = 0,
    USER,
    USER_0 = USER,
    USER_1,
    USER_2
};

/**
 * Template to get/set data to LTS
 * @tparam Data Type of data (should be same size as void*)
 * @tparam index (index in LTS)
 */
template<class Data, LTS index>
class Lts {
public:
    static_assert(sizeof(Data) == sizeof(void*), "Data must be same size as void*");

    static inline
    void SetData(Data data)
    {
        vTaskSetThreadLocalStoragePointer(nullptr, static_cast<BaseType_t>(index), reinterpret_cast<void*>(data));
    }

    static inline
    Data data(TaskHandle_t taskHandle = nullptr)
    {
        return reinterpret_cast<Data>(pvTaskGetThreadLocalStoragePointer(taskHandle, static_cast<BaseType_t>(index)));
    }
};

}