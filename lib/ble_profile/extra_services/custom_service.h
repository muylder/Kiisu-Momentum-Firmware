#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BleServiceCustom BleServiceCustom;

extern const uint8_t custom_service_uuid[16];
#define BLE_SVC_CUSTOM_DATA_MAX 20

struct BleServiceCustom {
    uint16_t svc_handle;
    uint16_t char_rx_handle;
    uint16_t char_tx_handle;
};

/**
 * @brief Start Custom BLE Service
 * @return BleServiceCustom* pointer to service context
 */
BleServiceCustom* ble_svc_custom_start(void);

/**
 * @brief Stop Custom BLE Service
 * @param custom_svc Pointer to service context
 */
void ble_svc_custom_stop(BleServiceCustom* custom_svc);

bool ble_svc_custom_tx(BleServiceCustom* custom_svc, const uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif
