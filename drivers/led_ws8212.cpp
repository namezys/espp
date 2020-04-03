#include "espp/drivers/led_ws8212.h"
#include "espp/utils/low_level.h"


namespace espp {

namespace {

const uint32_t d = 15;
const uint32_t T0H = 20 - d;
const uint32_t T0L = 57 - d;
const uint32_t T1H = 60 - d;
const uint32_t T1L = 12 - d;
const uint32_t RST_MS = 10;

inline void __attribute__ ((always_inline)) Wait(uint32_t delay)
{
    uint32_t cycleCount;
    uint32_t waitUntil;

    GET_CYCLE_COUNT(waitUntil);
    waitUntil += delay;
    do {
        GET_CYCLE_COUNT(cycleCount);
    } while(waitUntil > cycleCount);
}

inline void __attribute__ ((always_inline)) SendBit(uint32_t pinMask, uint32_t highCycleNumber, uint32_t lowCycleNumber)
{
    GPIO.out_w1ts = pinMask;
    Wait(highCycleNumber);
    GPIO.out_w1tc = pinMask;
    Wait(lowCycleNumber);
}


inline void __attribute__((always_inline)) Send32Bit(uint32_t pinMask, uint32_t value)
{
    uint32_t mask = 0x80;
    for(int i = 0; i < 8; ++i) {
        if(value & mask) {
            SendBit(pinMask, T1H, T1L);
        } else {
            SendBit(pinMask, T0H, T0L);
        }
        mask = mask >> 1u;
    }

    mask = 0x8000;
    for(int i = 0; i < 8; ++i) {
        if(value & mask) {
            SendBit(pinMask, T1H, T1L);
        } else {
            SendBit(pinMask, T0H, T0L);
        }
        mask = mask >> 1u;
    }

    mask = 0x800000;
    for(int i = 0; i < 8; ++i) {
        if(value & mask) {
            SendBit(pinMask, T1H, T1L);
        } else {
            SendBit(pinMask, T0H, T0L);
        }
        mask = mask >> 1u;
    }
    mask = 0x80000000;
    for(int i = 0; i < 8; ++i) {
        if(value & mask) {
            SendBit(pinMask, T1H, T1L);
        } else {
            SendBit(pinMask, T0H, T0L);
        }
        mask = mask >> 1u;
    }
}

inline void __attribute__((always_inline)) SendData(uint32_t mask, uint32_t* buffer, uint32_t size)
{
    const auto lastItem = buffer + size;
    for(auto item = buffer; item != lastItem; ++item) {
        Send32Bit(mask, *item);
    }
}

}

void IRAM_ATTR SendLed(uint32_t mask, uint32_t* buffer, uint32_t size)
{
    GPIO.out_w1tc = mask;
    vPortETSIntrLock();
    SendData(mask, buffer, size);
    GPIO.out_w1tc = mask;
    vPortETSIntrUnlock();
    vTaskDelay(pdMS_TO_TICKS(RST_MS));
}

#ifdef ENABLE_TEST
namespace testing {

#define TEST_WAIT_C_FUN_NAME(c) testWait##c

#define TEST_WAIT_C_FUN(c)                          \
    uint32_t IRAM_ATTR __attribute__((noinline))    \
    TEST_WAIT_C_FUN_NAME(c)()                       \
    {                                               \
        DECLARE_CYCLE_COUNT_VAR(start);             \
        Wait(c);                                    \
        DECLARE_CYCLE_COUNT_VAR(end);               \
        return end - start;                         \
    }

uint32_t IRAM_ATTR __attribute__((noinline)) testWaitVar(uint32_t v)
{
    DECLARE_CYCLE_COUNT_VAR(start);
    Wait(v);
    DECLARE_CYCLE_COUNT_VAR(end);
    return end - start;
}

TEST_WAIT_C_FUN(0)

TEST_WAIT_C_FUN(1)

TEST_WAIT_C_FUN(2)

TEST_WAIT_C_FUN(3)

TEST_WAIT_C_FUN(4)

TEST_WAIT_C_FUN(5)

TEST_WAIT_C_FUN(6)

TEST_WAIT_C_FUN(7)

TEST_WAIT_C_FUN(8)

TEST_WAIT_C_FUN(9)

TEST_WAIT_C_FUN(10)

TEST_WAIT_C_FUN(11)

TEST_WAIT_C_FUN(12)

TEST_WAIT_C_FUN(13)

TEST_WAIT_C_FUN(14)

TEST_WAIT_C_FUN(15)

TEST_WAIT_C_FUN(16)

TEST_WAIT_C_FUN(17)

TEST_WAIT_C_FUN(18)

TEST_WAIT_C_FUN(19)

#define RUN_TEST_WAIT_C(var, c) var[c] = TEST_WAIT_C_FUN_NAME(c)();
#define RUN_TEST_WAIT_V(var, c) var[c] = testWaitVar(c);


void testWaitConstResult(uint32_t constDelays[20])
{
    vPortETSIntrLock();
    RUN_TEST_WAIT_C(constDelays, 0);
    RUN_TEST_WAIT_C(constDelays, 1);
    RUN_TEST_WAIT_C(constDelays, 2);
    RUN_TEST_WAIT_C(constDelays, 3);
    RUN_TEST_WAIT_C(constDelays, 4);
    RUN_TEST_WAIT_C(constDelays, 5);
    RUN_TEST_WAIT_C(constDelays, 6);
    RUN_TEST_WAIT_C(constDelays, 7);
    RUN_TEST_WAIT_C(constDelays, 8);
    RUN_TEST_WAIT_C(constDelays, 9);
    RUN_TEST_WAIT_C(constDelays, 10);
    RUN_TEST_WAIT_C(constDelays, 11);
    RUN_TEST_WAIT_C(constDelays, 12);
    RUN_TEST_WAIT_C(constDelays, 13);
    RUN_TEST_WAIT_C(constDelays, 14);
    RUN_TEST_WAIT_C(constDelays, 15);
    RUN_TEST_WAIT_C(constDelays, 16);
    RUN_TEST_WAIT_C(constDelays, 17);
    RUN_TEST_WAIT_C(constDelays, 18);
    RUN_TEST_WAIT_C(constDelays, 19);
    vPortETSIntrUnlock();
}

void testWaitVarResult(uint32_t vars[20])
{
    vPortETSIntrLock();
    RUN_TEST_WAIT_V(vars, 0);
    RUN_TEST_WAIT_V(vars, 1);
    RUN_TEST_WAIT_V(vars, 2);
    RUN_TEST_WAIT_V(vars, 3);
    RUN_TEST_WAIT_V(vars, 4);
    RUN_TEST_WAIT_V(vars, 5);
    RUN_TEST_WAIT_V(vars, 6);
    RUN_TEST_WAIT_V(vars, 7);
    RUN_TEST_WAIT_V(vars, 8);
    RUN_TEST_WAIT_V(vars, 9);
    RUN_TEST_WAIT_V(vars, 10);
    RUN_TEST_WAIT_V(vars, 11);
    RUN_TEST_WAIT_V(vars, 12);
    RUN_TEST_WAIT_V(vars, 13);
    RUN_TEST_WAIT_V(vars, 14);
    RUN_TEST_WAIT_V(vars, 15);
    RUN_TEST_WAIT_V(vars, 16);
    RUN_TEST_WAIT_V(vars, 17);
    RUN_TEST_WAIT_V(vars, 18);
    RUN_TEST_WAIT_V(vars, 19);
    vPortETSIntrUnlock();
}


uint32_t IRAM_ATTR testGpioSet1()
{
    GPIO.out_w1tc = 0;
    vPortETSIntrLock();
    DECLARE_CYCLE_COUNT_VAR(start);
    GPIO.out_w1tc = 0x2;
    DECLARE_CYCLE_COUNT_VAR(end);
    vPortETSIntrUnlock();
    return end - start;
}

uint32_t IRAM_ATTR testGpioSet2()
{
    GPIO.out_w1tc = 0;
    vPortETSIntrLock();
    DECLARE_CYCLE_COUNT_VAR(start);
    GPIO.out_w1ts = 0x6;
    GPIO.out_w1tc = 0x2;
    DECLARE_CYCLE_COUNT_VAR(end);
    vPortETSIntrUnlock();
    return end - start;
}

uint32_t IRAM_ATTR testGpioSet3()
{
    GPIO.out_w1tc = 0;
    vPortETSIntrLock();
    DECLARE_CYCLE_COUNT_VAR(start);
    GPIO.out_w1ts = 0x6;
    GPIO.out_w1ts = 0x4;
    GPIO.out_w1tc = 0x2;
    DECLARE_CYCLE_COUNT_VAR(end);
    vPortETSIntrUnlock();
    return end - start;
}

uint32_t IRAM_ATTR __attribute__((noinline))
testPut32Bits(uint32_t mask, uint32_t value)
{
    GPIO.out_w1tc = 0;
    Wait(50000);
    DECLARE_CYCLE_COUNT_VAR(start);
    Send32Bit(mask, value);
    DECLARE_CYCLE_COUNT_VAR(end);
    return end - start;
}

uint32_t testPut32BitsResult()
{
    vPortETSIntrLock();
    auto res = testPut32Bits(0x4, 0xFFFFFFFF);
    vPortETSIntrUnlock();
    return res;
}

uint32_t IRAM_ATTR __attribute__((noinline))
testSendData(uint32_t mask, uint32_t values[3])
{
    GPIO.out_w1tc = 0;
    Wait(50000);
    DECLARE_CYCLE_COUNT_VAR(start);
    SendData(mask, values, 3);
    DECLARE_CYCLE_COUNT_VAR(end);
    return end - start;
}

uint32_t testSendDataResult()
{
    uint32_t values[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    vPortETSIntrLock();
    auto res = testSendData(0x4, values);
    vPortETSIntrUnlock();
    return res;
}

}
#endif

}
