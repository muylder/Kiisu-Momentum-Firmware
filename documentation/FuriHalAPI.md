# FuriHal API Documentation

Complete hardware abstraction layer reference for Flipper Zero firmware.

## Overview

**FuriHal** is the Hardware Abstraction Layer (HAL) that provides a clean, platform-independent API for interacting with all Flipper Zero hardware components. It abstracts the STM32WB55 microcontroller and external peripherals.

**Architecture**: Two-stage initialization (early + full)
**Location**: `targets/furi_hal_include/` (headers), `targets/f7/furi_hal/` (implementation)
**Hardware Target**: Flipper Zero (STM32WB55, TARGET_HW=7)

## Module Overview

| Module | Header | Purpose |
|--------|--------|---------|
| **System** | `furi_hal.h` | Core initialization and boot control |
| **Power** | `furi_hal_power.h` | Battery, charging, sleep modes, OTG |
| **GPIO** | `furi_hal_gpio.h` | Digital I/O and external pins |
| **Bluetooth** | `furi_hal_bt.h` | BLE stack on CPU2 (STM32WB55) |
| **USB** | `furi_hal_usb.h` | USB device modes |
| **NFC** | `furi_hal_nfc.h` | ST25R3916 NFC controller |
| **Sub-GHz** | `furi_hal_subghz.h` | CC1101 radio (315/433/868/915 MHz) |
| **Infrared** | `furi_hal_infrared.h` | IR transmit/receive |
| **RFID** | `furi_hal_rfid.h` | 125 kHz RFID reader |
| **iButton** | `furi_hal_ibutton.h` | 1-Wire protocol (Dallas/Maxim) |
| **Serial** | `furi_hal_serial.h` | UART/LPUART communication |
| **SPI** | `furi_hal_spi.h` | SPI bus management |
| **I2C** | `furi_hal_i2c.h` | I2C bus management |
| **ADC** | `furi_hal_adc.h` | Analog-to-digital conversion |
| **Crypto** | `furi_hal_crypto.h` | Hardware cryptography (AES) |
| **RTC** | `furi_hal_rtc.h` | Real-time clock and backup registers |
| **Flash** | `furi_hal_flash.h` | Internal flash management |
| **SD Card** | `furi_hal_sd.h` | microSD card interface |
| **Speaker** | `furi_hal_speaker.h` | Audio output control |
| **Vibro** | `furi_hal_vibro.h` | Vibration motor control |
| **Light** | `furi_hal_light.h` | RGB backlight control |
| **Display** | `furi_hal_display.h` | LCD screen control |
| **Random** | `furi_hal_random.h` | Hardware random number generator |
| **Version** | `furi_hal_version.h` | Device version and serial info |
| **Region** | `furi_hal_region.h` | Regional frequency settings |
| **Resources** | `furi_hal_resources.h` | Pin definitions and resources |
| **Interrupts** | `furi_hal_interrupt.h` | ISR management |

---

## Core System

### Initialization

FuriHal uses two-stage initialization to support bootloader operations:

```c
// Stage 1: Early initialization (minimal subsystems)
// Called by bootloader and firmware
void furi_hal_init_early(void);

// Stage 2: Full initialization (all subsystems)
// Called only by firmware after RTOS start
void furi_hal_init(void);

// Reverse early init (for firmware switching)
void furi_hal_deinit_early(void);
```

### Boot Control

```c
// Set normal boot flag
furi_hal_set_is_normal_boot(true);

// Check boot mode
if(furi_hal_is_normal_boot()) {
    // Full firmware boot
} else {
    // DFU or recovery mode
}
```

### Firmware Switching

```c
// Jump to another firmware (e.g., DFU mode)
// WARNING: Call only from main thread, no RTOS running
void furi_hal_switch(void* address);
```

---

## Power Management

### Battery Monitoring

```c
// Get battery level (0-100%)
uint8_t level = furi_hal_power_get_pct();

// Get battery voltage (V)
float voltage = furi_hal_power_get_battery_voltage(FuriHalPowerICFuelGauge);

// Get battery current (A)
float current = furi_hal_power_get_battery_current(FuriHalPowerICFuelGauge);

// Get battery temperature (°C)
float temp = furi_hal_power_get_battery_temperature(FuriHalPowerICFuelGauge);

// Get battery capacity
uint32_t remaining_mah = furi_hal_power_get_battery_remaining_capacity();
uint32_t full_mah = furi_hal_power_get_battery_full_capacity();
uint32_t design_mah = furi_hal_power_get_battery_design_capacity();
```

