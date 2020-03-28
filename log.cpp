#include "espp/log.h"

#include "driver/uart.h"
#include "freertos/task.h"

#include "espp/lts.h"

namespace espp {

namespace {
const uart_port_t logPort = static_cast<uart_port_t>(CONFIG_CONSOLE_UART_NUM);

const char SPACE = ' ';
const char NEW_LINE = '\n';
const char ZERO_STR[] = "0 ";
}

const char LogLine::HEX_PREFIX[] = "0x";

void Log::Init()
{
    static bool is_uart_inited = false;
    if(!is_uart_inited) {
        uart_config_t uart_config = {
            .baud_rate = 74880,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0,
            };
        uart_param_config(logPort, &uart_config);
        uart_driver_install(logPort, 1024 * 2, 0, 0, nullptr, 0);
        is_uart_inited = true;
    }
}

void Log::LogBuffer(const char *buffer, std::size_t size)
{
     uart_write_bytes(logPort, buffer, size);
}

void Log::PutSplitter()
{
    LogBuffer(&SPACE, 1);
}

void Log::PutEndOfLine()
{
    LogBuffer(&NEW_LINE, 1);
}

Log& Log::get_instance()
{
    return *reinterpret_cast<Log*>(pvTaskGetThreadLocalStoragePointer(nullptr, 0));
}

Log::Log(): _last_buffer_char(_buffer + _buffer_size - 1)
{
    *_last_buffer_char = ' ';
}

Log::~Log()
{
    if(&get_instance() == this) {
        vTaskSetThreadLocalStoragePointer(nullptr, static_cast<BaseType_t>(LTS::LOG), nullptr);
    }
}

void Log::Attach()
{
    vTaskSetThreadLocalStoragePointer(nullptr, static_cast<BaseType_t>(LTS::LOG), this);
}

void Log::PutInt(int src)
{
    if(src == 0) {
        LogBuffer(ZERO_STR, 2);
        return;
    }
    bool sign_flag = false;
    if(src < 0) {
        sign_flag = true;
        src = -src;
    }
    char* current_character = _last_buffer_char;
    while(src != 0) {
        current_character -= 1;
        *current_character = static_cast<char>('0' + src % 10);
        src /= 10;
    }
    if(sign_flag) {
        current_character -= 1;
        *current_character = '-';
    }
    LogBuffer(current_character, _last_buffer_char - current_character + 1);
}

void Log::PutUInt(unsigned int src)
{
    if(src == 0) {
        LogBuffer(ZERO_STR, 2);
        return;
    }
    char* current_character = _last_buffer_char;
    while(src != 0) {
        current_character -= 1;
        *current_character = static_cast<char>('0' + src % 10);
        src /= 10;
    }
    LogBuffer(current_character, _last_buffer_char - current_character + 1);
}

}