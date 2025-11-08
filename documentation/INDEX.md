# Documentation Index

Complete guide to all documentation in Kiisu Momentum Firmware.

## 📚 Quick Start

New to Kiisu or Flipper development? Start here:

1. **[CLAUDE.md](../CLAUDE.md)** - Project overview and development setup
2. **[KIISU_OVERVIEW.md](KIISU_OVERVIEW.md)** - Kiisu hardware platform guide
3. **[FuriCoreAPI.md](FuriCoreAPI.md)** - Core FURI API (threading, IPC)
4. **[FuriHalAPI.md](FuriHalAPI.md)** - Hardware abstraction layer
5. **[KIISU_DEVELOPMENT.md](KIISU_DEVELOPMENT.md)** - Build your first Kiisu app

---

## 🎯 Kiisu-Specific Documentation

### Platform Documentation

| Document | Description | Use When |
|----------|-------------|----------|
| [KIISU_OVERVIEW.md](KIISU_OVERVIEW.md) | Complete platform guide | Understanding Kiisu hardware, apps, architecture |
| [KIISU_DEVELOPMENT.md](KIISU_DEVELOPMENT.md) | Development guide | Building apps that use Kiisu hardware |

### Kiisu Applications

| Application | Location | Purpose |
|-------------|----------|---------|
| **Kiisu Manager** | `applications/kiisu-manager/` | Configure companion MCU settings |
| **Companion Bridge** | `applications/kiisu_companion_bridge/` | Flash companion firmware via I2C |
| **Sensor Hub** | `applications/kiisu_sensor_hub/` | Interactive sensor dashboard |

---

## 🔧 Hardware Documentation

### Complete Hardware Guides

| Document | Covers | Key Topics |
|----------|--------|------------|
| [hardware/gpio.md](hardware/gpio.md) | GPIO & External Pins | 8 external pins, modes, interrupts, PWM, ADC |
| [hardware/bluetooth.md](hardware/bluetooth.md) | Bluetooth Low Energy | Dual-core, profiles, advertising, security |
| [hardware/usb.md](hardware/usb.md) | USB Device Modes | VCP, HID (keyboard/mouse), CCID, U2F |
| [hardware/subghz.md](hardware/subghz.md) | Sub-GHz Radio (CC1101) | 300-928 MHz, packet/async modes, protocols |

### Hardware Reference

| Document | Description | Use When |
|----------|-------------|----------|
| [FuriHalAPI.md](FuriHalAPI.md) | Complete HAL API (26 modules) | Accessing any hardware peripheral |
| [HAL_SUMMARY.md](HAL_SUMMARY.md) | HAL architecture overview | Understanding HAL structure |
| [HardwareTargets.md](HardwareTargets.md) | Hardware variants | Working with different Flipper models |

---

## 💻 Core Firmware Documentation

### FURI Core API

| Document | Description | Key Concepts |
|----------|-------------|--------------|
| [FuriCoreAPI.md](FuriCoreAPI.md) | Complete FURI API | Threading, event loop, records, IPC |
| [FuriQuickReference.md](FuriQuickReference.md) | Quick API cheat sheet | Fast lookup of common APIs |
| [FURI_ARCHITECTURE_ANALYSIS.md](FURI_ARCHITECTURE_ANALYSIS.md) | Architecture deep dive | Understanding FURI internals |

### Key FURI Components

- **Threading**: `FuriThread`, priorities, thread flags
- **Event Loop**: Async event-driven programming
- **Records**: Service registry and discovery
- **Pub/Sub**: Event broadcasting
- **IPC**: Message queues, mutexes, semaphores

---

## 🛠️ Build System & Development

### Build & Deploy

| Document | Description | Use When |
|----------|-------------|----------|
| [fbt.md](fbt.md) | Flipper Build Tool guide | Building firmware, apps, packages |
| [AppManifests.md](AppManifests.md) | Application manifest format | Creating `application.fam` files |
| [AppsOnSDCard.md](AppsOnSDCard.md) | External app deployment | Deploying FAPs to SD card |
| [UnitTests.md](UnitTests.md) | Testing framework | Writing and running tests |

