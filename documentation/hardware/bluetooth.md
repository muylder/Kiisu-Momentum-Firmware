# Bluetooth Hardware Guide

Complete guide to Bluetooth Low Energy (BLE) on Flipper Zero.

## Overview

Flipper Zero features **dual-core Bluetooth** using the STM32WB55 microcontroller:

- **CPU1 (Cortex-M4)**: Main application processor running FreeRTOS and firmware
- **CPU2 (Cortex-M0+)**: Dedicated BLE stack processor (STM Wireless Co-Processor)

**Stack Versions**:
- **BLE Light**: Optimized for size, limited profiles
- **BLE Full**: Complete stack with all profiles

**Bluetooth Version**: 5.0
**Supported Profiles**: Serial Port, HID (Keyboard/Mouse), Custom GATT services
**Range**: ~10m (typical indoor), up to 50m (line of sight)

---

## Architecture

### Dual-Core Communication

```
┌──────────────────────┐
│      CPU1 (M4)       │  Application Firmware
│   ┌──────────────┐   │  - Apps
│   │ BLE HAL API  │   │  - Services
│   └──────┬───────┘   │  - User code
│          │           │
│   ┌──────▼───────┐   │
│   │ BLE Glue     │   │  Inter-Processor Communication
│   └──────┬───────┘   │
└──────────┼───────────┘
           │ IPCC (Mailbox)
┌──────────▼───────────┐
│   ┌──────────────┐   │
│   │  BLE Stack   │   │  STM Wireless Stack
│   │ (Link Layer) │   │  - GAP/GATT
│   └──────────────┘   │  - Security
│      CPU2 (M0+)      │  - Radio control
└──────────────────────┘
```

---

## BLE Stack Management

### Initialize Bluetooth

```c
#include <furi_hal_bt.h>

// Initialize BT subsystem (called during firmware init)
furi_hal_bt_init();

// Start radio stack (must be called before BLE operations)
if(!furi_hal_bt_start_radio_stack()) {
    FURI_LOG_E("BT", "Failed to start radio stack");
    return false;
}

// Check stack type
FuriHalBtStack stack = furi_hal_bt_get_radio_stack();
if(stack == FuriHalBtStackFull) {
    FURI_LOG_I("BT", "Full stack loaded");
} else if(stack == FuriHalBtStackLight) {
    FURI_LOG_I("BT", "Light stack loaded");
}
```

### Stack Capabilities

```c
// Check if GAT/GATT supported
bool has_gatt = furi_hal_bt_is_gatt_gap_supported();

// Check if testing mode supported
bool has_testing = furi_hal_bt_is_testing_supported();

// Check if stack is alive (CPU2 responding)
bool alive = furi_hal_bt_is_alive();
```

---

## BLE Profiles

### Available Profiles

Flipper firmware includes several built-in profiles:

```c
// Serial Port Profile (SPP-like over GATT)
extern const FuriHalBleProfileTemplate ble_profile_serial;

// HID Keyboard/Mouse
extern const FuriHalBleProfileTemplate ble_profile_hid;

// Battery Service
extern const FuriHalBleProfileTemplate ble_profile_battery;
```

### Starting a Profile

```c
void gap_event_callback(GapEvent event, void* context) {
    MyApp* app = context;

    switch(event.type) {
        case GapEventTypeConnected:
            FURI_LOG_I("BT", "Connected");
            app->connected = true;
            break;

        case GapEventTypeDisconnected:
            FURI_LOG_I("BT", "Disconnected");
            app->connected = false;
            break;

        case GapEventTypePinCodeShow:
            FURI_LOG_I("BT", "PIN Code: %06lu", event.data.pin_code);
            break;

        case GapEventTypePinCodeVerify:
            // Verify PIN entered by user
            FURI_LOG_I("BT", "Verify PIN: %06lu", event.data.pin_code);
            break;

        // ... other events ...
    }
}

// Start profile
FuriHalBleProfileBase* profile = furi_hal_bt_start_app(
    &ble_profile_serial,        // Profile template
    FuriHalBleProfileParamsNone, // Profile-specific params
    NULL,                        // Root security keys (optional)
    gap_event_callback,          // Event callback
    app_context                  // Context passed to callback
);

if(!profile) {
    FURI_LOG_E("BT", "Failed to start profile");
}
```

### Changing Profiles

```c
// Change to different profile (restarts CPU2)
FuriHalBleProfileBase* hid_profile = furi_hal_bt_change_app(
    &ble_profile_hid,
    FuriHalBleProfileParamsNone,
    NULL,
    gap_event_callback,
    app_context
);
```