### Charging Control

```c
// Check charging status
bool is_charging = furi_hal_power_is_charging();
bool is_done = furi_hal_power_is_charging_done();

// Set charge voltage limit (V)
furi_hal_power_set_battery_charge_voltage_limit(4.2f);

// Suppress charging (for clean power)
furi_hal_power_suppress_charge_enter();
// ... use clean power ...
furi_hal_power_suppress_charge_exit();
```

### Sleep Management

```c
// Prevent sleep (reference counted)
furi_hal_power_insomnia_enter();
// ... keep device awake ...
furi_hal_power_insomnia_exit();

// Check insomnia level
uint16_t insomnia = furi_hal_power_insomnia_level();  // 0 = can sleep

// Check if sleep is available
if(furi_hal_power_sleep_available()) {
    furi_hal_power_sleep();  // Enter low-power mode
}
```

### Power Control

```c
// Enable OTG (5V output on USB)
if(furi_hal_power_enable_otg()) {
    // OTG enabled successfully
}

// Disable OTG
furi_hal_power_disable_otg();

// Check OTG status
bool otg_on = furi_hal_power_is_otg_enabled();

// Enable 3.3V on external GPIO
furi_hal_power_enable_external_3_3v();

// Power off device
furi_hal_power_off();

// Reset device
furi_hal_power_reset();  // NORETURN
```

---

## GPIO (Digital I/O)

### Available External Pins

| Pin | Usage | Notes |
|-----|-------|-------|
| PC0 | GPIO | Can be USART6_TX |
| PC1 | GPIO | Can be USART6_RX |
| PC3 | GPIO | Can be LPUART1_RX |
| PB2 | GPIO | Can be SPI2_MOSI |
| PB3 | GPIO | Can be SPI2_MISO |
| PA4 | GPIO | Can be SPI1_SS |
| PA6 | GPIO | Can be SPI1_MISO |
| PA7 | GPIO | Can be SPI1_MOSI |

### GPIO Operations

```c
#include <furi_hal_resources.h>

// Pin definitions in furi_hal_resources.h
const GpioPin* pin = &gpio_ext_pc0;

// Configure as output
furi_hal_gpio_init_simple(pin, GpioModeOutputPushPull);

// Set pin high/low
furi_hal_gpio_write(pin, true);   // HIGH
furi_hal_gpio_write(pin, false);  // LOW

// Configure as input
furi_hal_gpio_init(pin, GpioModeInput, GpioPullUp, GpioSpeedLow);

// Read pin state
bool state = furi_hal_gpio_read(pin);

// Analog mode (for ADC)
furi_hal_gpio_init(pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
```

### GPIO Modes

```c
typedef enum {
    GpioModeInput,
    GpioModeOutputPushPull,
    GpioModeOutputOpenDrain,
    GpioModeAltFunctionPushPull,
    GpioModeAltFunctionOpenDrain,
    GpioModeAnalog,
    GpioModeInterruptRise,
    GpioModeInterruptFall,
    GpioModeInterruptRiseFall,
    GpioModeEventRise,
    GpioModeEventFall,
    GpioModeEventRiseFall,
} GpioMode;
```

### GPIO Interrupts

```c
void gpio_interrupt_callback(void* context) {
    // Handle interrupt (keep short!)
}

// Enable interrupt
furi_hal_gpio_init(pin, GpioModeInterruptRise, GpioPullDown, GpioSpeedLow);
furi_hal_gpio_add_int_callback(pin, gpio_interrupt_callback, context);

// Remove interrupt
furi_hal_gpio_remove_int_callback(pin);
```

---

## Bluetooth (BLE)

### Stack Management