### Common Build Commands

```bash
# Flash firmware
./fbt flash_usb_full

# Build specific app
./fbt fap_my_app

# Deploy all apps
./fbt fap_deploy

# Format code (required before commit)
./fbt format

# Run unit tests
./fbt FIRMWARE_APP_SET=unit_tests flash_usb_full
```

---

## 📡 Wireless Protocols

### SubGHz (CC1101)

| Document | Description | Topics |
|----------|-------------|--------|
| [hardware/subghz.md](hardware/subghz.md) | Complete radio guide | Frequencies, modulation, packet/async modes |
| [SubGHzSettings.md](SubGHzSettings.md) | Frequency configuration | Adding custom frequencies |
| [SubGHzRemoteProg.md](SubGHzRemoteProg.md) | Remote programming | Programming remotes |
| [SubGHzRemotePlugin.md](SubGHzRemotePlugin.md) | Remote plugins | Creating custom remotes |
| [SubGHzBypass&Extend.md](SubGHzBypass&Extend.md) | Frequency bypass | Extending frequency range |
| [file_formats/SubGhzFileFormats.md](file_formats/SubGhzFileFormats.md) | File format specs | `.sub` file format |

### NFC

| Document | Description |
|----------|-------------|
| [file_formats/NfcFileFormats.md](file_formats/NfcFileFormats.md) | NFC file formats |

### Infrared

| Document | Description |
|----------|-------------|
| [InfraredCaptures.md](InfraredCaptures.md) | Capturing IR signals |
| [UniversalRemotes.md](UniversalRemotes.md) | Universal remote database |
| [file_formats/InfraredFileFormats.md](file_formats/InfraredFileFormats.md) | IR file formats |

### RFID & iButton

| Document | Description |
|----------|-------------|
| [LFRFIDRaw.md](LFRFIDRaw.md) | Low-frequency RFID raw mode |
| [file_formats/LfRfidFileFormat.md](file_formats/LfRfidFileFormat.md) | RFID file format |
| [file_formats/iButtonFileFormat.md](file_formats/iButtonFileFormat.md) | iButton file format |

---

## 🎨 User Interface & Assets

### UI & Customization

| Document | Description | Use When |
|----------|-------------|----------|
| [file_formats/AssetPacks.md](file_formats/AssetPacks.md) | Asset pack system | Creating themes |
| [CustomFlipperName.md](CustomFlipperName.md) | Custom device names | Personalizing Flipper |

### File Formats

| Document | Description |
|----------|-------------|
| [file_formats/BadUsbScriptFormat.md](file_formats/BadUsbScriptFormat.md) | BadUSB script syntax |
| [file_formats/TarHeatshrinkFormat.md](file_formats/TarHeatshrinkFormat.md) | Compressed archive format |

---

## 🔐 Security Documentation

### Security Guides

| Document | Description |
|----------|-------------|
| [security/README.md](security/README.md) | Security overview |
| [security/QUICK_START.md](security/QUICK_START.md) | Security quick start |

### Security Topics

- Threat modeling
- Secure coding practices
- Vulnerability assessment
- Penetration testing guidelines
- Security architecture

---

## ⚙️ Advanced Topics

### Low-Level & Debugging

| Document | Description | Use When |
|----------|-------------|----------|
| [FuriHalDebugging.md](FuriHalDebugging.md) | HAL debugging | Debugging hardware issues |
| [FuriHalBus.md](FuriHalBus.md) | Peripheral bus management | Managing bus access |
| [FuriCheck.md](FuriCheck.md) | Assertion system | Runtime checks |

### Hardware Extensions

| Document | Description | Use When |
|----------|-------------|----------|
| [ExpansionModules.md](ExpansionModules.md) | Expansion module development | Creating hardware add-ons |
| [NRF24.md](NRF24.md) | NRF24L01+ wireless module | 2.4 GHz communication |

### Miscellaneous

