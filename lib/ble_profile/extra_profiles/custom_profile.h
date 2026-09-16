#pragma once

#include <furi_ble/profile_interface.h>
#include "../extra_services/custom_service.h"

#ifdef __cplusplus
extern "C" {
#endif

// Parameters struct to allow configuration when starting profile
typedef struct {
    uint8_t placeholder;
} BleProfileCustomParams;

// Expose the template so it can be passed to bt_profile_start
extern const FuriHalBleProfileTemplate* ble_profile_custom;

// API functions for interacting with the profile once it is started
bool ble_profile_custom_tx(FuriHalBleProfileBase* profile, uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif
