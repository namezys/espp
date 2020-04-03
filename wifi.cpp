#include "espp/wifi.h"
#include "espp/critical_section.h"

#include "espp/log.h"

#include <cstring>
#include <esp_event_loop.h>

namespace espp {

WiFi::~WiFi()
{
    if(_is_inited) {
        INFO << "DeInit WiFi";
        ESP_ERROR_CHECK(esp_wifi_deinit());
        _Init(nullptr);
        assert(!IsOccupied());
    }
}

bool WiFi::_Init(void* ptr)
{
    static void* current = nullptr;
    CriticalSection critical;
    if(ptr == nullptr) {
        assert(current != nullptr);
        current = nullptr;
        esp_event_loop_set_cb(nullptr, nullptr);
        return true;
    }

    if(current != nullptr) {
        return false;
    }

    current = ptr;
    return true;
}

void WiFi::_InitSystem(wifi_init_config_t& config)
{
    DEBUG << "Init TCP/IP adapter";
    tcpip_adapter_init();

    DEBUG << "Init WiFi";
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    _is_inited = true;
}

void WiFi::StartScan(const wifi_scan_config_t& config)
{
    assert(IsInited() && !IsOccupied());
    INFO << "Start WiFi scan";

    DEBUG << "Init WiFi station";
    wifi_config_t station_config = {};
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &station_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    DEBUG << "Init scan";
    ESP_ERROR_CHECK(esp_wifi_scan_start(&config, false));

    _is_scan = true;
    DEBUG << "Scan started";
}

void WiFi::StopScan()
{
    assert(_is_scan);
    INFO << "Stop scan";

    ESP_ERROR_CHECK(esp_wifi_scan_stop());
    _scan_result.clear();
    _is_scan = false;
}

void WiFi::Connect(const std::string& ssid, const std::string& password)
{
    assert(IsInited() && !IsOccupied());
    INFO << "Connect to" << ssid << "using password with length" << password.size();
    wifi_config_t config = {};
    assert(ssid.length() < 32);
    std::strncpy(reinterpret_cast<char*>(config.sta.ssid), ssid.data(), 32);
    if (!password.empty()) {
        assert(password.length() < 64);
        std::strncpy(reinterpret_cast<char*>(config.sta.password), password.data(), 64);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    _is_station = true;

    DEBUG << "Connect started";
}
void WiFi::Disconnect()
{
    assert(_is_station);
    INFO << "Disconnect";

    _is_station = false;
}

void WiFi::_FinishScan()
{
    DEBUG << "Save scan result";
    assert(_is_scan);
    uint16_t count;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&count));
    _scan_result.resize(count);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&count, _scan_result.data()));

    OnScanDone();
}

void WiFi::_StationStarted()
{
    assert(_is_scan || _is_station);

    if (_is_station) {
        DEBUG << "Station started. Connect";
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else {
        DEBUG << "Station for scan started";
    }
}

}