| Document | Description |
|----------|-------------|
| [KeyCombo.md](KeyCombo.md) | Key combinations |
| [MultiConverter.md](MultiConverter.md) | Multi-format converter |
| [SentrySafe.md](SentrySafe.md) | SentrySafe integration |
| [OTA.md](OTA.md) | Over-the-air updates |
| [OPTIMIZATIONS.md](OPTIMIZATIONS.md) | Performance optimizations |
| [BUGFIX_DEEP_SLEEP_CRASH.md](BUGFIX_DEEP_SLEEP_CRASH.md) | Deep sleep crash fix |

---

## 🗂️ File Organization

### Directory Structure

```
documentation/
├── INDEX.md                          # This file
├── KIISU_OVERVIEW.md                 # ✅ Kiisu platform guide
├── KIISU_DEVELOPMENT.md              # ✅ Kiisu dev guide
├── CLAUDE.md                         # (root) Project overview
│
├── FuriHalAPI.md                     # ✅ Complete HAL API
├── FuriCoreAPI.md                    # ✅ Core FURI API
├── HAL_SUMMARY.md                    # HAL architecture
├── FuriQuickReference.md             # API quick reference
├── FURI_ARCHITECTURE_ANALYSIS.md     # FURI deep dive
│
├── hardware/                         # ✅ Hardware guides
│   ├── gpio.md                       # GPIO & external pins
│   ├── bluetooth.md                  # BLE guide
│   ├── usb.md                        # USB modes
│   └── subghz.md                     # Sub-GHz radio
│
├── file_formats/                     # File format specifications
│   ├── SubGhzFileFormats.md
│   ├── NfcFileFormats.md
│   ├── InfraredFileFormats.md
│   ├── LfRfidFileFormat.md
│   ├── iButtonFileFormat.md
│   ├── BadUsbScriptFormat.md
│   ├── AssetPacks.md
│   └── TarHeatshrinkFormat.md
│
├── security/                         # Security documentation
│   ├── README.md
│   ├── QUICK_START.md
│   ├── adr/                          # Architecture decisions
│   ├── architecture/                 # Security architecture
│   ├── guidelines/                   # Coding guidelines
│   ├── implementation/               # Security implementations
│   ├── policies/                     # Security policies
│   ├── procedures/                   # Security procedures
│   ├── testing/                      # Security testing
│   └── threat-model/                 # Threat modeling
│
├── devboard/                         # Developer board docs
│   ├── Get started with the Dev Board.md
│   ├── Debugging via the Devboard.md
│   ├── Reading logs via the Dev Board.md
│   └── ...
│
└── doxygen/                          # Doxygen configuration
    └── doxygen-awesome-css/
```

---

## 📖 Documentation by Use Case

### I Want to...

**Build Kiisu Apps**:
1. [KIISU_OVERVIEW.md](KIISU_OVERVIEW.md) - Understand platform
2. [KIISU_DEVELOPMENT.md](KIISU_DEVELOPMENT.md) - Development guide
3. [FuriHalAPI.md](FuriHalAPI.md) - I2C and HAL APIs

