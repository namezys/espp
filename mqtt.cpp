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
    }

    _client = esp_mqtt_client_init(&mqtt_cfg);
    ESPP_CHECK(_client != nullptr);
}

bool Mqtt::Connect()
{
    INFO << "Connect Mqtt";
    Mutex::LockGuard lock(_mutex);
    ESPP_ASSERT(_client != nullptr);

    const auto result = ESP_OK == esp_mqtt_client_start(_client);
    DEBUG << "Connection finished" << result;
    return result;
}

bool Mqtt::Disconnect()
{
    INFO << "Disconnect Mqtt";
    Mutex::LockGuard lock(_mutex);
    const auto result = ESP_OK == esp_mqtt_client_stop(_client);
    DEBUG << "Disconnection finished" << result;
    return result;
}

void Mqtt::OnConnect(int)
{
}

void Mqtt::OnDisconnect()
{
}

void Mqtt::OnEvent(const esp_mqtt_event_handle_t& event)
{
    Mutex::LockGuard lock(_mutex);
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
    INFO << "Mqtt connected";

    Mutex::LockGuard lock(_mutex);
    for(auto& subscription: _subscriptions) {
        ESPP_CHECK(esp_mqtt_client_subscribe(_client, subscription.first.c_str(), 0) != -1);
    }
    _is_connected = true;
    DEBUG << "Connection finished";
}

void Mqtt::_ProcessDisconnect()
{
    INFO << "Mqtt disconnected";
    Mutex::LockGuard lock(_mutex);
    _is_connected = false;
}

}

