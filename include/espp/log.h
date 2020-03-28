#pragma once

#include "freertos/FreeRTOS.h"

#include <cstring>

#include "freertos/FreeRTOS.h"

namespace espp {

class Log{
private:
    static const UBaseType_t _buffer_size = 32;
    char _buffer[_buffer_size];
    char* _last_buffer_char;

public:
    static void Init();

    static void LogBuffer(const char* buffer, std::size_t size);

    static void PutSplitter();

    static void PutEndOfLine();

    static Log& get_instance();

    Log();

    ~Log();

    void Attach();

    void PutInt(int src);

    void PutUInt(unsigned int src);
};

class LogLine{
private:
    static const char HEX_PREFIX[];
    Log& log;

public:
    explicit LogLine():
        log(Log::get_instance())
    {
    }

    ~LogLine()
    {
        Log::PutEndOfLine();
    }

    const LogLine& operator<<(char obj) const
    {
        Log::LogBuffer(&obj, 1);
        Log::PutSplitter();
        return *this;
    }

    const LogLine& operator<<(const char* obj) const
    {
        Log::LogBuffer(obj, strlen(obj));
        Log::PutSplitter();
        return *this;
    }

    const LogLine& operator<<(int obj) const
    {
        log.PutInt(obj);
        return *this;
    }

    const LogLine& operator<<(unsigned int obj) const
    {
        log.PutUInt(obj);
        return *this;
    }

    const LogLine& operator<<(const void* obj) const
    {
        espp::Log::LogBuffer(HEX_PREFIX, 2);
        const auto value = reinterpret_cast<uintptr_t>(obj);
        log.PutUInt(value);
        return *this;
    }
};

}


#define LOG(level) espp::LogLine() << #level
#define DEBUG LOG("D")
#define INFO LOG("I")
#define ERROR LOG("E")