```c
// Initialize BT subsystem
furi_hal_bt_init();

// Start radio stack (must be called before BLE operations)
if(furi_hal_bt_start_radio_stack()) {
    // Radio stack started successfully
}

// Get stack type
FuriHalBtStack stack = furi_hal_bt_get_radio_stack();
// Returns: FuriHalBtStackLight or FuriHalBtStackFull

// Check capabilities
bool has_gatt = furi_hal_bt_is_gatt_gap_supported();
bool has_testing = furi_hal_bt_is_testing_supported();
```

### Profile Management

```c
// Start BLE application with profile
FuriHalBleProfileBase* profile = furi_hal_bt_start_app(
    &ble_profile_serial,        // Profile template
    profile_params,              // Profile-specific params (can be NULL)
    &root_security_keys,         // Root keys (can be NULL)
    gap_event_callback,          // Event callback
    context                      // Context for callback
);

// Change profile (restarts CPU2)
profile = furi_hal_bt_change_app(
    &ble_profile_hid,
    params,
    &keys,
    event_cb,
    ctx
);
```

### Advertising

```c
// Start advertising (discoverable)
furi_hal_bt_start_advertising();

// Stop advertising
furi_hal_bt_stop_advertising();

// Check if active (connected or advertising)
bool active = furi_hal_bt_is_active();
```

### Battery Updates

```c
// Update battery level in BLE services
furi_hal_bt_update_battery_level(75);  // 0-100%

// Update power state
furi_hal_bt_update_power_state(true);  // charging
```

### Extra Beacon (Custom Advertisement)

```c
// Set custom beacon data
uint8_t beacon_data[] = {0x01, 0x02, 0x03};
furi_hal_bt_extra_beacon_set_data(beacon_data, sizeof(beacon_data));

// Configure beacon
GapExtraBeaconConfig config = {
    .min_interval_ms = 100,
    .max_interval_ms = 200,
    .tx_power = 6,  // dBm
    .address_type = 0,
};
furi_hal_bt_extra_beacon_set_config(&config);

// Start/stop beacon
furi_hal_bt_extra_beacon_start();
furi_hal_bt_extra_beacon_stop();
```

### BLE Testing

```c
// Start tone transmission
furi_hal_bt_start_tone_tx(channel, power);

// Start packet transmission
furi_hal_bt_start_packet_tx(channel, pattern, datarate);

// Stop and get packet count
uint16_t packets = furi_hal_bt_stop_packet_test();

// Start packet reception
furi_hal_bt_start_packet_rx(channel, datarate);

// Get RSSI
float rssi = furi_hal_bt_get_rssi();
```

---

## USB

### USB Modes

```c
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>
#include <furi_hal_usb_ccid.h>

// Available modes
extern const FuriHalUsbInterface usb_cdc_single;    // VCP (Virtual COM Port)
extern const FuriHalUsbInterface usb_cdc_dual;      // Dual VCP
extern const FuriHalUsbInterface usb_hid;           // HID (Keyboard/Mouse)
extern const FuriHalUsbInterface usb_hid_u2f;       // HID + U2F
extern const FuriHalUsbInterface usb_ccid;          // CCID (Smart Card)
```

### USB Control

```c
// Set USB mode
furi_hal_usb_set_config(&usb_cdc_single, NULL);

// Reinitialize USB
furi_hal_usb_reinit();

// Disable USB
furi_hal_usb_disable();

// Check if USB configured
bool configured = furi_hal_usb_is_locked();
```

### HID Operations

```c
// Press keyboard key
furi_hal_hid_kb_press(HID_KEYBOARD_A);

// Release keyboard key
furi_hal_hid_kb_release(HID_KEYBOARD_A);

// Release all keys
furi_hal_hid_kb_release_all();

// Move mouse
furi_hal_hid_mouse_move(dx, dy);

// Mouse buttons
furi_hal_hid_mouse_press(HID_MOUSE_BTN_LEFT);
furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);

// Mouse wheel
furi_hal_hid_mouse_scroll(delta);
```

---

## Serial Communication (UART)

### Serial Handles

```c
// Available serial ports
FuriHalSerialHandle* handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
FuriHalSerialHandle* handle = furi_hal_serial_control_acquire(FuriHalSerialIdLpuart);
```

### Serial Operations