### Profile Parameters

Some profiles accept configuration parameters:

```c
// Example: HID profile with custom parameters
FuriHalBleProfileParams hid_params = {
    .hid_params = {
        .device_name = "Flipper Keyboard",
        .mac_addr = NULL,  // Auto-generate
    }
};

profile = furi_hal_bt_start_app(
    &ble_profile_hid,
    hid_params,
    NULL,
    gap_event_callback,
    context
);
```

---

## Advertising

### Basic Advertising

```c
// Start advertising (device becomes discoverable)
furi_hal_bt_start_advertising();

// Check if active (connected or advertising)
bool active = furi_hal_bt_is_active();

// Stop advertising
furi_hal_bt_stop_advertising();
```

### Custom Advertisement (Extra Beacon)

Flipper supports a **second simultaneous advertisement** (Extra Beacon) for custom data:

```c
#include <extra_beacon.h>

// Set custom beacon data (max 31 bytes)
uint8_t beacon_data[] = {
    0x02, 0x01, 0x06,  // Flags
    0x03, 0x03, 0xAA, 0xFE,  // Complete list of 16-bit UUIDs
    // ... your custom data ...
};

bool success = furi_hal_bt_extra_beacon_set_data(beacon_data, sizeof(beacon_data));

// Configure beacon parameters
GapExtraBeaconConfig config = {
    .min_interval_ms = 100,     // Min advertising interval
    .max_interval_ms = 200,     // Max advertising interval
    .tx_power = 6,              // TX power in dBm (-40 to +6)
    .address_type = 0,          // Public address
};

success = furi_hal_bt_extra_beacon_set_config(&config);

// Start extra beacon
if(furi_hal_bt_extra_beacon_start()) {
    FURI_LOG_I("BT", "Extra beacon started");
}

// Check if running
bool running = furi_hal_bt_extra_beacon_is_active();

// Stop extra beacon
furi_hal_bt_extra_beacon_stop();
```

**Use Cases for Extra Beacon**:
- iBeacon/Eddystone broadcasting
- Custom proximity beacons
- Simultaneous advertising while connected
- Location tracking applications

---

## GAP Events

### Event Types

```c
typedef enum {
    GapEventTypeConnected,         // Device connected
    GapEventTypeDisconnected,      // Device disconnected
    GapEventTypeStartAdvertising,  // Advertising started
    GapEventTypeStopAdvertising,   // Advertising stopped
    GapEventTypePinCodeShow,       // Show PIN to user
    GapEventTypePinCodeVerify,     // Verify PIN from user
    GapEventTypeUpdateMTU,         // MTU size updated
    GapEventTypeBeaconStart,       // Extra beacon started
    GapEventTypeBeaconStop,        // Extra beacon stopped
} GapEventType;
```

### Handling Events

```c
void gap_event_callback(GapEvent event, void* context) {
    MyBleApp* app = context;

    switch(event.type) {
        case GapEventTypeConnected:
            FURI_LOG_I("BLE", "Connected");
            app->is_connected = true;
            // Get connection parameters
            FURI_LOG_I("BLE", "MTU: %d", event.data.mtu);
            break;

        case GapEventTypeDisconnected:
            FURI_LOG_I("BLE", "Disconnected: reason=%d", event.data.disconnect_reason);
            app->is_connected = false;
            // Auto-restart advertising if desired
            furi_hal_bt_start_advertising();
            break;

        case GapEventTypePinCodeShow:
            // Display PIN code to user
            FURI_LOG_I("BLE", "Show PIN: %06lu", event.data.pin_code);
            // Update GUI with PIN
            break;

        case GapEventTypePinCodeVerify:
            // Ask user to verify PIN
            FURI_LOG_I("BLE", "Verify PIN: %06lu", event.data.pin_code);
            // Show confirmation dialog
            break;

        case GapEventTypeUpdateMTU:
            FURI_LOG_I("BLE", "MTU updated: %d", event.data.mtu);
            app->mtu_size = event.data.mtu;
            break;
    }
}
```

---

## Security & Pairing

### Security Keys

BLE supports secure pairing with persistent bonding:

```c
// Root security keys (persistent across reboots)
GapRootSecurityKeys root_keys = {0};

// Load keys from storage
// ... load from file or secure storage ...

// Start app with security keys
profile = furi_hal_bt_start_app(
    &ble_profile_serial,
    FuriHalBleProfileParamsNone,
    &root_keys,  // Pass keys
    gap_event_callback,
    context
);
```

### White List Management

