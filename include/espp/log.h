#pragma once

#include "freertos/FreeRTOS.h"

#include <cstring>

#include "freertos/FreeRTOS.h"
#include "espp/lts.h"

namespace espp {

class Log{
private:
    static
    void _OutChar(int ch);

public:
    explicit inline
    Log()
    {
        vPortETSIntrLock();
    }

    inline
    ~Log()
    {
        _OutChar('\n');
        vPortETSIntrUnlock();
    }

    const Log& operator<<(char obj) const;
    const Log& operator<<(const char* obj) const;
    const Log& operator<<(int obj) const;
    const Log& operator<<(unsigned int obj) const;
    const Log& operator<<(const void* obj) const;
};

}

#define LOG(level) espp::Log() << #level
#define DEBUG LOG(DEBUG)
#define INFO LOG(INFO)
#define ERROR LOG(ERROR)
