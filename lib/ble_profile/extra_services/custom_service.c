#include "custom_service.h"
#include <furi.h>
#include <furi_hal_bt.h>
#include <ble/ble.h>

#define TAG "BleCustomService"

// Example UUIDs
// Service UUID: 00000000-0000-1000-8000-00805F9B34FB
const uint8_t custom_service_uuid[16] =
    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// RX Char UUID
const uint8_t custom_char_rx_uuid[16] =
    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};

// TX Char UUID
const uint8_t custom_char_tx_uuid[16] =
    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};

BleServiceCustom* ble_svc_custom_start(void) {
    BleServiceCustom* custom_svc = malloc(sizeof(BleServiceCustom));

    Service_UUID_t service_uuid;
    memcpy(service_uuid.Service_UUID_128, custom_service_uuid, 16);
    if(aci_gatt_add_service(
           UUID_TYPE_128, &service_uuid, PRIMARY_SERVICE, 6, &custom_svc->svc_handle) !=
       BLE_STATUS_SUCCESS) {
        FURI_LOG_E(TAG, "Failed to add Custom Service");
        free(custom_svc);
        return NULL;
    }

    Char_UUID_t char_uuid;
    memcpy(char_uuid.Char_UUID_128, custom_char_rx_uuid, 16);
    tBleStatus status = aci_gatt_add_char(
        custom_svc->svc_handle,
        UUID_TYPE_128,
        &char_uuid,
        BLE_SVC_CUSTOM_DATA_MAX,
        CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RESP,
        ATTR_PERMISSION_NONE,
        GATT_DONT_NOTIFY_EVENTS,
        16,
        CHAR_VALUE_LEN_VARIABLE,
        &custom_svc->char_rx_handle);
    if(status == BLE_STATUS_SUCCESS) {
        memcpy(char_uuid.Char_UUID_128, custom_char_tx_uuid, 16);
        status = aci_gatt_add_char(
            custom_svc->svc_handle,
            UUID_TYPE_128,
            &char_uuid,
            BLE_SVC_CUSTOM_DATA_MAX,
            CHAR_PROP_NOTIFY,
            ATTR_PERMISSION_NONE,
            GATT_DONT_NOTIFY_EVENTS,
            16,
            CHAR_VALUE_LEN_VARIABLE,
            &custom_svc->char_tx_handle);
    }
    if(status != BLE_STATUS_SUCCESS) {
        FURI_LOG_E(TAG, "Failed to add characteristic: %u", status);
        aci_gatt_del_service(custom_svc->svc_handle);
        free(custom_svc);
        return NULL;
    }

    FURI_LOG_I(TAG, "Custom GATT Service started successfully");
    return custom_svc;
}

void ble_svc_custom_stop(BleServiceCustom* custom_svc) {
    furi_check(custom_svc);

    // Removing the service also removes its characteristics and descriptors.
    aci_gatt_del_service(custom_svc->svc_handle);

    free(custom_svc);
}

bool ble_svc_custom_tx(BleServiceCustom* custom_svc, const uint8_t* data, uint16_t size) {
    furi_check(custom_svc);
    if(!data || !size || size > BLE_SVC_CUSTOM_DATA_MAX) return false;
    return aci_gatt_update_char_value(
               custom_svc->svc_handle, custom_svc->char_tx_handle, 0, size, data) ==
           BLE_STATUS_SUCCESS;
}
