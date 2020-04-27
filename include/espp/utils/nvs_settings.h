#pragma once

#include <string>
#include <nvs.h>
#include <vector>

namespace espp {

class NvsSettings{
public:
    NvsSettings();

    bool isInited() const
    {
        return _isStorageInited;
    }

    void Load();

    void Save();

protected:
    virtual void DoLoad();

    virtual void DoSave();

    std::string ReadStr(const char* name)
    {
        size_t length;
        const auto getResult = nvs_get_str(_nvs, name, nullptr, &length);
        if(getResult == ESP_ERR_NVS_NOT_FOUND) {
            return {};
        }
        ESP_ERROR_CHECK(getResult);
        length += 1;
        std::vector<char> result(length);
        ESP_ERROR_CHECK(nvs_get_str(_nvs, name, result.data(), &length));
        return {result.data(), result.data() + length};
    }

    void WriteStr(const char* name, const std::string& str, bool& changed)
    {
        if (changed) {
            ESP_ERROR_CHECK(nvs_set_str(_nvs, name, str.c_str()));
            changed = false;
        }
    }

private:
    nvs_handle _nvs = {};
    bool _isStorageInited = false;
};

}
