#include <espp/utils/boot_counter.h>
#include <espp/log.h>
#include <espp/utils/macros.h>

namespace espp {

RecoveryBootCounter::RecoveryBootCounter(const char* label):
    _partition(esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, label))
{
    ESPP_CHECK(_partition != nullptr);
}

void RecoveryBootCounter::Erase()
{
    INFO << "Erase recovery reboot counter";
    ESP_ERROR_CHECK(esp_partition_erase_range(_partition, 0, _partition->size));
    _offset = -1;
    _segment_offset = 0;
    auto counter = _partial_boot_counter;
    if(counter > 10) {
        INFO << "Counter too big. Reduce to 10";
        counter = 10;
    }
    DEBUG << "Save" << counter << "partial boots";
    _partial_boot_counter = 0;
    for(; counter > 0; --counter) {
        StartBoot();
    }
}

void RecoveryBootCounter::Read()
{
    INFO << "Read recovery reboot counter";

    _offset = -1;
    _segment_offset = 0;
    _partial_boot_counter = 0;
    int32_t current_offset = _partition->size - 1;
    for(; current_offset >= 0; --current_offset) {
        uint8_t value;
        ESP_ERROR_CHECK(esp_partition_read(_partition, current_offset, &value, 1));
        if(value == 0xFFu) {
            continue;
        }
        VERBOSE << "Found boot record at offset" << current_offset << "value" << value;
        for(signed segment_offset = 6; segment_offset >= 0; segment_offset -= 2) {
            uint8_t mask = (0x3u << static_cast<unsigned>(segment_offset));
            uint8_t masked_value = mask & value;
            if(masked_value == mask) {
                continue;
            }
            if(_offset == -1) {
                _offset = current_offset;
                _segment_offset = segment_offset;
                DEBUG << "Found last boot record segment" << _segment_offset << "at offset" << _offset;
            }
            if(masked_value == 0x0u) {
                DEBUG << "Found success reboot and" << _partial_boot_counter << "partial boots";
                return;
            }
            VERBOSE << "Found partial reboot";
            _partial_boot_counter += 1;
        }
    }
    DEBUG << "Found " << _partial_boot_counter << "partial boots";
}

void RecoveryBootCounter::StartBoot()
{
    INFO << "Mark boot";
    if(_offset == -1) {
        _offset = 0;
        _segment_offset = 0;
    } else {
        if(_segment_offset == 6) {
            _offset += 1;
            _segment_offset = 0;
        } else {
            _segment_offset += 2;
        }
    }
    if(_offset == _partition->size) {
        DEBUG << "Partition full, Erase";
        Erase();
    }
    uint8_t value = (0x2u << _segment_offset);
    value = ~value;
    DEBUG << "Write" << value << "to" << _offset << "with segment" << _segment_offset;
    ESP_ERROR_CHECK(esp_partition_write(_partition, _offset, &value, 1));
    _partial_boot_counter += 1;
}

void RecoveryBootCounter::FinishBoot()
{
    INFO << "Mark normal boot";
    ESPP_ASSERT(_offset >= 0);
    ESPP_ASSERT(_segment_offset <= 6);
    uint8_t value = (0x3u << _segment_offset);
    value = ~value;
    DEBUG << "Write" << value << "to" << _offset << "with segment" << _segment_offset;
    ESP_ERROR_CHECK(esp_partition_write(_partition, _offset, &value, 1));
    _partial_boot_counter = 0;
}
}
