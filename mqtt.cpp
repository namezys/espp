#include "freertos/FreeRTOS.h"
#include "mqtt_client.h"

#include "espp/mqtt.h"
#include "espp/utils/macros.h"

#include <utility>

namespace espp {

namespace {

bool _SetMqttSingleton(bool init)
{
    static bool is_inited = false;
    if (init == is_inited) {
        return false;
    }
    is_inited = init;
    return true;
}

}

Mqtt::Mqtt(std::string url, std::string status_topic):
    _url(std::move(url)), _status_topic(std::move(status_topic))
{
    ESPP_CHECK(_SetMqttSingleton(true));
}

Mqtt::~Mqtt()
{
    ESPP_CHECK(_SetMqttSingleton(false));
    if (_client != nullptr) {
        ESP_ERROR_CHECK(esp_mqtt_client_destroy(_client));
    }
    if (_update_status_timer != nullptr) {
        ESPP_CHECK(pdPASS == xTimerDelete(_update_status_timer, portMAX_DELAY));
    }
}

void Mqtt::Init(const MqttMsg& status_msg)
{
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.uri = _url.data();
    mqtt_cfg.event_handle = _EventHandler;
    mqtt_cfg.user_context = this;
    if (!_status_topic.empty()) {
        DEBUG << "Init Mqtt LWT";
        assert(!status_msg.empty());
        mqtt_cfg.lwt_msg = reinterpret_cast<const char*>(status_msg.data());
        mqtt_cfg.lwt_msg_len = status_msg.size();
        mqtt_cfg.lwt_topic = _status_topic.c_str();
        mqtt_cfg.lwt_qos = 0;
        mqtt_cfg.lwt_retain = 1;
        mqtt_cfg.keepalive = _keep_alive_timeout;

        DEBUG << "Init update status timer";
        _update_status_timer = xTimerCreate(nullptr, _update_status_interval, pdTRUE,
                                            this, _UpdateStatusEventHandler);
        assert(_update_status_timer != nullptr);
    }

    _client = esp_mqtt_client_init(&mqtt_cfg);
    ESPP_CHECK(_client != nullptr);
}

void Mqtt::Connect()
{
    INFO << "Connect Mqtt";
    espp::Mutex::LockGuard lock(_mutex);
    assert(!_is_connection);
    ESP_ERROR_CHECK(esp_mqtt_client_start(_client));
    _is_connection = true;
}

bool Mqtt::Disconnect()
{
    INFO << "Disconnect Mqtt";
    espp::Mutex::LockGuard lock(_mutex);
    if (!_is_connection) {
        DEBUG << "No connection";
        return false;
    }
    if (ESP_OK != esp_mqtt_client_stop(_client)) {
        DEBUG << "Can't stop because mqtt was stopped";
    }
    _is_connection = false;
    return true;
}

void Mqtt::OnConnect(int)
{
}

void Mqtt::OnDisconnect()
{
    Disconnect();
}

void Mqtt::OnEvent(const esp_mqtt_event_handle_t& event)
{
    const std::string topic(event->topic, event->topic_len);
    DEBUG << "Got message in topic" << topic;
    const auto it = _subscriptions.find(topic);
    if (it != _subscriptions.end()) {
        DEBUG << "Found subscription";
        it->second->OnEvent(event);
    } else {
        DEBUG << "Can't find subscription";
    }
}

esp_err_t Mqtt::_EventHandler(esp_mqtt_event_handle_t event)
{
    Mqtt* impl = reinterpret_cast<Mqtt*>(event->user_context);
    impl->_ProcessEvent(event);
    return ESP_OK;
}

void Mqtt::_UpdateStatusEventHandler(TimerHandle_t xTimer)
{
    Mqtt* impl = reinterpret_cast<Mqtt*>(pvTimerGetTimerID(xTimer));
    impl->OnStatusUpdateTimer();
}

void Mqtt::_ProcessEvent(esp_mqtt_event_handle_t event)
{
    switch(event->event_id) {
        case MQTT_EVENT_CONNECTED:
            _ProcessConnect(event->session_present);
            OnConnect(event->session_present);
            break;
        case MQTT_EVENT_DISCONNECTED:
            _ProcessDisconnect();
            OnDisconnect();
            break;
        case MQTT_EVENT_DATA:
            OnEvent(event);
            break;
        default:
            ;
    }
}

void Mqtt::_ProcessConnect(int session_present)
{
    DEBUG << "Process connection. Subscribe all subscription";
    espp::Mutex::LockGuard lock(_mutex);
    for(auto& subscription: _subscriptions) {
        ESPP_CHECK(esp_mqtt_client_subscribe(_client, subscription.first.c_str(), 0) != -1);
    }
    if (_update_status_timer != nullptr) {
        DEBUG << "Start update status timer";
        ESPP_CHECK(pdPASS == xTimerStart(_update_status_timer, portMAX_DELAY));
    }
}

void Mqtt::_ProcessDisconnect()
{
    DEBUG << "Process disconnect";
    espp::Mutex::LockGuard lock(_mutex);
    if (_update_status_timer != nullptr) {
        DEBUG << "Stop update status timer";
        ESPP_CHECK(pdPASS == xTimerStop(_update_status_timer, portMAX_DELAY));
    }
}

}

