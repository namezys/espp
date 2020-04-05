#include($ENV{IDF_PATH}/tools/cmake/project.cmake)

idf_component_register(SRCS
        include/espp/lts.h
        include/espp/log.h log.cpp
        include/espp/task.h
        include/espp/gpio.h
        include/espp/mutex.h
        include/espp/critical_section.h
        include/espp/mqtt.h mqtt.cpp
        include/espp/protobuf.h
        include/espp/utils/low_level.h
        include/espp/utils/profile.h
        include/espp/utils/test.h
        include/espp/wifi.h wifi.cpp
        include/espp/drivers/led_ws8212.h drivers/led_ws8212.cpp
        test_include.cpp
        INCLUDE_DIRS "include"
        REQUIRES tcpip_adapter mqtt
        )
