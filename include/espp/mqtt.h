#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "mqtt_client.h"

#include "espp/log.h"
#include "espp/mutex.h"
#include "espp/task.h"

#include <string>
#include <vector>
#include <map>

#include <espp/buffer.h>

namespace espp {

class MqttSubscription{
public:
    virtual void OnEvent(esp_mqtt_event_handle_t event)
    {
        if(event->data_len > 0) {
            OnEventData({event->data, static_cast<size_t>(event->data_len)});
        }
    }

    virtual void OnEventData(const Buffer& msg) = 0;
};

class Mqtt: public TaskBase{
public:
    using Mutex = espp::Mutex<>;

    explicit
    Mqtt(std::string url, std::string status_topic = {});

    virtual ~Mqtt();

    void Init(const Buffer& status_msg);

    bool Connect();

    bool Disconnect();

    bool isConnected() const
    {
        return _is_connected;
    }

    bool Publish(const char* topic, const char* data, std::size_t data_len, bool retain = false)
    {
        DEBUG << "Publish message to" << topic << "retain" << retain;
        ESPP_ASSERT(data_len > 0);
        ESPP_ASSERT(_client != nullptr);
        return ESP_OK == esp_mqtt_client_publish(_client, topic, data, data_len, 0, retain ? 1 : 0);
    }

    bool Publish(const Data& topic, const Buffer& data, bool retain = false)
    {
        return Publish(topic.charData(), data.charData(), data.length(), retain);
    }

    void UpdateStatus(const Buffer& msg)
    {
        ESPP_ASSERT(!_status_topic.empty());
        Publish(_status_topic, msg, true);
    }

    void Subscribe(MqttSubscription& subscription, const std::string& topic);

protected:
    virtual void OnConnect(int session_present);

    virtual void OnDisconnect();

    virtual void OnEvent(const esp_mqtt_event_handle_t& event);

private:
    using SubList = std::vector<std::pair<std::string, MqttSubscription*>>;
    const std::string _url;
    const std::string _status_topic;
    const int _keep_alive_timeout = 15;

    Mutex _mutex;
    esp_mqtt_client_handle_t _client = nullptr;
    SubList _subscriptions;
    bool _is_connected = true;

    static
    esp_err_t _EventHandler(esp_mqtt_event_handle_t event);

    void _ProcessEvent(esp_mqtt_event_handle_t event);

    void _ProcessConnect(int session_present);

    void _ProcessDisconnect();
};

};
