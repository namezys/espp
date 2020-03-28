#pragma once

#include "esp8266/eagle_soc.h"

#define PROFILE(statement) {                                   \
    const auto start = soc_get_ccount();                       \
    statement;                                                 \
    const auto end = soc_get_ccount();                         \
    DEBUG << "cycles: " << end - start << "for" << #statement; \
}
