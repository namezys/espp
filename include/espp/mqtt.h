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

namespace espp {

class MqttMsg: public std::vector<uint8_t> {
public:
    MqttMsg():
        std::vector<uint8_t>()
    {}

    MqttMsg(const char* begin, const char* end):
        std::vector<uint8_t>(begin, end)
    {}

    MqttMsg(const char* str, std::size_t length):
        std::vector<uint8_t>(str, str + length)
    {}

    MqttMsg(const char* str):
        std::vector<uint8_t>(str, str + std::strlen(str))
    {}

    MqttMsg(const std::vector<uint8_t>& other):
        std::vector<uint8_t>(other)
    {}

    MqttMsg(std::vector<uint8_t>&& other):
        std::vector<uint8_t>(other)
    {}

    MqttMsg(const MqttMsg& other) = default;
    MqttMsg(MqttMsg&& other) = default;

    operator std::string() const
    {
        return {begin(), end()};
    }

    MqttMsg& operator=(const MqttMsg&) = default;

    MqttMsg& operator=(const char* str)
    {
        *this = MqttMsg(str);
        return *this;
    }
};

class MqttSubscription {
public:
    using Msg = MqttMsg;

    virtual void OnEvent(esp_mqtt_event_handle_t event)
    {
        if (event->data_len > 0) {
            MqttMsg msg(event->data, event->data + event->data_len);
            OnEventData(msg);
        }
    }
    virtual void OnEventData(const MqttMsg& msg) = 0;
};

class Mqtt: public TaskBase {
public:
    using Mutex = espp::Mutex<>;
    using Msg = MqttMsg;

    explicit
    Mqtt(std::string  url, std::string status_topic = {});
    virtual ~Mqtt();

    void Init(const MqttMsg& status_msg = {});
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

    bool Publish(const std::string& topic, const MqttMsg& data, bool retain = false)
    {
        return Publish(topic.data(), reinterpret_cast<const char*>(data.data()), data.size(), retain);
    }

    void UpdateStatus(const MqttMsg& msg)
    {
        ESPP_ASSERT(!_status_topic.empty());
        Publish(_status_topic, msg, true);
    }

    void Subscribe(MqttSubscription& subscription, const std::string& topic)
    {
        assert(_subscriptions.count(topic) == 0);
        _subscriptions[topic] = &subscription;
    }

protected:
    virtual void OnConnect(int session_present);
    virtual void OnDisconnect();
    virtual void OnEvent(const esp_mqtt_event_handle_t& event);

private:
    const std::string _url;
    const std::string _status_topic;
    const int _keep_alive_timeout = 15;

    Mutex _mutex;
    esp_mqtt_client_handle_t _client = nullptr;
    std::map<std::string, MqttSubscription*> _subscriptions;
    bool _is_connected = true;

    static
    esp_err_t _EventHandler(esp_mqtt_event_handle_t event);

    void _ProcessEvent(esp_mqtt_event_handle_t event);
    void _ProcessConnect(int session_present);
    void _ProcessDisconnect();
};

};
