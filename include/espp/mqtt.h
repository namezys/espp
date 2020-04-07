#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "mqtt_client.h"

#include "espp/log.h"
#include "espp/mutex.h"

#include <string>
#include <vector>
#include <map>

namespace espp {

using MqttMsg = std::vector<uint8_t>;

class MqttSubscription {
public:
    virtual void OnEvent(esp_mqtt_event_handle_t event)
    {
        if (event->data_len > 0) {
            MqttMsg msg(event->data, event->data + event->data_len);
            OnEventData(msg);
        }
    }
    virtual void OnEventData(const MqttMsg& msg) = 0;
};

class Mqtt {
public:
    explicit
    Mqtt(std::string  url, std::string status_topic = {});
    virtual ~Mqtt();

    void Init(const MqttMsg& status_msg = {});
    void Connect();
    bool Disconnect();

    bool Publish(const char* topic, const char* data, std::size_t data_len, bool retain = false)
    {
        DEBUG << "Publish message to" << topic << "retain" << retain;
        assert(data_len > 0);
        espp::Mutex::LockGuard lock(_mutex);
        assert(_client != nullptr);
        return ESP_OK == esp_mqtt_client_publish(_client, topic, data, data_len, 0, retain ? 1 : 0);
    }

    bool Publish(const std::string& topic, const MqttMsg& data, bool retain = false)
    {
        return Publish(topic.data(), reinterpret_cast<const char*>(data.data()), data.size(), retain);
    }

    void UpdateStatus(const MqttMsg& msg)
    {
        assert(!_status_topic.empty());
        Publish(_status_topic, msg, true);
    }

protected:
    virtual void OnConnect(int session_present);
    virtual void OnDisconnect();
    virtual void OnEvent(const esp_mqtt_event_handle_t& event);

    virtual void OnStatusUpdateTimer() {}

    void Subscribe(MqttSubscription& subscription, const std::string& topic)
    {
        assert(_subscriptions.count(topic) == 0);
        _subscriptions[topic] = &subscription;
    }

private:
    const std::string _url;
    const std::string _status_topic;
    const int _keep_alive_timeout = 15;
    const UBaseType_t _update_status_interval = pdMS_TO_TICKS(5000);
    espp::Mutex _mutex;
    esp_mqtt_client_handle_t _client = nullptr;
    std::map<std::string, MqttSubscription*> _subscriptions;
    TimerHandle_t _update_status_timer = nullptr;
    bool _is_connection = false;

    static
    esp_err_t _EventHandler(esp_mqtt_event_handle_t event);

    static
    void _UpdateStatusEventHandler(TimerHandle_t xTimer);

    void _ProcessEvent(esp_mqtt_event_handle_t event);
    void _ProcessConnect(int session_present);
    void _ProcessDisconnect();
};

};
