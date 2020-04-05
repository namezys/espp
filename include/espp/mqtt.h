#pragma once

#include <string>
#include <vector>

#include "mqtt_client.h"

#include "espp/log.h"

namespace espp {

class MqTT {
public:
    explicit
    MqTT(std::string  url);

    virtual ~MqTT();

    void Init();

    void publish(const char* topic, const char* data, std::size_t data_len)
    {
        assert(_client != nullptr);
        assert(data_len > 0);
        ESP_ERROR_CHECK(esp_mqtt_client_publish(_client, topic, data, data_len, 0, 0));
    }

    void publish(const std::string& topic, const std::vector<uint8_t>& data)
    {
        publish(topic.data(), reinterpret_cast<const char*>(data.data()), data.size());
    }

protected:
    virtual void OnConnect(int session_present) {}
    virtual void OnEvent(const esp_mqtt_event_handle_t& event) {}

private:
    const std::string _url;
    esp_mqtt_client_handle_t _client = nullptr;

    static
    bool _Init(bool init);

    static
    esp_err_t EventHandler(esp_mqtt_event_handle_t event);

    void ProcessEvent(esp_mqtt_event_handle_t event);
};

};