#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string>
#include <cstring>

#include "espp/lts.h"

#define LOG_ENABLED true
#define LOG_TASK true
#define LOG_TIME true
#define LOG_VERBOSE false

namespace espp {

class Log{
private:
    static
    void _OutChar(int ch);


public:
    explicit inline
    Log()
    {
        TaskStatus_t status;
        vTaskGetInfo(nullptr, &status, pdFAIL, eInvalid);
        vPortETSIntrLock();
        if (LOG_TASK) {
            *this << "TASK" << status.xTaskNumber;
            if(status.pcTaskName) {
                *this << status.pcTaskName;
            }
        }
        if (LOG_TIME) {
            *this << xTaskGetTickCount();
        }
    }

    inline
    ~Log()
    {
        _OutChar('\n');
        vPortETSIntrUnlock();
    }

    template<class T>
    void hex(const T& v)
    {

    }

    const Log& operator<<(char obj) const;
    const Log& operator<<(uint8_t obj) const
    {
        return *this << static_cast<unsigned int>(obj);
    }
    const Log& operator<<(unsigned long obj) const
    {
        return *this << static_cast<unsigned int>(obj);
    }

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


#define ROW_LOG espp::Log()
#define LOG(level) if(LOG_ENABLED) ROW_LOG << #level << __FILE__ << __LINE__
#define VERBOSE if (LOG_VERBOSE) ROW_LOG
#define DEBUG LOG(DEBUG)
#define INFO LOG(INFO)
#define ERROR LOG(ERROR)
