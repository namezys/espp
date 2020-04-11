#pragma once

#include "freertos/FreeRTOS.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "espp/task.h"
#include "espp/mutex.h"

#include <vector>

namespace espp {

class WiFi : protected TaskBase {
public:
    using ScanResult = std::vector<wifi_ap_record_t>;
    WiFi() = default;
    virtual ~WiFi();

    void Init();

    void StartScan(const wifi_scan_config_t& config);
    void StartScan();
    ScanResult scanResult();
    bool StopScan();

    bool isScan() const
    {
        return _is_scan;
    }

    bool isScanDone() const
    {
        return _is_scan_done;
    }

    void ConnectStation(const std::string& ssid, const std::string& password);
    bool DisconnectStation();

    bool isStationStarted() const
    {
        return _is_station_started;
    }

    bool isStationConnected() const
    {
        return _is_station_connected;
    }

protected:
    using Mutex = espp::Mutex<>;
    Mutex _mutex;
    wifi_init_config_t _config = WIFI_INIT_CONFIG_DEFAULT();
    std::vector<wifi_ap_record_t> _scan_result;

    bool isInited()
    {
        return _is_inited;
    }

    bool isOccupied() const
    {
        return _is_scan || _is_station_started;
    }

    virtual void OnReady();
    virtual void OnScanDone();
    virtual void OnConnected();
    virtual void OnDisconnected();

private:
    bool _is_inited = false;
    bool _is_scan = false;
    bool _is_scan_done = false;
    bool _is_station_started = false;
    bool _is_station_connected = false;

    void _Init();

    static esp_err_t _EventHandler(void* ctx, system_event_t *event);
    void _ProcessEvent(system_event_t& event);

    void _StationStarted();
    void _StationConnected();
    void _StationDisconnected();
    void _StationStopped();
};

}
