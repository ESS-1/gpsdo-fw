#include "eeprom.h"

ee_storage_t ee_storage;

bool ee_is_changed;

bool ee_save_config()
{
    return EE_Write();
}
