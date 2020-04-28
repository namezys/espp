#include <espp/wifi.h>
#include <esp_event_loop.h>

namespace lamp {

WiFi::WiFi()
{
    DEBUG << "Init wifi";
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(WiFi::StaticEventHandler, this));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
}

Data WiFi::accessPointSsid() const
{
    if(!_isAccessPoint) {
        return {};
    }
    wifi_config_t config;
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_AP, &config));
    return {config.ap.ssid, config.ap.ssid_len};
}

Data WiFi::stationSsid() const
{
    if(!_isStation) {
        return {};
    }
    wifi_config_t config;
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &config));
    return {config.sta.ssid};
}

void WiFi::SetAccessPoint(const Buffer& ssid)
{
    INFO << "Set access point: SSID" << ssid;
    Mutex::LockGuard lock(_mutex);
    ESPP_CHECK(_state == State::none);
    if(ssid.empty()) {
        DEBUG << "Disable access point";
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

        _isAccessPoint = false;
    } else {
        DEBUG << "Enable access point";
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

        wifi_config_t wifi_config;
        wifi_config.ap.ssid_len = ssid.CopyTo(wifi_config.ap.ssid, 32, true);
        wifi_config.ap.max_connection = 5;
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;

        _isAccessPoint = true;
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    }
}

void WiFi::SetConnection(const Buffer& ssid, const Buffer& password)
{
    INFO << "Set connection to" << ssid << "with password length" << password.length();
    Mutex::LockGuard lock(_mutex);
    ESPP_CHECK(_state == State::none || _state == State::wait_start || _state == State::started);
    if(ssid.empty()) {
        DEBUG << "Disable station";
        _isStation = false;
    } else {
        wifi_config_t config = {};
        ssid.StringCopyTo(config.sta.ssid, 32);
        password.StringCopyTo(config.sta.password, 64);
        _isStation = true;

        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config));
    }
}

bool WiFi::Start()
{
    INFO << "Start WiFi";
    Mutex::LockGuard lock(_mutex);
    if(_state != State::none) {
        DEBUG << "Invalid state";
        return false;
    }

    _state = State::wait_start;
    ESP_ERROR_CHECK(esp_wifi_start());
    return true;
}

bool WiFi::Connect()
{
    INFO << "Connect WiFi station";
    Mutex::LockGuard lock(_mutex);
    if(!_isStation) {
        DEBUG << "Station isn't inited";
        return false;
    }

    switch(_state) {
        case State::wait_start:
            DEBUG << "Wait start. Append wait connection";
            _state = State::wait_start_connect;
            break;
        case State::started:
            DEBUG << "Connect station";
            _state = State::wait_connect;
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        default:
            DEBUG << "Invalid state";
            return false;
    }

    return true;
}

bool WiFi::Disconnect()
{
    INFO << "Disconnect WiFi station";
    Mutex::LockGuard lock(_mutex);
    if(!_isStation) {
        DEBUG << "Station isn't inited";
        return false;
    }
    switch(_state) {
        case State::wait_start_connect:
            DEBUG << "Wait start and connect. Reset wait connect";
            _state = State::wait_start;
            break;
        case State::wait_connect:
        case State::connected:
            DEBUG << "Station connected. Disconnect";
            ESP_ERROR_CHECK(esp_wifi_disconnect());
            break;
        default:
            DEBUG << "Invalid state";
            return false;
    }
    return true;
}

bool WiFi::Stop()
{
    INFO << "Stop wifi";
    Mutex::LockGuard lock(_mutex);
    if(_state == State::none) {
        DEBUG << "Wifi wasn't started";
        return false;
    }
    ESP_ERROR_CHECK(esp_wifi_stop());
    return true;
}

void WiFi::OnStarted()
{
    INFO << "On started";
    Mutex::LockGuard lock(_mutex);
    switch(_state) {
        case State::wait_start:
            DEBUG << "WiFi started";
            _state = State::started;
            break;
        case State::wait_start_connect:
            DEBUG << "WiFi started and wait connection";
            _state = State::wait_connect;
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        default:
            DEBUG << "Invalid state";
    }
}

void WiFi::OnConnected()
{
    INFO << "On connected";
    Mutex::LockGuard lock(_mutex);
    switch(_state) {
        case State::wait_connect:
            _state = State::connected;
            break;
        default:
            ERROR << "Invalid state";
    }
}

void WiFi::OnGotIp()
{
    INFO << "On got IP";
    Mutex::LockGuard lock(_mutex);
    switch(_state) {
        case State::connected:
            _hasIp = true;
            break;
        default:
            ERROR << "Invalid state";
    }
}

void WiFi::OnDisconnected()
{
    INFO << "On disconnected";
    Mutex::LockGuard lock(_mutex);
    switch(_state) {
        case State::wait_connect:
            DEBUG << "Wait connect reset";
            _state = State::started;
            break;
        case State::connected:
            DEBUG << "Reset connected state";
            _state = State::started;
            break;
        default:
            ERROR << "Invalid state";
    }
    _hasIp = false;
}

void WiFi::OnStop()
{
    INFO << "On stop";
    Mutex::LockGuard lock(_mutex);
    _state = State::none;
}


esp_err_t WiFi::StaticEventHandler(void* context, system_event_t* event)
{
    WiFi* instance = reinterpret_cast<WiFi*>(context);
    instance->HandleEvent(*event);
    return ESP_OK;
}

void WiFi::HandleEvent(const system_event_t& event)
{
    switch(event.event_id) {
        case SYSTEM_EVENT_STA_START:
            OnStarted();
            break;
        case SYSTEM_EVENT_STA_STOP:
            OnStop();
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            OnConnected();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            OnGotIp();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            OnDisconnected();
            break;
//        case SYSTEM_EVENT_AP_START:
//            OnStarted();
//            break;
//        case SYSTEM_EVENT_AP_STOP:
//            OnStop();
//            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            OnStationConnected(event.event_info.sta_connected);
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            OnStationDisconnected(event.event_info.sta_disconnected);
            break;
        case SYSTEM_EVENT_AP_STAIPASSIGNED:
            OnStationIpAssigned(event.event_info.ap_staipassigned);
            break;
        default:
            DEBUG << "Unprocessed network event with id" << event.event_id;
    }
}

void WiFi::OnStationConnected(const system_event_ap_staconnected_t&)
{
    DEBUG << "station connected to" << "access point";
    Mutex::LockGuard lock(_mutex);
    if(!isStarted()) {
        ERROR << "Unexpected event";
        return;
    }
}

void WiFi::OnStationDisconnected(const system_event_ap_stadisconnected_t&)
{
    DEBUG << "station disconnected from" << "access point";
    Mutex::LockGuard lock(_mutex);
    if(!isStarted()) {
        ERROR << "Unexpected event";
        return;
    }
}

void WiFi::OnStationIpAssigned(const system_event_ap_staipassigned_t& event)
{
    DEBUG << "station got ip from" << "access point";
    Mutex::LockGuard lock(_mutex);
    if(!isStarted()) {
        ERROR << "Unexpected event";
        return;
    }
    DEBUG << "Assign ip" << ip4addr_ntoa(&event.ip);
}

}