```c
// Acquire serial port
FuriHalSerialHandle* serial = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
if(serial) {
    // Initialize with baud rate
    furi_hal_serial_init(serial, 115200);

    // Set RX callback
    furi_hal_serial_async_rx_start(serial, rx_callback, context, false);

    // Transmit data
    furi_hal_serial_tx(serial, data, size);

    // Wait for TX complete
    furi_hal_serial_tx_wait_complete(serial);

    // Stop RX
    furi_hal_serial_async_rx_stop(serial);

    // Deinitialize
    furi_hal_serial_deinit(serial);

    // Release port
    furi_hal_serial_control_release(serial);
}
```

### RX Callback

```c
void rx_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context
) {
    if(event == FuriHalSerialRxEventData) {
        size_t bytes_available = furi_hal_serial_async_rx_available(handle);
        uint8_t buffer[128];
        size_t received = furi_hal_serial_async_rx(handle, buffer, sizeof(buffer));
        // Process received data
    }
}
```

---

## SPI

### SPI Bus Management

```c
// Acquire SPI bus
FuriHalSpiBusHandle* spi = furi_hal_spi_bus_handle_alloc(spi_device);

// Acquire with timeout
furi_hal_spi_acquire(spi);

// Transfer data
furi_hal_spi_bus_tx(spi, tx_data, size, timeout_ms);
furi_hal_spi_bus_rx(spi, rx_data, size, timeout_ms);
furi_hal_spi_bus_trx(spi, tx_data, rx_data, size, timeout_ms);

// Release bus
furi_hal_spi_release(spi);

// Free handle
furi_hal_spi_bus_handle_free(spi);
```

---

## I2C

### I2C Operations

```c
// Acquire I2C bus
furi_hal_i2c_acquire(FuriHalI2cBusExternal);

// Check if device is present
bool present = furi_hal_i2c_is_device_ready(
    FuriHalI2cBusExternal,
    device_address,
    timeout_ms
);

// Write data
bool success = furi_hal_i2c_tx(
    FuriHalI2cBusExternal,
    device_address,
    data,
    size,
    timeout_ms
);

// Read data
bool success = furi_hal_i2c_rx(
    FuriHalI2cBusExternal,
    device_address,
    buffer,
    size,
    timeout_ms
);

// Write then read (common pattern)
bool success = furi_hal_i2c_trx(
    FuriHalI2cBusExternal,
    device_address,
    tx_data, tx_size,
    rx_data, rx_size,
    timeout_ms
);

// Release I2C bus
furi_hal_i2c_release(FuriHalI2cBusExternal);
```

---

## ADC (Analog Input)

### ADC Channels

```c
typedef enum {
    FuriHalAdcChannel2V5,       // Internal 2.5V reference
    FuriHalAdcChannelVREFINT,   // Internal reference
    FuriHalAdcChannelTEMPSENSOR, // Internal temperature
    FuriHalAdcChannel16,        // External GPIO PA4
    FuriHalAdcChannel17,        // External GPIO PA6
    FuriHalAdcChannelMAX
} FuriHalAdcChannel;
```

### ADC Operations

```c
// Acquire ADC
furi_hal_adc_acquire();

// Configure pin for analog
furi_hal_gpio_init(pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

// Read ADC value (12-bit: 0-4095)
uint16_t raw = furi_hal_adc_read(FuriHalAdcChannel16);

// Convert to voltage (V)
float voltage = (raw / 4095.0f) * 2.5f;  // Assuming 2.5V reference

// Release ADC
furi_hal_adc_release();
```

---

## Cryptography

### AES Operations

```c
// Initialize crypto
furi_hal_crypto_init();

// Encrypt data (AES-256-CBC)
bool success = furi_hal_crypto_encrypt(
    input_data,
    output_data,
    size  // Must be multiple of 16
);

// Decrypt data
bool success = furi_hal_crypto_decrypt(
    input_data,
    output_data,
    size
);

// Store/load keys
furi_hal_crypto_store_add_key(key_slot, key_data, key_size);
furi_hal_crypto_store_load_key(key_slot, key_data);
```

---

## Speaker & Vibration

### Speaker Control

```c
// Acquire speaker
furi_hal_speaker_acquire(1000);  // Timeout ms

// Start tone (frequency in Hz)
furi_hal_speaker_start(440, 1.0f);  // 440 Hz, full volume

// Stop tone
furi_hal_speaker_stop();

// Release speaker
furi_hal_speaker_release();
```

