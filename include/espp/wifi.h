#pragma once

#include <esp_wifi.h>

#include <espp/task.h>
#include <espp/mutex.h>
#include <espp/log.h>
#include <espp/buffer.h>

namespace lamp {

using espp::Buffer;
using espp::Data;

/**
 * Can handle event in his own thread
 */
class WiFi: espp::TaskBase{
public:
    enum class State{
        none,                // -> wait_start
        started,             // -> wait_connect
        connected,           // -> started
        wait_start,          // -> started, wait_start_connect
        wait_connect,        // -> connected
        wait_start_connect,  // -> wait_connect, wait_start
    };

    State _state = State::none;
    bool _hasIp = false;

    WiFi();

    WiFi(const WiFi&) = delete;

    WiFi(WiFi&&) = delete;

    bool isAccessPoint() const
    {
        return _isAccessPoint;
    }

    bool isStation() const
    {
        return _isStation;
    }

    bool isStarted() const
    {
        return _state != State::none && _state != State::wait_start && _state != State::wait_start_connect;
    }

    bool isStationConnected() const
    {
        return _state == State::connected;
    }

    bool isStationConnecting() const
    {
        return _state == State::wait_connect || _state == State::wait_start_connect;
    }

    bool hasStationIp() const
    {
        return _hasIp;
    }

    Data accessPointSsid() const;

    Data stationSsid() const;

    void SetAccessPoint(const Buffer& ssid);

    void SetConnection(const Buffer& ssid, const Buffer& password);

    bool Start();

    bool Connect();

    bool Disconnect();

    bool Stop();


private:
    using Mutex = espp::Mutex<pdMS_TO_TICKS(100)>;
    mutable Mutex _mutex;
    bool _isScan = false;
    bool _isStation = false;
    bool _isAccessPoint = false;

    static
    esp_err_t StaticEventHandler(void* context, system_event_t* event);

    void HandleEvent(const system_event_t& event);

    void OnStarted();

    void OnConnected();

    void OnGotIp();

    void OnDisconnected();

    void OnStop();

    void OnStationConnected(const system_event_ap_staconnected_t&);

    void OnStationDisconnected(const system_event_ap_stadisconnected_t&);

    void OnStationIpAssigned(const system_event_ap_staipassigned_t&);
};

inline
const espp::Log& operator<<(const espp::Log& log, const WiFi& wifi)
{
    log << "WiFi started:" << wifi.isStarted() << "\n"
        << "\tstation" << "\n"
        << "\t\tactive:" << wifi.isStation() << "\n"
        << "\t\tconnected:" << wifi.isStationConnected() << "\n"
        << "\t\tSSID:" << wifi.stationSsid() << "\n"
        << "\taccess point" << "\n"
        << "\t\tactive:" << wifi.isAccessPoint() << "\n"
        << "\t\tSSID:" << wifi.accessPointSsid();
    return log;
}


}
