# Sub-GHz Radio Hardware Guide

Complete guide to Sub-GHz wireless communications on Flipper Zero.

## Overview

Flipper Zero features a **Texas Instruments CC1101** Sub-GHz radio transceiver for wireless communication on ISM bands:

**Radio IC**: CC1101
**Frequency Bands**:
- 300-348 MHz
- 350-467 MHz (enhanced, hardware limit)
- 778-928 MHz

**Max Output Power**: +10 dBm (10 mW) with PA
**Sensitivity**: -112 dBm @ 1.2 kbps
**Modulations**: ASK/OOK, 2-FSK, 4-FSK, GFSK, MSK

**Common Uses**:
- Remote controls (car keys, garage doors, gates)
- Wireless sensors (weather stations, tire pressure)
- IoT devices (LoRa, Sigfox)
- Security systems
- ISM band protocols

---

## Hardware Architecture

### CC1101 Connection

```
┌─────────────┐
│   STM32WB55 │
│             │
│    SPI2     ├───┐
│             │   │
│    GPIO     ├───┤
└─────────────┘   │
                  │
          ┌───────▼────────┐
          │   CC1101       │
          │   Transceiver  │
          └───────┬────────┘
                  │
          ┌───────▼────────┐
          │  RF Switch     │
          │  (Antenna Sel) │
          └───────┬────────┘
                  │
          ┌───────▼────────┐
          │  Antenna       │
          │  (PCB Trace)   │
          └────────────────┘
```

**Pins**:
- `gpio_cc1101_g0` (PA1): GDO0 - Data/IRQ
- `gpio_subghz_cs` (PD0): SPI chip select
- `gpio_rf_sw_0` (PC4): RF path switch

---

## Frequency Management

### Supported Bands

The CC1101 supports three main bands with Flipper-specific enhancements:

| Band | Official Range | Flipper Range | Path | Common Uses |
|------|---------------|---------------|------|-------------|
| 1 | 300-348 MHz | 300-350 MHz | `FuriHalSubGhzPath315` | Tire pressure, medical |
| 2 | 387-464 MHz | 350-467 MHz | `FuriHalSubGhzPath433` | Remote controls, ISM |
| 3 | 779-928 MHz | 778-928 MHz | `FuriHalSubGhzPath868` | ISM, LoRa, sensors |

### Setting Frequency

```c
#include <furi_hal_subghz.h>

// Initialize SubGHz
furi_hal_subghz_init();

// Set frequency (Hz) and automatically select path
uint32_t actual_freq = furi_hal_subghz_set_frequency_and_path(433920000);
FURI_LOG_I("SubGHz", "Frequency set to: %lu Hz", actual_freq);

// Or set frequency and path manually
furi_hal_subghz_set_frequency(315000000);
furi_hal_subghz_set_path(FuriHalSubGhzPath315);
```

### Frequency Validation

```c
// Check if frequency is valid
if(!furi_hal_subghz_is_frequency_valid(433920000)) {
    FURI_LOG_E("SubGHz", "Invalid frequency");
    return false;
}

// Check if transmission is allowed (regional restrictions)
if(!furi_hal_subghz_is_tx_allowed(433920000)) {
    FURI_LOG_W("SubGHz", "TX not allowed on this frequency");
    return false;
}

// Get detailed TX permission status
SubGhzTx tx_status = furi_hal_subghz_check_tx(433920000);
// Returns: SubGhzTxOk, SubGhzTxNotAllowed, SubGhzTxBlocked, etc.
```

---

## RF Paths

### Antenna Path Selection

Flipper has three RF paths for optimal performance across frequency bands:

```c
typedef enum {
    FuriHalSubGhzPathIsolate,  // Disconnect antenna (safe mode)
    FuriHalSubGhzPath433,      // 387-467 MHz (optimal for 433 MHz)
    FuriHalSubGhzPath315,      // 300-350 MHz (optimal for 315 MHz)
    FuriHalSubGhzPath868,      // 779-928 MHz (optimal for 868/915 MHz)
} FuriHalSubGhzPath;
```