### Vibration Motor

```c
// Turn on vibro (duty cycle: 0-100%)
furi_hal_vibro_on(true);

// Turn off
furi_hal_vibro_on(false);
```

---

## RGB Backlight

### Light Control

```c
// Set backlight color and brightness
furi_hal_light_set(LightRed, 255);    // 0-255
furi_hal_light_set(LightGreen, 128);
furi_hal_light_set(LightBlue, 64);

// Set backlight (display backlight)
furi_hal_light_set(LightBacklight, brightness);

// Blink sequence
furi_hal_light_blink_start(LightRed, 500, 50);   // On 500ms, off 500ms, brightness 50%
furi_hal_light_blink_stop();
```

---

## Random Number Generation

```c
// Get random number
uint32_t random = furi_hal_random_get();

// Fill buffer with random bytes
furi_hal_random_fill_buf(buffer, size);
```

---

## RTC & Backup Registers

### RTC Operations

```c
// Get datetime
FuriHalRtcDateTime datetime;
furi_hal_rtc_get_datetime(&datetime);

// Set datetime
furi_hal_rtc_set_datetime(&datetime);

// Validate datetime
bool valid = furi_hal_rtc_validate_datetime(&datetime);

// Get timestamp (seconds since boot)
uint32_t timestamp = furi_hal_rtc_get_timestamp();
```

### Backup Registers

```c
// Store persistent data (survives reboot, not power loss)
furi_hal_rtc_set_register(FuriHalRtcRegisterHeader, value);

// Read persistent data
uint32_t value = furi_hal_rtc_get_register(FuriHalRtcRegisterHeader);
```

---

## Version & Info

### Device Information

```c
// Get device name
const char* name = furi_hal_version_get_name_ptr();

// Get device serial
size_t size = furi_hal_version_get_device_name_ptr();

// Get hardware version
uint8_t hw_ver = furi_hal_version_get_hw_version();

// Get hardware target
uint8_t hw_target = furi_hal_version_get_hw_target();

// Get hardware region
FuriHalRegion region = furi_hal_version_get_hw_region();

// Get firmware version
const Version* fw_ver = furi_hal_version_get_firmware_version();
```

---

## Region Settings

### Frequency Region Configuration

```c
// Get current region
FuriHalRegion region = furi_hal_region_get();

// Check if region is provisioned
bool provisioned = furi_hal_region_is_provisioned();

// Get region name
const char* name = furi_hal_region_get_name();

// Get bands for region (for SubGHz)
const FuriHalRegionBands* bands = furi_hal_region_get_bands();
```

---

## Interrupt Management

### ISR Priorities

FuriHal uses 8 interrupt priority levels:

```c
// Priority levels (0 = highest, 15 = lowest)
#define FURI_HAL_INTERRUPT_PRIORITY_HIGHEST      (0)   // Critical hardware
#define FURI_HAL_INTERRUPT_PRIORITY_HIGH         (5)   // Time-sensitive
#define FURI_HAL_INTERRUPT_PRIORITY_NORMAL       (10)  // Standard ISRs
#define FURI_HAL_INTERRUPT_PRIORITY_LOW          (15)  // Can be delayed
```

### Critical Sections

```c
// Disable interrupts
FURI_CRITICAL_ENTER();

// Critical code here (keep minimal!)

// Enable interrupts
FURI_CRITICAL_EXIT();
```

---

## Best Practices

### Resource Management

1. **Always pair acquire/release**:
   ```c
   furi_hal_i2c_acquire(bus);
   // ... use I2C ...
   furi_hal_i2c_release(bus);
   ```

2. **Check return values**:
   ```c
   if(!furi_hal_i2c_tx(bus, addr, data, size, timeout)) {
       // Handle error
   }
   ```

3. **Use timeouts**:
   ```c
   furi_hal_spi_bus_tx(spi, data, size, 1000);  // 1 second timeout
   ```

### Power Management

1. **Enable insomnia when needed**:
   ```c
   furi_hal_power_insomnia_enter();
   // Keep device awake during operation
   furi_hal_power_insomnia_exit();
   ```

