#include "espp/wifi.h"

#include "espp/critical_section.h"
#include "espp/log.h"
#include "espp/utils/macros.h"

#include <cstring>
#include <esp_event_loop.h>

namespace espp {

namespace {

bool _SetWiFiSingleton(bool init)
{
    static bool is_inited = false;
    if (init == is_inited) {
        return false;
    }
    is_inited = init;
    return true;
}

}

WiFi::~WiFi()
{
    Mutex::LockGuard lock(_mutex);
    if(isInited()) {
        INFO << "DeInit WiFi";
        ESP_ERROR_CHECK(esp_wifi_deinit());
        ESPP_CHECK(_SetWiFiSingleton(false));
        ESPP_CHECK(_EventHandler == esp_event_loop_set_cb(nullptr, nullptr));
    }
}

void WiFi::Init()
{
    _Init();
    OnReady();
}

void WiFi::_Init()
{
    INFO << "Init WiFi system";
    Mutex::LockGuard lock(_mutex);
    assert(!isInited());
    ESPP_CHECK(_SetWiFiSingleton(true));
    const auto init_loop_result = esp_event_loop_init(_EventHandler, this);
    if(init_loop_result == ESP_FAIL) {
        DEBUG << "Loop was inited early. Set event handler";
        ESPP_CHECK(nullptr == esp_event_loop_set_cb(_EventHandler, this));
    } else {
        DEBUG << "Loop was inited";
    }
    DEBUG << "Init TCP/IP adapter";
    tcpip_adapter_init();

    DEBUG << "Init WiFi";
    ESP_ERROR_CHECK(esp_wifi_init(&_config));
    _is_inited = true;
    DEBUG << "WiFi ready";
}

void WiFi::StartScan(const wifi_scan_config_t& config)
{
    INFO << "Start WiFi scan";
    espp::Mutex::LockGuard lock(_mutex);
    assert(isInited() && !isOccupied());

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

void WiFi::StartScan()
{
    wifi_scan_config_t config = {nullptr,nullptr,0,true,
                                 WIFI_SCAN_TYPE_ACTIVE, {}};
    config.scan_time.active.min = 500;
    config.scan_time.active.max = 1000;
    StartScan(config);
}

WiFi::ScanResult WiFi::scanResult()
{
    DEBUG << "Get scan result";
    espp::Mutex::LockGuard lock(_mutex);
    if (!_is_scan) {
        DEBUG << "No scan";
        return {};
    }
    uint16_t count;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&count));
    _scan_result.resize(count);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&count, _scan_result.data()));
    return _scan_result;
}

bool WiFi::StopScan()
{
    INFO << "Stop scan";
    espp::Mutex::LockGuard lock(_mutex);

    if(!_is_scan) {
        DEBUG << "No scan";
        return false;
    }
    ESP_ERROR_CHECK(esp_wifi_scan_stop());
    ESP_ERROR_CHECK(esp_wifi_stop());
    _scan_result.clear();
    _is_scan = false;
    DEBUG << "Scan stopped";
    return true;
}

void WiFi::Connect(const std::string& ssid, const std::string& password)
{
    INFO << "Connect to" << ssid << "using password with length" << password.size();
    espp::Mutex::LockGuard lock(_mutex);

    assert(isInited() && !isOccupied());
    wifi_config_t config = {};
    assert(ssid.length() < 32);
    std::strncpy(reinterpret_cast<char*>(config.sta.ssid), ssid.data(), 32);
    if(!password.empty()) {
        assert(password.length() < 64);
        std::strncpy(reinterpret_cast<char*>(config.sta.password), password.data(), 64);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config));
    ESP_ERROR_CHECK(esp_wifi_start());

    _is_station = true;
    DEBUG << "Connect started";
}

bool WiFi::Disconnect()
{
    INFO << "Disconnect";
    espp::Mutex::LockGuard lock(_mutex);
    if( !_is_station) {
        DEBUG << "No connection";
        return false;
    }

    ESP_ERROR_CHECK(esp_wifi_stop());

    _is_station = false;
    return true;
}

void WiFi::OnReady()
{
}

void WiFi::OnScanDone()
{
    StopScan();
}

void WiFi::OnConnected()
{
}

void WiFi::OnDisconnected()
{
    Disconnect();
}


esp_err_t WiFi::_EventHandler(void* ctx, system_event_t* event)
{
    WiFi* impl = reinterpret_cast<WiFi*>(ctx);
    impl->_ProcessEvent(*event);
    return ESP_OK;
}

void WiFi::_ProcessEvent(system_event_t& event)
{
    switch(event.event_id) {
        case SYSTEM_EVENT_SCAN_DONE:
            OnScanDone();
            break;
        case SYSTEM_EVENT_STA_START:
            _StationStarted();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            OnConnected();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            _StationStopped();
            break;
        default:
            ;
    }
}

void WiFi::_StationStarted()
{
    Mutex::LockGuard lock(_mutex);
    assert(_is_scan || _is_station);

    if(_is_station) {
        DEBUG << "Station started. Connect";
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else {
        DEBUG <<  "No station is inited";
    }
}

void WiFi::_StationStopped()
{
    Mutex::LockGuard lock(_mutex);
    assert(_is_scan || _is_station);

    if(_is_station) {
        DEBUG << "Station stopped. Connect";
        ESP_ERROR_CHECK(esp_wifi_stop())
        OnDisconnected();
    } else {
        DEBUG <<  "No station is inited";
    }
}

}
