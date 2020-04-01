#include "espp/log.h"

#include "driver/uart.h"

#include "espp/lts.h"

#include "esp_libc.h"

namespace espp {

namespace {
const UBaseType_t _buffer_size = 32;
char _buffer[_buffer_size] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    ' ', ' '
};
char* _last_buffer_char = _buffer + _buffer_size - 1;

const char ZERO_STR[] = "0 ";
const char HEX_PREFIX[] = "0x";

inline
void _OutChar(int ch)
{
    ets_putc(ch);
}

inline
void _OutBuffer(const char *buffer, std::size_t size)
{
    for(const auto until_ptr = buffer + size; buffer != until_ptr; ++buffer) {
        _OutChar(*buffer);
    }
}

inline
void _OutUInt(unsigned int src)
{
    if(src == 0) {
        _OutBuffer(ZERO_STR, 2);
        return;
    }
    char* current_character = _last_buffer_char;
    while(src != 0) {
        current_character -= 1;
        *current_character = static_cast<char>('0' + src % 10);
        src /= 10;
    }
    _OutBuffer(current_character, _last_buffer_char - current_character + 1);
}

inline
void _OutInt(int src)
{
    if(src < 0) {
        _OutChar('-');
        src = -src;
    }
    _OutUInt(src);
}

}

void Log::_OutChar(int ch)
{
    ::espp::_OutChar(ch);
}


const Log& Log::operator<<(char obj) const
{
    _OutChar(obj);
    _OutChar(' ');
    return *this;
}

const Log& Log::operator<<(const char* obj) const
{
    for(;*obj != 0; ++obj) {
        _OutChar(*obj);
    }
    _OutChar(' ');
    return *this;
}

const Log& Log::operator<<(int obj) const
{
    _OutInt(obj);
    return *this;
}

const Log& Log::operator<<(unsigned int obj) const
{
    _OutUInt(obj);
    return *this;
}

const Log& Log::operator<<(const void* obj) const
{
    _OutBuffer(HEX_PREFIX, 2);
    const auto value = reinterpret_cast<uintptr_t>(obj);
    _OutUInt(value);
    return *this;
}

}