```c
// Clear white list (unpair all devices)
bool success = furi_hal_bt_clear_white_list();

// Key storage change callback
void key_storage_changed(uint8_t* changed_data, uint16_t size, void* context) {
    FURI_LOG_I("BT", "Keys changed: %d bytes", size);
    // Save to persistent storage
}

furi_hal_bt_set_key_storage_change_callback(key_storage_changed, context);
```

### Accessing Key Storage

```c
// Get key storage buffer
uint8_t* key_buffer;
uint16_t key_size;
furi_hal_bt_get_key_storage_buff(&key_buffer, &key_size);

// Acquire SRAM2 semaphore (required for access)
furi_hal_bt_nvm_sram_sem_acquire();

// Read/Write keys
memcpy(my_buffer, key_buffer, key_size);

// Release semaphore
furi_hal_bt_nvm_sram_sem_release();
```

---

## Battery Status Updates

BLE services can report battery status:

```c
// Update battery level (0-100%)
uint8_t battery_pct = furi_hal_power_get_pct();
furi_hal_bt_update_battery_level(battery_pct);

// Update charging state
bool is_charging = furi_hal_power_is_charging();
furi_hal_bt_update_power_state(is_charging);
```

**Best Practice**: Update battery status periodically or on change:

```c
void update_ble_battery(void* context) {
    static uint8_t last_pct = 0;
    static bool last_charging = false;

    uint8_t pct = furi_hal_power_get_pct();
    bool charging = furi_hal_power_is_charging();

    if(pct != last_pct) {
        furi_hal_bt_update_battery_level(pct);
        last_pct = pct;
    }

    if(charging != last_charging) {
        furi_hal_bt_update_power_state(charging);
        last_charging = charging;
    }
}
```

---

## BLE Testing & Debugging

### Transmit Testing

```c
// Start continuous tone transmission (for testing)
furi_hal_bt_start_tone_tx(
    channel,  // 0-39 (BLE channels)
    power     // 0-7 (TX power level)
);

// Stop tone
furi_hal_bt_stop_tone_tx();
```

### Packet Testing

```c
// Start packet transmission
furi_hal_bt_start_packet_tx(
    channel,   // 0-39
    pattern,   // 0-7 (PRBS9, 11110000, 10101010, etc.)
    datarate   // 1=1Mbps, 2=2Mbps, 3=500kbps, 4=125kbps
);

// Stop and get packet count
uint16_t packets_sent = furi_hal_bt_stop_packet_test();
FURI_LOG_I("BT", "Packets sent: %d", packets_sent);
```

### Reception Testing

```c
// Start packet reception
furi_hal_bt_start_packet_rx(
    channel,   // 0-39
    datarate   // 1-4
);

// Stop and get packet count
uint16_t packets_received = furi_hal_bt_stop_packet_test();
FURI_LOG_I("BT", "Packets received: %d", packets_received);
```

### RSSI Measurement

```c
// Start listening
furi_hal_bt_start_rx(channel);

// Get RSSI
float rssi_dbm = furi_hal_bt_get_rssi();
FURI_LOG_I("BT", "RSSI: %.1f dBm", rssi_dbm);

// Stop listening
furi_hal_bt_stop_rx();
```

---

## CPU2 Management

### Core2 Control

```c
// Lock CPU2 (prevent state changes)
furi_hal_bt_lock_core2();

// ... perform operations ...

// Unlock CPU2
furi_hal_bt_unlock_core2();
```

### Reinitialize Core2

```c
// Reinitialize CPU2 (useful for power management)
furi_hal_bt_reinit();
```

### Mode Switching

```c
typedef enum {
    BleGlueC2ModeUnknown = 0,
    BleGlueC2ModeFUS,       // Firmware Update Service
    BleGlueC2ModeStack,     // BLE Stack running
} BleGlueC2Mode;

// Switch to specific mode
bool success = furi_hal_bt_ensure_c2_mode(BleGlueC2ModeStack);
```

---

## Custom GATT Services

### Creating Custom Service

While Flipper doesn't expose direct GATT service creation in HAL, you can create profiles:

```c
// Define profile template
const FuriHalBleProfileTemplate my_custom_profile = {
    .advertise_name = "MyDevice",
    .advertise_name_len = 8,
    .address = NULL,  // Auto-generate
    .start = my_profile_start,
    .stop = my_profile_stop,
    .app_callback = my_profile_callback,
};

// Profile callbacks
static bool my_profile_start(FuriHalBleProfileBase* profile, void* context) {
    // Initialize GATT services
    // Register characteristics
    return true;
}

static void my_profile_stop(FuriHalBleProfileBase* profile) {
    // Clean up GATT services
}

static bool my_profile_callback(
    FuriHalBleProfileBase* profile,
    FuriHalBleProfileEvent event,
    void* data,
    size_t data_len
) {
    // Handle profile events
    return true;
}
```

