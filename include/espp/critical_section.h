#pragma once

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