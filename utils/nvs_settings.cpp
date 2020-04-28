#include <espp/utils/nvs_settings.h>

#include <espp/log.h>
#include <nvs_flash.h>
#include <espp/utils/macros.h>

namespace espp {

NvsSettings::NvsSettings()
{
    INFO << "Init NVS";
    ESP_ERROR_CHECK(nvs_flash_init());
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    if(ESP_OK != nvs_open("settings", NVS_READWRITE, &_nvs)) {
        INFO << "Can't init settings";
        _isStorageInited = false;
    } else {
        INFO << "Storage was inited";
        _isStorageInited = true;
    }
}

void NvsSettings::Load()
{
    if(!_isStorageInited) {
        DEBUG << "Store not inited. Skip load";
        return;
    }
    DoLoad();
}

void NvsSettings::Save()
{
    if(!_isStorageInited) {
        DEBUG << "Store not inited. Skip load";
        return;
    }
    DoSave();
    ESP_ERROR_CHECK(nvs_commit(_nvs));
}

void NvsSettings::DoLoad()
{
    ESPP_CHECK(!"Can't be called and must be override");
}

void NvsSettings::DoSave()
{
    ESPP_CHECK(!"Can't be called and must be override");
}


}
