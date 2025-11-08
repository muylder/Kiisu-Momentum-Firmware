#include "kiisu_i2c_helpers.h"
#include <furi_hal_gpio.h>
#include <furi.h>
#include <string.h>

// Optimization #1: Reduced retries with exponential backoff
#define I2C_DEFAULT_RETRIES 2
static const uint8_t RETRY_DELAYS_MS[] = {0, 1};

// External I2C pins for pull-up configuration
extern const GpioPin gpio_ext_pc0; // I2C SCL
extern const GpioPin gpio_ext_pc1; // I2C SDA

// Power I2C pins (if needed)
extern const GpioPin gpio_i2c_power_sda;
extern const GpioPin gpio_i2c_power_scl;

bool kiisu_i2c_write_mem_retry(
    KiisuI2cConfig* cfg,
    uint8_t device_addr,
    uint8_t mem_addr,
    const uint8_t* data,
    size_t len) {
    furi_assert(cfg);
    furi_assert(data);
    furi_assert(cfg->bus);

    uint8_t retries = cfg->retries > 0 ? cfg->retries : I2C_DEFAULT_RETRIES;

    for(uint8_t attempt = 0; attempt < retries; attempt++) {
        furi_hal_i2c_acquire(cfg->bus);

        // Method 1: Try furi_hal_i2c_write_mem first (most reliable)
        bool ok = furi_hal_i2c_write_mem(
            cfg->bus, device_addr, mem_addr, data, len, cfg->timeout_ms);

        // Method 2: If write_mem fails and data is small, try manual transaction
        if(!ok && len <= 31) { // Max 32 bytes total (1 addr + 31 data)
            uint8_t buf[32];
            buf[0] = mem_addr;
            memcpy(buf + 1, data, len);
            ok = furi_hal_i2c_tx(cfg->bus, device_addr, buf, len + 1, cfg->timeout_ms);
        }

        furi_hal_i2c_release(cfg->bus);

        if(ok) return true;

        // Optimization #1: Exponential backoff delay before retry
        if(attempt < retries - 1) {
            furi_delay_ms(RETRY_DELAYS_MS[attempt]);
        }
    }

    return false;
}

bool kiisu_i2c_read_mem_retry(
    KiisuI2cConfig* cfg,
    uint8_t device_addr,
    uint8_t mem_addr,
    uint8_t* data,
    size_t len) {
    furi_assert(cfg);
    furi_assert(data);
    furi_assert(cfg->bus);

    uint8_t retries = cfg->retries > 0 ? cfg->retries : I2C_DEFAULT_RETRIES;

    for(uint8_t attempt = 0; attempt < retries; attempt++) {
        furi_hal_i2c_acquire(cfg->bus);
        bool ok = furi_hal_i2c_read_mem(
            cfg->bus, device_addr, mem_addr, data, len, cfg->timeout_ms);
        furi_hal_i2c_release(cfg->bus);

        if(ok) return true;

        // Optimization #1: Exponential backoff delay before retry
        if(attempt < retries - 1) {
            furi_delay_ms(RETRY_DELAYS_MS[attempt]);
        }
    }

    return false;
}

bool kiisu_i2c_is_device_ready(KiisuI2cConfig* cfg, uint8_t device_addr) {
    furi_assert(cfg);
    furi_assert(cfg->bus);

    uint8_t retries = cfg->retries > 0 ? cfg->retries : I2C_DEFAULT_RETRIES;

    for(uint8_t attempt = 0; attempt < retries; attempt++) {
        furi_hal_i2c_acquire(cfg->bus);
        bool ok = furi_hal_i2c_is_device_ready(cfg->bus, device_addr, cfg->timeout_ms);
        furi_hal_i2c_release(cfg->bus);

        if(ok) return true;

        if(attempt < retries - 1) {
            furi_delay_ms(RETRY_DELAYS_MS[attempt]);
        }
    }

    return false;
}

void kiisu_i2c_configure_pullups(const FuriHalI2cBusHandle* handle, bool enable) {
    furi_assert(handle);

    LL_GPIO_InitTypeDef pull_mode = enable ? LL_GPIO_PULL_UP : LL_GPIO_PULL_NO;

    if(handle == &furi_hal_i2c_handle_power) {
        // Power I2C (internal bus)
        LL_GPIO_SetPinPull(gpio_i2c_power_sda.port, gpio_i2c_power_sda.pin, pull_mode);
        LL_GPIO_SetPinPull(gpio_i2c_power_scl.port, gpio_i2c_power_scl.pin, pull_mode);
    } else if(handle == &furi_hal_i2c_handle_external) {
        // External I2C bus
        LL_GPIO_SetPinPull(gpio_ext_pc1.port, gpio_ext_pc1.pin, pull_mode); // SDA
        LL_GPIO_SetPinPull(gpio_ext_pc0.port, gpio_ext_pc0.pin, pull_mode); // SCL
    }
}
