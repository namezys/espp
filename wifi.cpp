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
    Mutex::LockGuard lock(_mutex);
    ESPP_ASSERT(isInited());
    ESPP_ASSERT(!isOccupied());

    DEBUG << "Init WiFi station";
    wifi_config_t station_config = {};
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &station_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    DEBUG << "Init scan";
    ESP_ERROR_CHECK(esp_wifi_scan_start(&config, false));

    _is_scan_done = false;
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
    Mutex::LockGuard lock(_mutex);
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
    Mutex::LockGuard lock(_mutex);

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

void WiFi::ConnectStation(const std::string &ssid, const std::string &password)
{
    INFO << "Connect to" << ssid << "using password with length" << password.size();
    Mutex::LockGuard lock(_mutex);

    ESPP_ASSERT(isInited());
    ESPP_ASSERT(!isOccupied());

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

    _is_station_started = true;
    DEBUG << "Station inited. Wait events: start, started";
}

bool WiFi::DisconnectStation()
{
    INFO << "Disconnect";
    Mutex::LockGuard lock(_mutex);
    if( !_is_station_started) {
        DEBUG << "No connection";
        return false;
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
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
}


esp_err_t WiFi::_EventHandler(void* ctx, system_event_t* event)
{
    WiFi* impl = reinterpret_cast<WiFi*>(ctx);
    impl->_ProcessEvent(*event);
    return ESP_OK;
}

void WiFi::_ProcessEvent(system_event_t& event)
{
    switch(static_cast<int>(event.event_id)) {
        case SYSTEM_EVENT_SCAN_DONE:
            _is_scan_done = true;
            OnScanDone();
            break;

        case SYSTEM_EVENT_STA_START:
            _StationStarted();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            _StationConnected();
            OnConnected();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            _StationDisconnected();
            break;
        case SYSTEM_EVENT_STA_STOP:
            _StationStopped();
            OnDisconnected();
            break;
        default:
            ;
    }
}

void WiFi::_StationStarted()
{
    Mutex::LockGuard lock(_mutex);
    DEBUG << "Station started";
    ESPP_CHECK((_is_scan || _is_station_started) && !_is_station_connected);

    if(_is_station_started) {
        DEBUG << "Connect";
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else {
        DEBUG <<  "No station is inited";
    }
}

void WiFi::_StationConnected()
{
    Mutex::LockGuard lock(_mutex);
    DEBUG << "Station connected";
    ESPP_CHECK(_is_station_started && !_is_station_connected);

    if(_is_station_started) {
        _is_station_connected = true;
    } else {
        DEBUG << "No station inited";
    }
}

void WiFi::_StationDisconnected()
{
    Mutex::LockGuard lock(_mutex);
    DEBUG << "Station disconnected";
    ESPP_CHECK(_is_station_started);

    if(_is_station_started) {
        _is_station_connected = false;
        esp_wifi_stop();
    } else {
        DEBUG << "No station inited";
    }
}

void WiFi::_StationStopped()
{
    Mutex::LockGuard lock(_mutex);
    DEBUG << "Station stopped";
    ESPP_CHECK(_is_station_started || _is_scan);

    if (_is_station_started) {
        _is_station_started = false;
    } else {
        DEBUG << "No station inited";
    }
}


}