2. **Monitor battery before high-power operations**:
   ```c
   if(furi_hal_power_get_pct() < 10) {
       // Warn user about low battery
   }
   ```

### Thread Safety

1. **Most HAL functions are NOT thread-safe** - Use FURI mutexes
2. **Acquire resources in consistent order** - Prevent deadlocks
3. **Keep ISR handlers short** - Defer work to threads

### GPIO

1. **Initialize before use**:
   ```c
   furi_hal_gpio_init_simple(pin, GpioModeOutputPushPull);
   ```

2. **Reset to safe state when done**:
   ```c
   furi_hal_gpio_init_simple(pin, GpioModeAnalog);  // High-Z
   ```

### External Peripherals

1. **Enable 3.3V for external GPIO**:
   ```c
   furi_hal_power_enable_external_3_3v();
   ```

2. **Check device presence before operations**:
   ```c
   if(furi_hal_i2c_is_device_ready(bus, addr, timeout)) {
       // Device present, proceed
   }
   ```

---

## Hardware-Specific Documentation

For detailed module-specific guides, see:

- `documentation/hardware/gpio.md` - GPIO pinout and examples
- `documentation/hardware/bluetooth.md` - BLE profiles and protocols
- `documentation/hardware/usb.md` - USB device implementation
- `documentation/hardware/nfc.md` - NFC protocols and ST25R3916
- `documentation/hardware/subghz.md` - Sub-GHz radio and CC1101
- `documentation/hardware/infrared.md` - IR protocols and hardware
- `documentation/hardware/rfid.md` - 125kHz RFID reader

---

## Source Files

**Headers**: `targets/furi_hal_include/`
**Implementation**: `targets/f7/furi_hal/`

Key header files:
- `furi_hal.h` - Main HAL header (includes all modules)
- `furi_hal_resources.h` - Pin definitions and GPIO resources
- `furi_hal_bus.h` - Peripheral bus management

---

## Examples

### Complete GPIO Example

```c
#include <furi_hal.h>

void gpio_example(void) {
    const GpioPin* led = &gpio_ext_pa7;

    // Enable external 3.3V
    furi_hal_power_enable_external_3_3v();

    // Configure as output
    furi_hal_gpio_init_simple(led, GpioModeOutputPushPull);

    // Blink LED
    for(int i = 0; i < 10; i++) {
        furi_hal_gpio_write(led, true);
        furi_delay_ms(500);
        furi_hal_gpio_write(led, false);
        furi_delay_ms(500);
    }

    // Reset pin
    furi_hal_gpio_init_simple(led, GpioModeAnalog);

    // Disable external 3.3V
    furi_hal_power_disable_external_3_3v();
}
```

### Complete I2C Example

```c
#include <furi_hal.h>

#define DEVICE_ADDR 0x48

bool i2c_read_sensor(uint8_t* data) {
    bool success = false;

    // Enable external power
    furi_hal_power_enable_external_3_3v();
    furi_delay_ms(10);  // Power stabilization

    // Acquire I2C bus
    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    // Check device presence
    if(furi_hal_i2c_is_device_ready(FuriHalI2cBusExternal, DEVICE_ADDR, 100)) {
        // Read register
        uint8_t reg = 0x00;
        success = furi_hal_i2c_trx(
            FuriHalI2cBusExternal,
            DEVICE_ADDR,
            &reg, 1,      // Write register address
            data, 2,      // Read 2 bytes
            100           // 100ms timeout
        );
    }

    // Release I2C
    furi_hal_i2c_release(FuriHalI2cBusExternal);

    // Disable external power
    furi_hal_power_disable_external_3_3v();

    return success;
}
```

---

## Related Documentation

- [HAL_SUMMARY.md](HAL_SUMMARY.md) - Quick HAL overview
- [FuriCoreAPI.md](FuriCoreAPI.md) - FURI Core API
- [FuriQuickReference.md](FuriQuickReference.md) - Quick reference
- [HardwareTargets.md](HardwareTargets.md) - Hardware variants
- [ExpansionModules.md](ExpansionModules.md) - External modules
- [fbt.md](fbt.md) - Build system

---

**Oracle Documentation** - Complete hardware knowledge transfer accomplished.
