#include "eeprom.h"

ee_storage_t ee_storage;

bool ee_is_changed;

bool ee_save_config()
{
    ee_storage.total_writes++;
    if (EE_Write()) {
        ee_is_changed = false;
        return true;
    }

    return false;
}
