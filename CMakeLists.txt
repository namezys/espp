#include($ENV{IDF_PATH}/tools/cmake/project.cmake)

idf_component_register(SRCS
        include/espp/lts.h
        include/espp/buffer.h
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
        include/espp/utils/macros.h
        include/espp/wifi.h wifi.cpp
        include/espp/web_server.h include/espp/web_server_html_template.h
        include/espp/drivers/led_ws8212.h drivers/led_ws8212.cpp
        include/espp/utils/boot_counter.h utils/boot_counter.cpp
        include/espp/utils/nvs_settings.h utils/nvs_settings.cpp
        test_include.cpp
        INCLUDE_DIRS "include"
        REQUIRES tcpip_adapter mqtt esp_http_server spi_flash nvs_flash
        )
