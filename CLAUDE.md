# CLAUDE.md - Kiisu Momentum Firmware

This file provides guidance to Claude Code when working with code in this repository.

## Overview

This is **Kiisu Momentum Firmware**, a custom firmware for Flipper Zero that combines:

1. **[Momentum Firmware](https://github.com/Next-Flip/Momentum-Firmware)** - Feature-rich firmware (continuation of Xtreme)
2. **Kiisu Hardware Platform** - Hardware extensions with companion MCU and advanced sensors

**Firmware Origin**: Momentum (continuation of Xtreme firmware)
**Target Hardware**: Flipper Zero with Kiisu hardware extensions (TARGET_HW=7)
**Main Language**: C (with some C++ and Python scripts)
**Build System**: FBT (Flipper Build Tool) - wrapper around SCons

---

## Kiisu-Specific Features

### Hardware Platform

**Kiisu** adds a companion MCU with:
- **Communication**: I2C @ address 0x30
- **Sensors**: LSM303 (accel+mag), GXHTC3C (temp+humidity), light sensor (ADC)
- **UI**: RGB LED with programmable patterns
- **Updates**: OTA firmware via I2C

### Kiisu Applications

Three dedicated applications located in `applications/`:

#### 1. Kiisu Manager (`kiisu-manager/`)
- **Purpose**: Configure companion MCU settings
- **Features**: Sleep timer, LED brightness, auto power-off, startup color, charge rainbow
- **Location**: `applications/kiisu-manager/`
- **Author**: 2elw

#### 2. Kiisu Companion Bridge (`kiisu_companion_bridge/`)
- **Purpose**: Flash companion MCU firmware safely via I2C
- **Features**: OTA updates, checksum verification, safe protocol with rollback
- **Requirements**: USB power, firmware ≤48KB
- **Location**: `applications/kiisu_companion_bridge/`
- **Author**: 2elw

#### 3. Kiisu Sensor Hub (`kiisu_sensor_hub/`)
- **Purpose**: Interactive sensor dashboard
- **Features**: Accelerometer (gravity sim, level tool), magnetometer (compass), temp/humidity, light
- **Calibration**: Built-in magnetometer and accelerometer calibration
- **Location**: `applications/kiisu_sensor_hub/`
- **Author**: 2elw

---

## Project Structure

### Kiisu-Specific Directories

```
applications/
├── kiisu-manager/                # Settings management
│   ├── kiisu_manager.c           # Main application
│   ├── settings_client.c         # I2C settings protocol
│   └── ui.c                      # User interface
├── kiisu_companion_bridge/       # Firmware updater
│   └── kiisu_companion_bridge.c  # OTA update protocol
└── kiisu_sensor_hub/             # Sensor dashboard
    └── src/
        ├── kiisu_sensor_hub_app.c # Main application
        └── drivers/               # Sensor drivers
            ├── lsm303.c           # Accelerometer + Magnetometer
            ├── gxhtc3c.c          # Temperature + Humidity
            └── kiisu_light_adc.c  # Light sensor (ADC)
```

### Documentation Structure

```
documentation/
├── KIISU_OVERVIEW.md             # ✅ Kiisu platform guide
├── KIISU_DEVELOPMENT.md          # ✅ Kiisu app development
├── FuriHalAPI.md                 # ✅ Complete HAL API
├── FuriCoreAPI.md                # ✅ FURI Core API
├── HAL_SUMMARY.md                # HAL architecture summary
├── FuriQuickReference.md         # Quick API reference
└── hardware/                     # ✅ Hardware-specific guides
    ├── gpio.md                   # GPIO pinout and examples
    ├── bluetooth.md              # BLE guide
    ├── usb.md                    # USB modes and HID
    └── subghz.md                 # Sub-GHz radio (CC1101)
```

---

## Build Commands

### Standard Build Commands

```bash
# Flash firmware to Flipper via USB (recommended for development)
./fbt flash_usb_full

# Build updater package (.tgz for qFlipper)
./fbt updater_package

# Build and launch specific app for testing
./fbt launch APPSRC=your_appid

# Build firmware and resources (default target)
./fbt fw_dist

# Build all external apps (.faps)
./fbt faps
```

### Kiisu App Development

```bash
# Build specific Kiisu app
./fbt fap_kiisu_manager
./fbt fap_kiisu_companion_bridge
./fbt fap_kiisu_sensor_hub

# Build all Kiisu apps
./fbt fap_kiisu_manager fap_kiisu_companion_bridge fap_kiisu_sensor_hub

# Deploy to Flipper
./fbt fap_deploy
```

### Code Quality

```bash
# Format code (MUST run before commits)
./fbt format

# Lint C code
./fbt lint

# Format Python
./fbt format_py

# Format all (C, Python, images)
./fbt format_all
```

---

## Core Architecture

### FURI (Flipper Universal Registry Implementation)

**Location**: `furi/`

FURI is the RTOS abstraction layer (built on FreeRTOS) providing:
- **Threading**: Threads, thread flags, priorities
- **Event Loop**: Async event-driven programming
- **IPC**: Message queues, pub-sub, records (service registry)
- **Synchronization**: Mutexes, semaphores, event flags
- **Memory**: Heap management, per-thread tracking

**Key Concepts**:
- One event loop per thread
- Records for service discovery
- Publish-subscribe for event broadcasting

### Hardware Abstraction Layer (HAL)

**Location**: `targets/`

- **API Headers**: `targets/furi_hal_include/`
- **F7 Implementation**: `targets/f7/furi_hal/`

**Key Modules**:
1. **GPIO** - Digital I/O, external pins
2. **Power** - Battery, charging, sleep modes
3. **Bluetooth** - BLE stack on CPU2 (STM32WB55)
4. **USB** - VCP, HID, CCID, U2F
5. **Sub-GHz** - CC1101 radio (315/433/868/915 MHz)
6. **NFC** - ST25R3916 controller
7. **I2C** - **Critical for Kiisu** - External I2C bus for companion MCU and sensors
8. **SPI** - Bus management for peripherals

---

## Kiisu I2C Communication

### I2C Bus Access

Kiisu uses **external I2C bus** for all communication:

```c
#include <furi_hal_i2c.h>

#define KIISU_COMPANION_ADDR 0x30  // Companion MCU
#define LSM303_ACCEL_ADDR    0x19  // Accelerometer
#define LSM303_MAG_ADDR      0x1E  // Magnetometer
#define GXHTC3C_ADDR         0x70  // Temp/Humidity

// Acquire bus
furi_hal_i2c_acquire(FuriHalI2cBusExternal);

// Check device presence
bool present = furi_hal_i2c_is_device_ready(
    FuriHalI2cBusExternal,
    KIISU_COMPANION_ADDR,
    100  // timeout ms
);

// Communicate
furi_hal_i2c_tx(FuriHalI2cBusExternal, addr, data, size, timeout);
furi_hal_i2c_rx(FuriHalI2cBusExternal, addr, buffer, size, timeout);
furi_hal_i2c_trx(FuriHalI2cBusExternal, addr, tx_data, tx_size, rx_data, rx_size, timeout);

// Release bus
furi_hal_i2c_release(FuriHalI2cBusExternal);
```

**Important**:
- Always acquire/release bus
- Use appropriate timeouts (100-1000ms)
- Check device presence before operations
- Handle communication errors gracefully

---

## Development Guidelines

### Kiisu App Development

When creating Kiisu apps:

1. **I2C Communication**:
   - Use `FuriHalI2cBusExternal` for all Kiisu devices
   - Always check device presence
   - Handle timeouts and errors
   - Reference existing Kiisu apps for patterns

2. **Sensor Access**:
   - Use drivers from `kiisu_sensor_hub/src/drivers/`
   - Implement calibration when needed (especially magnetometer)
   - Poll at reasonable rates (10-100 Hz typical)

3. **Power Management**:
   - Check USB power for firmware updates
   - Use companion MCU sleep modes
   - Coordinate with Momentum power settings

4. **LED Control**:
   - Respect user brightness settings
   - Use appropriate colors for feedback
   - Implement smooth transitions

### Code Style

Follow Flipper coding standards:

**Naming**:
- Types: `PascalCase` (e.g., `KiisuSensorData`)
- Functions: `snake_case` (e.g., `kiisu_sensor_read`)
- Files: `snake_case` (e.g., `kiisu_manager.c`)

**Indentation**: 4 spaces (no tabs)
**Line Length**: <100 characters preferred
**Comments**: Document non-obvious code, reference standards

**Always run before commit**:
```bash
./fbt format
```

---

## Testing

### Building and Testing Kiisu Apps

```bash
# Build specific app
./fbt fap_kiisu_sensor_hub

# Launch on connected Flipper
./fbt launch APPSRC=kiisu_sensor_hub

# Check logs
./fbt cli
```

### Unit Tests

```bash
# Build with unit tests
./fbt FIRMWARE_APP_SET=unit_tests

# Flash and run
./fbt flash_usb_full
```

---

## Documentation Quick Links

### Kiisu-Specific
- [KIISU_OVERVIEW.md](documentation/KIISU_OVERVIEW.md) - Platform overview, features, hardware specs
- [KIISU_DEVELOPMENT.md](documentation/KIISU_DEVELOPMENT.md) - Complete development guide with examples

### Hardware Documentation
- [FuriHalAPI.md](documentation/FuriHalAPI.md) - Complete HAL API reference (26 modules)
- [hardware/gpio.md](documentation/hardware/gpio.md) - GPIO pinout, modes, examples
- [hardware/bluetooth.md](documentation/hardware/bluetooth.md) - BLE profiles, advertising, security
- [hardware/usb.md](documentation/hardware/usb.md) - USB modes, HID, VCP
- [hardware/subghz.md](documentation/hardware/subghz.md) - CC1101 radio, frequencies, protocols

### Core Documentation
- [FuriCoreAPI.md](documentation/FuriCoreAPI.md) - FURI threading, event loop, IPC
- [FuriQuickReference.md](documentation/FuriQuickReference.md) - Quick API cheat sheet
- [HAL_SUMMARY.md](documentation/HAL_SUMMARY.md) - HAL architecture overview

### Build & Development
- [fbt.md](documentation/fbt.md) - Build system documentation
- [AppManifests.md](documentation/AppManifests.md) - Application manifest format
- [AppsOnSDCard.md](documentation/AppsOnSDCard.md) - External app deployment

---

## Git Workflow

- **Main development branch**: `dev`
- **Always run before commits**: `./fbt format`
- **Line endings**: LF (configured in `.gitattributes`)

### Commit Guidelines

1. Format code: `./fbt format`
2. Test changes: Build and test on device
3. Write clear commit messages
4. Reference issues when applicable

---

## Important Notes

### Kiisu Hardware Requirements

- Kiisu hardware with companion MCU
- Latest [enhanced-kiisu4-fw](https://github.com/twoelw/enhanced-kiisu4-fw) on companion MCU
- I2C address 0x30 must respond
- USB power for firmware updates

### Power Considerations

- External 3.3V rail is switchable via `furi_hal_power_enable_external_3_3v()`
- Always disable when done: `furi_hal_power_disable_external_3_3v()`
- Kiisu sensors use external I2C bus
- Companion MCU has independent power management

### Windows Development

This repository is developed on Windows:
- Use `./fbt` (not just `fbt`)
- Use `fbt.cmd` for native Windows command prompt
- Path separators in code should use POSIX style
- Line endings: LF (Git handles conversion)

---

## Resources

### External Links

- [Momentum Firmware](https://github.com/Next-Flip/Momentum-Firmware) - Base firmware
- [Enhanced Kiisu4 Firmware](https://github.com/twoelw/enhanced-kiisu4-fw) - Companion MCU firmware
- [Flipper Zero Docs](https://docs.flipper.net/) - Official documentation
- [FBT Documentation](documentation/fbt.md) - Build tool guide

### Datasheets

- STM32WB55 Reference Manual (main MCU)
- CC1101 Datasheet (Sub-GHz radio)
- ST25R3916 Datasheet (NFC controller)
- LSM303 Datasheet (Accel + Mag)
- SHTC3 Datasheet (Temp + Humidity, GXHTC3C compatible)

---

## Quick Reference

### Common Tasks

**Build and flash firmware**:
```bash
./fbt flash_usb_full
```

**Build Kiisu apps**:
```bash
./fbt fap_kiisu_manager fap_kiisu_companion_bridge fap_kiisu_sensor_hub
./fbt fap_deploy
```

**Format code**:
```bash
./fbt format
```

**Open CLI**:
```bash
./fbt cli
```

**Check I2C devices**:
```c
// In your app
furi_hal_i2c_acquire(FuriHalI2cBusExternal);
bool found = furi_hal_i2c_is_device_ready(FuriHalI2cBusExternal, 0x30, 100);
furi_hal_i2c_release(FuriHalI2cBusExternal);
```

---

**This file is maintained by Oracle - Knowledge transfer complete.**
