#pragma once

#include <esp_partition.h>

namespace espp {

/**
 * Count number of success boot and write it into NV partition
 */
class RecoveryBootCounter{
public:
    explicit
    RecoveryBootCounter(const char* label);

    /** Erase partition and reset counter */
    void Erase();

    /** Read current value */
    void Read();

    void StartBoot();

    void FinishBoot();

    int partialBootCounter() const
    {
        return _partial_boot_counter;
    }

private:
    const esp_partition_t* _partition;
    int _offset = -1;
    unsigned int _segment_offset = -1;
    int _partial_boot_counter = 0;
};

}