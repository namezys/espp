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
    WiFi() = default;
    ~WiFi();

    template<class WiFiImpl> static
    void Init(WiFiImpl& impl)
    {
        assert(!impl._is_inited);

        INFO << "Init WiFi system";
        assert(_Init(&impl));
        const auto callback = _EventHandler<WiFiImpl>;
        const auto init_loop_result = esp_event_loop_init(callback, &impl);
        if (init_loop_result == ESP_FAIL) {
            DEBUG << "Loop was inited early";
            assert(nullptr == esp_event_loop_set_cb(callback, &impl));
        } else {
            DEBUG << "Loop was inited";
        }
        impl._InitSystem(impl._config);
        impl._is_inited = true;
        Mutex::LockGuard lock(impl._mutex);
        impl.OnReady();
    }

protected:
    Mutex _mutex;
    wifi_init_config_t _config = WIFI_INIT_CONFIG_DEFAULT();

    std::vector<wifi_ap_record_t> _scan_result;

    static bool _Init(void* ptr);
    void _InitSystem(wifi_init_config_t& config);

    template<class WiFiImpl> static
    esp_err_t _EventHandler(void* ctx, system_event_t *event)
    {
        WiFiImpl* impl = reinterpret_cast<WiFiImpl*>(ctx);
        Mutex::LockGuard lock(impl->_mutex);
        switch(event->event_id) {
            case SYSTEM_EVENT_SCAN_DONE:
                impl->_FinishScan();
                break;
            case SYSTEM_EVENT_STA_START:
                impl->_StationStarted();
                break;
            case SYSTEM_EVENT_STA_GOT_IP:
                impl->OnConnected();
            default:
                ;
        }
        return ESP_OK;
    }

    void OnReady()
    {
    }

    void OnScanDone()
    {
    }

    void OnConnected()
    {
    };

    bool IsInited()
    {
        return _is_inited;
    }

    bool IsOccupied() const
    {
        return _is_scan || _is_station;
    }

    void StartScan(const wifi_scan_config_t& config);
    void StartScan()
    {
        wifi_scan_config_t config = {nullptr,nullptr,0,true,
                                     WIFI_SCAN_TYPE_ACTIVE, {}};
        config.scan_time.active.min = 1200;
        config.scan_time.active.max = 1500;
        StartScan(config);
    }
    void StopScan();

    void Connect(const std::string& ssid, const std::string& password);
    void Disconnect();

private:
    bool _is_inited = false;
    bool _is_scan = false;
    bool _is_station = false;

    void _FinishScan();
    void _StationStarted();

};

}