**Usage**:

```c
// Isolate antenna (safe, no RF)
furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);

// Set path for 433 MHz
furi_hal_subghz_set_path(FuriHalSubGhzPath433);

// Or use automatic path selection
furi_hal_subghz_set_frequency_and_path(433920000);  // Auto-selects Path433
```

---

## Modulation Presets

### Loading Presets

CC1101 uses register configurations called "presets" for different modulations:

```c
// Load preset from library
#include <lib/subghz/devices/preset.h>

// Common presets:
// - AM650 (ASK OOK, 650 µs pulse)
// - AM270 (ASK OOK, 270 µs pulse)
// - FM238 (2-FSK, 2.38 kHz deviation)
// - FM476 (2-FSK, 4.76 kHz deviation)

// Load preset
furi_hal_subghz_load_custom_preset(preset_data);
```

### Custom Register Configuration

```c
// Register array format: {address, value, address, value, ..., 0x00, 0x00}
const uint8_t custom_preset[] = {
    0x00, 0x2E,  // IOCFG2: GDO2 output pin config
    0x01, 0x2E,  // IOCFG1: GDO1 output pin config
    0x02, 0x06,  // IOCFG0: GDO0 output pin config (async serial data)
    0x03, 0x47,  // FIFOTHR: FIFO threshold
    0x04, 0xD3,  // SYNC1: Sync word, high byte
    0x05, 0x91,  // SYNC0: Sync word, low byte
    0x06, 0xFF,  // PKTLEN: Packet length
    0x07, 0x04,  // PKTCTRL1: Packet automation control
    0x08, 0x05,  // PKTCTRL0: Packet automation control
    // ... more registers ...
    0x00, 0x00   // Terminator
};

furi_hal_subghz_load_registers(custom_preset);
```

### Power Table (PATABLE)

```c
// PATABLE controls output power (8 levels)
// Values are CC1101-specific, see datasheet

// Example: +10 dBm output
const uint8_t patable_10dbm[8] = {
    0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0
};

furi_hal_subghz_load_patable(patable_10dbm);

// Example: 0 dBm output (moderate power)
const uint8_t patable_0dbm[8] = {
    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60
};

furi_hal_subghz_load_patable(patable_0dbm);
```

---

## Basic Radio Operations

### State Machine

CC1101 has several operational states:

```c
// Sleep mode (lowest power)
furi_hal_subghz_sleep();

// Idle mode (ready to TX/RX)
furi_hal_subghz_idle();

// Receive mode
furi_hal_subghz_rx();

// Transmit mode (checks regional restrictions)
if(furi_hal_subghz_tx()) {
    // TX enabled
} else {
    // TX not allowed
}
```

### RSSI & Link Quality

```c
// Get RSSI (Received Signal Strength Indicator) in dBm
float rssi = furi_hal_subghz_get_rssi();
FURI_LOG_I("SubGHz", "RSSI: %.1f dBm", rssi);

// Get LQI (Link Quality Indicator)
uint8_t lqi = furi_hal_subghz_get_lqi();
FURI_LOG_I("SubGHz", "LQI: %d", lqi);
```

---

## Packet Mode (FIFO)

### Transmitting Packets

```c
// Configure packet mode
furi_hal_subghz_idle();

// Load preset and frequency
furi_hal_subghz_load_custom_preset(preset_am650);
furi_hal_subghz_set_frequency_and_path(433920000);

// Write packet to FIFO (max 64 bytes)
uint8_t packet[] = {0x01, 0x02, 0x03, 0x04, 0x05};
furi_hal_subghz_write_packet(packet, sizeof(packet));

// Transmit
if(furi_hal_subghz_tx()) {
    // Wait for transmission to complete
    furi_delay_ms(50);

    // Return to idle
    furi_hal_subghz_idle();
}
```

### Receiving Packets

