#pragma once

#include "freertos/FreeRTOS.h"

#include <string>
#include <cstring>

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

    const Log& operator<<(const uint8_t* obj) const
    {
        return *this << reinterpret_cast<const char*>(obj);
    }

    const Log& operator<<(const std::string& obj) const
    {
        return *this << obj.data();
    }
};

}

#define LOG(level) espp::Log() << #level << __FILE__ << __LINE__
#define DEBUG LOG(DEBUG)
#define INFO LOG(INFO)
#define ERROR LOG(ERROR)
