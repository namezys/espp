#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace espp {

class CriticalSection {
public:
    CriticalSection()
    {
        taskENTER_CRITICAL();
    }

    ~CriticalSection()
    {
        taskEXIT_CRITICAL();
    }
};

}