```c
// Configure receive mode
furi_hal_subghz_idle();
furi_hal_subghz_load_custom_preset(preset_am650);
furi_hal_subghz_set_frequency_and_path(433920000);

// Start receiving
furi_hal_subghz_rx();

// Poll for data
while(true) {
    if(furi_hal_subghz_rx_pipe_not_empty()) {
        uint8_t buffer[64];
        uint8_t size;

        // Read packet
        furi_hal_subghz_read_packet(buffer, &size);

        // Check CRC
        if(furi_hal_subghz_is_rx_data_crc_valid()) {
            FURI_LOG_I("SubGHz", "Received %d bytes", size);
            // Process packet
        } else {
            FURI_LOG_W("SubGHz", "CRC error");
        }
    }

    furi_delay_ms(10);
}

// Clean up
furi_hal_subghz_idle();
```

### FIFO Management

```c
// Flush RX FIFO
furi_hal_subghz_flush_rx();

// Flush TX FIFO
furi_hal_subghz_flush_tx();
```

---

## Async Mode (Direct Modulation)

Async mode allows direct control of the radio for custom protocols without using the CC1101 FIFO.

### Async Receive

```c
#include <toolbox/level_duration.h>

void subghz_rx_callback(bool level, uint32_t duration, void* context) {
    // This callback is called for each edge transition
    // level: true = HIGH, false = LOW
    // duration: microseconds since last transition

    MyApp* app = context;

    // Decode signal timing
    if(duration > 1000) {
        // Long pulse
        app->decode_long_pulse(level);
    } else {
        // Short pulse
        app->decode_short_pulse(level);
    }
}

void start_async_receive(MyApp* app) {
    // Configure radio
    furi_hal_subghz_idle();
    furi_hal_subghz_load_custom_preset(preset_am650);
    furi_hal_subghz_set_frequency_and_path(433920000);

    // Start async RX (captures edge timings)
    furi_hal_subghz_start_async_rx(subghz_rx_callback, app);

    // Radio will call callback on each edge
    // Keep running until ready to stop

    // Stop async RX
    furi_hal_subghz_stop_async_rx();
    furi_hal_subghz_idle();
}
```

### Async Transmit

```c
typedef struct {
    uint32_t* timings;  // Array of pulse durations (µs)
    size_t count;       // Number of timings
    size_t index;       // Current index
} TxContext;

LevelDuration subghz_tx_callback(void* context) {
    TxContext* ctx = context;

    if(ctx->index >= ctx->count) {
        // End of transmission
        return level_duration_reset();
    }

    // Get next timing
    uint32_t duration = ctx->timings[ctx->index];
    bool level = (ctx->index % 2) == 0;  // Alternate HIGH/LOW
    ctx->index++;

    return level_duration_make(level, duration);
}

void start_async_transmit(TxContext* ctx) {
    // Configure radio
    furi_hal_subghz_idle();
    furi_hal_subghz_load_custom_preset(preset_am650);
    furi_hal_subghz_set_frequency_and_path(433920000);

    // Reset context
    ctx->index = 0;

    // Start async TX
    if(furi_hal_subghz_start_async_tx(subghz_tx_callback, ctx)) {
        // Wait for transmission to complete
        while(!furi_hal_subghz_is_async_tx_complete()) {
            furi_delay_ms(1);
        }

        // Stop transmission
        furi_hal_subghz_stop_async_tx();
    }

    furi_hal_subghz_idle();
}
```

---

## Advanced Features

### Async Mirror Pin

Debug feature to mirror RX/TX signal to external GPIO:

```c
// Enable signal mirroring to GPIO
const GpioPin* mirror_pin = &gpio_ext_pa7;
furi_hal_subghz_set_async_mirror_pin(mirror_pin);

// Disable mirroring
furi_hal_subghz_set_async_mirror_pin(NULL);
```

### Rolling Code Counter

For protocols with rolling codes (KeeLoq, etc.):