---

## Common BLE Patterns

### Simple Serial Communication

```c
typedef struct {
    bool connected;
    FuriHalBleProfileBase* profile;
} BleSerialApp;

void ble_serial_init(BleSerialApp* app) {
    // Start serial profile
    app->profile = furi_hal_bt_start_app(
        &ble_profile_serial,
        FuriHalBleProfileParamsNone,
        NULL,
        gap_event_callback,
        app
    );

    // Start advertising
    furi_hal_bt_start_advertising();
}

void ble_serial_send(BleSerialApp* app, const uint8_t* data, size_t len) {
    if(app->connected && app->profile) {
        // Send data via profile
        // Implementation depends on profile API
    }
}
```

### BLE with Power Management

```c
void ble_sleep_prepare(void) {
    // Stop advertising
    furi_hal_bt_stop_advertising();

    // Reinit for low power
    furi_hal_bt_reinit();
}

void ble_wakeup_restore(void) {
    // Restart radio stack if needed
    furi_hal_bt_start_radio_stack();

    // Restart advertising
    furi_hal_bt_start_advertising();
}
```

---

## Debugging

### Dump BLE State

```c
FuriString* buffer = furi_string_alloc();
furi_hal_bt_dump_state(buffer);
FURI_LOG_I("BT", "%s", furi_string_get_cstr(buffer));
furi_string_free(buffer);
```

### Check BLE Activity

```c
// Check if connected or advertising
bool active = furi_hal_bt_is_active();

// Check if stack is responding
bool alive = furi_hal_bt_is_alive();

// Get transmitted packet count (testing mode)
uint32_t tx_packets = furi_hal_bt_get_transmitted_packets();
```

---

## Troubleshooting

### Common Issues

**BLE not starting**:
- Check if radio stack started: `furi_hal_bt_start_radio_stack()`
- Verify CPU2 is alive: `furi_hal_bt_is_alive()`
- Check for firmware update issues

**Connection drops**:
- Monitor battery level (BLE needs sufficient power)
- Check for interference (2.4 GHz congestion)
- Verify connection parameters
- Increase supervision timeout

**Pairing fails**:
- Clear white list: `furi_hal_bt_clear_white_list()`
- Check security requirements
- Verify PIN code handling

**Poor range**:
- Check battery level (low battery reduces TX power)
- Verify antenna is not blocked
- Reduce TX power for testing: `config.tx_power = -20`

---

## Best Practices

### Power Management

1. **Stop advertising when not needed**:
   ```c
   furi_hal_bt_stop_advertising();
   ```

2. **Use low power modes**:
   ```c
   furi_hal_bt_reinit();  // Prepare for sleep
   ```

3. **Monitor battery**:
   ```c
   uint8_t pct = furi_hal_power_get_pct();
   if(pct < 10) {
       // Reduce BLE activity
   }
   ```

### Security

1. **Always use pairing for sensitive data**
2. **Store security keys securely**
3. **Clear white list when resetting device**
4. **Implement PIN code verification**

### Performance

1. **Negotiate higher MTU** for better throughput
2. **Use connection interval optimization**
3. **Batch data transfers** when possible
4. **Monitor RSSI** for link quality

---

## BLE Specifications

### Hardware

- **Chip**: STM32WB55CEU6
- **Bluetooth**: 5.0 certified
- **TX Power**: -40 to +6 dBm
- **RX Sensitivity**: -96 dBm @ 1 Mbps
- **Channels**: 40 (3 advertising, 37 data)
- **Data Rates**: 125 kbps, 500 kbps, 1 Mbps, 2 Mbps

### Supported Features

- ✓ LE Advertising
- ✓ LE Scan
- ✓ LE Connection (Central & Peripheral)
- ✓ LE Data Length Extension
- ✓ LE Privacy
- ✓ LE Secure Connections
- ✓ LE 2M PHY
- ✗ LE Coded PHY (not supported)
- ✗ LE Audio (not supported)

---

## Related Documentation

- [FuriHalAPI.md](../FuriHalAPI.md) - Complete HAL API
- [furi_hal_bt.h](../../targets/furi_hal_include/furi_hal_bt.h) - BT API header
- STM32WB Wireless Stack documentation
- Bluetooth SIG specifications

---

**Oracle Documentation** - Bluetooth mastery complete.
