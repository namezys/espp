#pragma once

#define GET_CYCLE_COUNT(cycleCount) __asm__ __volatile__("rsr     %0, ccount":"=a" (cycleCount));
#define DECLARE_CYCLE_COUNT_VAR(varName) uint32_t varName; GET_CYCLE_COUNT(varName)
#define NOP __asm__ __volatile__("nop;");