```c
// Get current counter multiplier
int32_t mult = furi_hal_subghz_get_rolling_counter_mult();

// Set counter multiplier (-50, -10, -1, 0, 1, 10, 50)
furi_hal_subghz_set_rolling_counter_mult(1);   // Increment by 1
furi_hal_subghz_set_rolling_counter_mult(-1);  // Decrement by 1
furi_hal_subghz_set_rolling_counter_mult(0);   // No change (replay)
```

### Debug State

```c
// Dump CC1101 state to console
furi_hal_subghz_dump_state();
```

---

## Power Management

### Sleep Modes

```c
// Full initialization (from cold start)
furi_hal_subghz_init();

// Sleep mode (lowest power, regs preserved)
furi_hal_subghz_sleep();

// Wake up (return to idle)
furi_hal_subghz_idle();

// Shutdown (regs lost, requires full reset)
furi_hal_subghz_shutdown();

// Reset (regs lost, requires reconfiguration)
furi_hal_subghz_reset();
```

---

## Frequency Configuration

### User-Defined Frequencies

Frequencies can be configured via SD card:

**File**: `SD:///subghz/assets/setting_user`

```
Filetype: Flipper SubGhz Setting File
Version: 1

# Add custom frequencies (in Hz)
Add_standard_frequencies: true

Frequency: 433920000
Frequency: 315000000
Frequency: 868350000
Frequency: 915000000

# Hopper frequencies (used by frequency hopping scanner)
Hopper_frequency: 433920000
Hopper_frequency: 868350000
```

**Default Frequencies** (partial list):
- 315.000 MHz (Car remotes, US)
- 433.920 MHz (ISM Europe, car remotes)
- 868.350 MHz (ISM Europe)
- 915.000 MHz (ISM US)

See: `documentation/SubGHzSettings.md` for complete frequency management guide.

---

## Regional Restrictions

### TX Restrictions

Transmission on certain frequencies may be restricted based on hardware region:

```c
// Check if TX allowed
if(!furi_hal_subghz_is_tx_allowed(315000000)) {
    FURI_LOG_E("SubGHz", "TX not allowed in this region");
    return;
}

// Get detailed status
SubGhzTx status = furi_hal_subghz_check_tx(315000000);
switch(status) {
    case SubGhzTxOk:
        FURI_LOG_I("SubGHz", "TX allowed");
        break;
    case SubGhzTxNotAllowed:
        FURI_LOG_W("SubGHz", "TX blocked by region");
        break;
    // ... other cases
}
```

---

## Complete Examples

### Simple Transmitter

```c
#include <furi_hal.h>
#include <furi_hal_subghz.h>

void simple_transmit(void) {
    // Initialize
    furi_hal_subghz_init();

    // Configure
    furi_hal_subghz_idle();
    furi_hal_subghz_load_custom_preset(preset_am650);
    furi_hal_subghz_set_frequency_and_path(433920000);

    // Prepare packet
    uint8_t packet[] = {0xFF, 0x00, 0xFF, 0x00, 0xAA, 0x55};
    furi_hal_subghz_write_packet(packet, sizeof(packet));

    // Transmit
    if(furi_hal_subghz_tx()) {
        FURI_LOG_I("TX", "Transmitting...");
        furi_delay_ms(100);
        furi_hal_subghz_idle();
        FURI_LOG_I("TX", "Complete");
    } else {
        FURI_LOG_E("TX", "Not allowed");
    }

    // Sleep
    furi_hal_subghz_sleep();
}
```

### Simple Receiver