**Control GPIO**:
1. [hardware/gpio.md](hardware/gpio.md) - Complete GPIO guide
2. [FuriHalAPI.md](FuriHalAPI.md#gpio-digital-io) - GPIO API reference

**Use Bluetooth**:
1. [hardware/bluetooth.md](hardware/bluetooth.md) - BLE guide
2. [FuriHalAPI.md](FuriHalAPI.md#bluetooth-ble) - BLE API reference

**Work with Sub-GHz**:
1. [hardware/subghz.md](hardware/subghz.md) - Complete radio guide
2. [SubGHzSettings.md](SubGHzSettings.md) - Add frequencies
3. [file_formats/SubGhzFileFormats.md](file_formats/SubGhzFileFormats.md) - File format

**Create USB HID Device**:
1. [hardware/usb.md](hardware/usb.md) - USB and HID guide
2. [file_formats/BadUsbScriptFormat.md](file_formats/BadUsbScriptFormat.md) - BadUSB scripts

**Understand FURI**:
1. [FuriCoreAPI.md](FuriCoreAPI.md) - Complete FURI API
2. [FURI_ARCHITECTURE_ANALYSIS.md](FURI_ARCHITECTURE_ANALYSIS.md) - Architecture
3. [FuriQuickReference.md](FuriQuickReference.md) - Quick reference

**Build and Deploy**:
1. [fbt.md](fbt.md) - Build system
2. [AppManifests.md](AppManifests.md) - App configuration
3. [AppsOnSDCard.md](AppsOnSDCard.md) - Deployment

**Access Sensors (Kiisu)**:
1. [KIISU_DEVELOPMENT.md](KIISU_DEVELOPMENT.md#sensor-integration) - Sensor drivers
2. Kiisu Sensor Hub source: `applications/kiisu_sensor_hub/src/drivers/`

---

## 🎯 Documentation Status

| Category | Status | Files |
|----------|--------|-------|
| ✅ Kiisu Platform | Complete | 2 docs |
| ✅ Hardware (HAL) | Complete | 5 docs |
| ✅ Core (FURI) | Complete | 3 docs |
| ✅ Build System | Complete | 4 docs |
| ✅ Protocols | Complete | 10+ docs |
| ✅ File Formats | Complete | 8 docs |
| ⚠️ Security | Partial | Structure created |

---

## 🔄 Recent Updates

- ✅ Created complete Kiisu platform documentation
- ✅ Added FuriHalAPI.md with 26 HAL modules
- ✅ Created hardware-specific guides (GPIO, BLE, USB, SubGHz)
- ✅ Comprehensive Kiisu development guide with examples
- ✅ Updated CLAUDE.md with Kiisu information
- ✅ Created this documentation index

---

## 🤝 Contributing to Documentation

When adding new documentation:

1. **Place in correct directory**:
   - Kiisu-specific: `documentation/KIISU_*.md`
   - Hardware: `documentation/hardware/*.md`
   - File formats: `documentation/file_formats/*.md`

2. **Update this index**: Add entry in appropriate section

3. **Cross-reference**: Link to related docs

4. **Use templates**:
   - Start with "# Title"
   - Include "## Overview" section
   - Add code examples
   - End with "## Related Documentation"

5. **Format code**:
   ```bash
   ./fbt format  # Format before commit
   ```

---

## 📝 Documentation Style Guide

### Headers

```markdown
# Document Title (H1 - only once)

## Major Section (H2)

### Subsection (H3)

#### Detail (H4)
```

### Code Blocks

```markdown
```c
// C code example
void example(void) {
    // ...
}
```

```bash
# Bash commands
./fbt flash_usb_full
```
```

### Links

```markdown
[Relative link](FuriHalAPI.md)
[Absolute link](https://github.com/...)
```

### Tables

```markdown
| Column 1 | Column 2 |
|----------|----------|
| Value 1  | Value 2  |
```

---

## 🎓 Learning Path

### Beginner

1. Read [CLAUDE.md](../CLAUDE.md)
2. Understand [FuriCoreAPI.md](FuriCoreAPI.md)
3. Try [hardware/gpio.md](hardware/gpio.md) examples
4. Build first app with [AppManifests.md](AppManifests.md)

### Intermediate

1. Explore [FuriHalAPI.md](FuriHalAPI.md)
2. Work with protocols ([hardware/subghz.md](hardware/subghz.md), [hardware/bluetooth.md](hardware/bluetooth.md))
3. Study [FURI_ARCHITECTURE_ANALYSIS.md](FURI_ARCHITECTURE_ANALYSIS.md)

### Advanced (Kiisu)

1. Read [KIISU_OVERVIEW.md](KIISU_OVERVIEW.md)
2. Study [KIISU_DEVELOPMENT.md](KIISU_DEVELOPMENT.md)
3. Analyze existing Kiisu apps
4. Build sensor-based applications

---

**Oracle Documentation Index** - Complete knowledge map established.
