#include "custom_profile.h"
#include <furi.h>
#include <ble/ble.h>

#define TAG "BleProfileCustom"

// Internal profile structure
typedef struct {
    FuriHalBleProfileBase base; // Must be first!
    BleServiceCustom* custom_svc;
    BleProfileCustomParams params;
} BleProfileCustom;

static FuriHalBleProfileBase* ble_profile_custom_start(FuriHalBleProfileParams profile_params) {
    BleProfileCustom* profile = malloc(sizeof(BleProfileCustom));
    profile->base.config = ble_profile_custom; // point to template

    if(profile_params) {
        profile->params = *(BleProfileCustomParams*)profile_params;
    }

    profile->custom_svc = ble_svc_custom_start();
    if(!profile->custom_svc) {
        free(profile);
        return NULL;
    }
    return (FuriHalBleProfileBase*)profile;
}

static void ble_profile_custom_stop(FuriHalBleProfileBase* profile_base) {
    BleProfileCustom* profile = (BleProfileCustom*)profile_base;
    ble_svc_custom_stop(profile->custom_svc);
    free(profile);
}

static void ble_profile_custom_get_gap_config(
    GapConfig* target_config,
    FuriHalBleProfileParams profile_params) {
    UNUSED(profile_params);
    *target_config = (GapConfig){
        .adv_service.UUID_Type = UUID_TYPE_128,
        .bonding_mode = true,
        .pairing_method = GapPairingPinCodeShow,
        .conn_param = {.conn_int_min = 0x06, .conn_int_max = 0x24},
    };
    memcpy(target_config->adv_service.Service_UUID_128, custom_service_uuid, 16);
    memcpy(
        target_config->mac_address,
        furi_hal_version_get_ble_mac(),
        sizeof(target_config->mac_address));
    target_config->mac_address[2] += 2;
    snprintf(
        target_config->adv_name,
        sizeof(target_config->adv_name),
        "%c%s",
        AD_TYPE_COMPLETE_LOCAL_NAME,
        "Kiisu Custom");
}

// Define the template instance!
static const FuriHalBleProfileTemplate ble_profile_custom_template = {
    .start = ble_profile_custom_start,
    .stop = ble_profile_custom_stop,
    .get_gap_config = ble_profile_custom_get_gap_config,
};

const FuriHalBleProfileTemplate* ble_profile_custom = &ble_profile_custom_template;

bool ble_profile_custom_tx(FuriHalBleProfileBase* profile_base, uint8_t* data, uint16_t size) {
    furi_check(profile_base && profile_base->config == ble_profile_custom);
    BleProfileCustom* profile = (BleProfileCustom*)profile_base;
    return ble_svc_custom_tx(profile->custom_svc, data, size);
}