```c
#include <furi_hal.h>
#include <furi_hal_subghz.h>

void simple_receive(uint32_t timeout_ms) {
    furi_hal_subghz_init();

    // Configure
    furi_hal_subghz_idle();
    furi_hal_subghz_load_custom_preset(preset_am650);
    furi_hal_subghz_set_frequency_and_path(433920000);

    // Start RX
    furi_hal_subghz_rx();
    FURI_LOG_I("RX", "Listening...");

    // Listen
    uint32_t start = furi_get_tick();
    while((furi_get_tick() - start) < timeout_ms) {
        if(furi_hal_subghz_rx_pipe_not_empty()) {
            uint8_t buffer[64];
            uint8_t size;

            furi_hal_subghz_read_packet(buffer, &size);

            if(furi_hal_subghz_is_rx_data_crc_valid()) {
                FURI_LOG_I("RX", "Packet: %d bytes", size);

                // Print packet
                for(uint8_t i = 0; i < size; i++) {
                    FURI_LOG_I("RX", "[%d] = 0x%02X", i, buffer[i]);
                }
            }
        }
        furi_delay_ms(10);
    }

    // Stop
    furi_hal_subghz_idle();
    furi_hal_subghz_sleep();
}
```

---

## Protocol Libraries

Flipper includes libraries for common protocols:

**Location**: `lib/subghz/protocols/`

**Supported Protocols**:
- KeeLoq (car remotes)
- Princeton (PT2260, PT2262)
- Gate TX (linear garage doors)
- Came (gates)
- Nice Flor-S (gates)
- Star Line (car alarms)
- And many more...

**Usage**:
```c
#include <lib/subghz/protocols/protocol_items.h>
```

See protocol-specific documentation in `lib/subghz/protocols/` for details.

---

## Troubleshooting

### Common Issues

**No signal received**:
- Check frequency is correct
- Verify preset matches signal modulation
- Check RSSI value (should be > -100 dBm)
- Ensure antenna path is correct
- Try different frequencies

**Weak signal**:
- Check PATABLE configuration (output power)
- Verify antenna path for frequency band
- Check battery level (low battery reduces TX power)
- Move away from interference sources

**TX not working**:
- Check `furi_hal_subghz_is_tx_allowed(freq)`
- Verify regional restrictions
- Ensure preset is loaded correctly
- Check power table (PATABLE)

**Garbled data**:
- Verify CRC: `furi_hal_subghz_is_rx_data_crc_valid()`
- Check preset matches transmitter
- Adjust frequency (fine-tuning may be needed)
- Check for interference

---

## Safety & Legal

### Important Warnings

1. **Regional Compliance**: Always check local regulations for ISM band usage
2. **Licensed Frequencies**: Never transmit on licensed frequencies (amateur radio, public safety, etc.)
3. **Interference**: Avoid interfering with critical systems (medical, aviation, etc.)
4. **Duty Cycle**: Some regions limit transmission duty cycle (e.g., EU: <10% on 433 MHz)
5. **Power Limits**: Respect regional power limits (typically 10-25 mW EIRP)

### Best Practices

- Use `furi_hal_subghz_is_tx_allowed()` before transmission
- Test in controlled environments
- Monitor for interference
- Use lowest power necessary
- Implement proper error handling

---

## Hardware Specifications

**CC1101 Radio**:
- Transceiver: Texas Instruments CC1101
- Interface: SPI (up to 10 MHz)
- Supply: 3.3V
- Current: 15 mA RX, 30 mA TX @ +10 dBm

**Antenna**:
- Type: PCB trace (omnidirectional)
- Impedance: 50Ω
- Efficiency: Band-dependent

**RF Switch**:
- Type: SP3T (Single-Pole Triple-Throw)
- Purpose: Band selection and matching

---

## Related Documentation

- [FuriHalAPI.md](../FuriHalAPI.md) - Complete HAL API
- [SubGHzSettings.md](../SubGHzSettings.md) - Frequency configuration
- [SubGhzFileFormats.md](../file_formats/SubGhzFileFormats.md) - File formats
- [SubGHzRemoteProg.md](../SubGHzRemoteProg.md) - Remote programming
- [furi_hal_subghz.h](../../targets/f7/furi_hal/furi_hal_subghz.h) - API header
- CC1101 Datasheet (Texas Instruments)

---

**Oracle Documentation** - Sub-GHz radio mastery complete.
