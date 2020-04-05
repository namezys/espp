#include "freertos/FreeRTOS.h"

#include "espp/mqtt.h"

#include <utility>
#include "mqtt_client.h"

namespace espp {

MqTT::MqTT(std::string url):
    _url(std::move(url))
{
    assert(_Init(true));
}

MqTT::~MqTT()
{
    if (_client != nullptr) {
        ESP_ERROR_CHECK(esp_mqtt_client_stop(_client));
    }
    assert(!_Init(false));
}

void MqTT::Init()
{
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.uri = _url.data();
    mqtt_cfg.event_handle = EventHandler;
    mqtt_cfg.user_context = this;

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
    _client = client;
}

bool MqTT::_Init(bool init)
{
    static bool is_inited = false;
    if (init == is_inited) {
        return false;
    }
    is_inited = init;
    return true;
}

esp_err_t MqTT::EventHandler(esp_mqtt_event_handle_t event)
{
    MqTT* impl = reinterpret_cast<MqTT*>(event->user_context);
    impl->ProcessEvent(event);
    return ESP_OK;
}

void MqTT::ProcessEvent(esp_mqtt_event_handle_t event)
{
    switch(event->event_id) {
        case MQTT_EVENT_CONNECTED:
            OnConnect(event->session_present);
            break;
        case MQTT_EVENT_DATA:
            OnEvent(event);
            break;
        default:
            ;
    }
}

}

