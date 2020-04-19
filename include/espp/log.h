#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string>
#include <cstring>

#include "espp/lts.h"

#define LOG_TASK true
#define LOG_TIME true

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

#ifndef LOG_INFO_ENABLED
#define LOG_INFO_ENABLED true
#endif

#ifndef LOG_ERROR_ENABLED
#define LOG_ERROR_ENABLED true
#endif

#ifndef LOG_DEBUG_ENABLED
#define LOG_DEBUG_ENABLED false
#endif

#ifndef LOG_VERBOSE_ENABLED
#define LOG_VERBOSE_ENABLED false
#endif

#define ROW_LOG espp::Log()
//#define LOG(level) if(LOG_ENABLED) ROW_LOG << #level << __FILE__ << __LINE__
#define LOG(level) ROW_LOG << #level

#define ERROR if(LOG_ERROR_ENABLED || LOG_INFO_ENABLED || LOG_DEBUG_ENABLED || LOG_VERBOSE_ENABLED) LOG(ERROR)
#define INFO if(LOG_INFO_ENABLED || LOG_DEBUG_ENABLED || LOG_VERBOSE_ENABLED) LOG(INFO)
#define DEBUG if(LOG_DEBUG_ENABLED || LOG_VERBOSE_ENABLED) LOG(DEBUG)
#define VERBOSE if(LOG_VERBOSE_ENABLED) LOG(VERBOSE)
