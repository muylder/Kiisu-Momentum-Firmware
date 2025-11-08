# Kiisu Hardware Platform Overview

Complete guide to Kiisu-specific features and hardware extensions for Flipper Zero.

## What is Kiisu?

**Kiisu** is a hardware enhancement platform for Flipper Zero that adds:

- **Companion MCU**: Auxiliary microcontroller with independent firmware
- **Advanced Sensors**: Accelerometer, magnetometer, temperature, humidity, light
- **Enhanced UI**: RGB LED with programmable patterns
- **Extended Functionality**: Additional processing power and peripherals
- **Over-the-Air Updates**: Firmware updates via I2C without soldering

**Hardware Target**: Kiisu4 with enhanced companion firmware
**Communication**: I2C @ address 0x30
**Firmware Repository**: [enhanced-kiisu4-fw](https://github.com/twoelw/enhanced-kiisu4-fw)

---

## Architecture

### System Overview

```
┌─────────────────────────────────────┐
│     Flipper Zero (STM32WB55)        │
│  ┌──────────────────────────────┐   │
│  │  Kiisu Momentum Firmware     │   │
│  │  - kiisu-manager             │   │
│  │  - kiisu_companion_bridge    │   │
│  │  - kiisu_sensor_hub          │   │
│  └────────────┬─────────────────┘   │
│               │ I2C                  │
└───────────────┼──────────────────────┘
                │
┌───────────────▼──────────────────────┐
│   Kiisu Companion MCU (0x30)         │
│  ┌──────────────────────────────┐    │
│  │  Enhanced Kiisu4 Firmware    │    │
│  │  - Settings management       │    │
│  │  - LED control               │    │
│  │  - Sensor interface          │    │
│  │  - Firmware update protocol  │    │
│  └──────────────────────────────┘    │
│                                       │
│   Hardware:                           │
│   - LSM303 (Accel + Mag)             │
│   - GXHTC3C (Temp + Humidity)        │
│   - Light sensor (ADC)               │
│   - RGB LED                          │
└───────────────────────────────────────┘
```

### Communication Protocol

**Interface**: I2C (External Bus)
**Address**: 0x30 (companion MCU)
**Speed**: Standard mode (100 kHz) or Fast mode (400 kHz)
**Bus Handle**: `furi_hal_i2c_handle_external`

---

## Kiisu Applications

### 1. Kiisu Manager

**Location**: `applications/kiisu-manager/`
**Category**: Tools
**Purpose**: Configure companion MCU settings

**Features**:
- ⏱️ Sleep timer configuration (Always ON → 30 min)
- 💡 LED brightness control (Auto, 5-100%)
- ⚡ Auto power-off (15s → 60m)
- 🎨 Startup color selection (8 colors + off)
- 🌈 Charge rainbow effect toggle

**Usage**:
```
Tools → Kiisu Manager
- Navigate with UP/DOWN
- Adjust settings with LEFT/RIGHT
- Changes saved automatically
```

**API Functions** (from `settings_client.h`):
```c
// Initialize settings client
void kiisu_settings_init(void);

// Get/Set sleep timeout
uint16_t kiisu_settings_get_sleep_timeout(void);
void kiisu_settings_set_sleep_timeout(uint16_t minutes);

// Get/Set LED brightness (0-100, 255 = auto)
uint8_t kiisu_settings_get_led_brightness(void);
void kiisu_settings_set_led_brightness(uint8_t percent);

// Get/Set auto power-off
uint16_t kiisu_settings_get_auto_poweroff(void);
void kiisu_settings_set_auto_poweroff(uint16_t seconds);

// Get/Set startup color
uint8_t kiisu_settings_get_startup_color(void);
void kiisu_settings_set_startup_color(uint8_t color);

// Get/Set charge rainbow
bool kiisu_settings_get_charge_rainbow(void);
void kiisu_settings_set_charge_rainbow(bool enabled);
```

---

### 2. Kiisu Companion Bridge

**Location**: `applications/kiisu_companion_bridge/`
**Category**: Tools
**Purpose**: Flash companion MCU firmware safely

**Features**:
- 🔄 OTA firmware updates via I2C
- ✅ Checksum verification
- 🛡️ Safe update protocol with rollback
- 📁 Load firmware from SD card (.bin files)
- ⚠️ Power and integrity checks

**Requirements**:
- USB power connected (charging required)
- Compatible companion firmware with updater at I2C 0x30
- Firmware size ≤ 48 KiB
- Base address: 0x08000000 or 0x08010000

**Update Protocol**:
```
1. Select firmware .bin file from SD card
2. Verify vector table and checksum
3. Enter bootloader mode (device confirms)
4. Erase flash sectors
5. Program firmware in chunks
6. Verify CRC
7. Finalize and reboot
```

**Safety Features**:
- Power check (must be charging)
- Vector table validation
- CRC verification per chunk
- Commit timeout protection
- Rollback on failure

**Usage**:
```
Tools → Kiisu Companion Bridge
1. Plug USB power
2. Press OK to select .bin file
3. Read warning and confirm
4. Wait 1-2 minutes for flashing
5. Green LED blinks → unplug and re-plug power
```

**Troubleshooting**:
- Device not found (0x30): Update to latest companion firmware
- Bad vector table: Rebuild for 0x08000000 or 0x08010000
- Commit timeout/CRC mismatch: Check power stability, retry
- Finalize failed: Retry; if persistent, use USB DFU recovery

---

### 3. Kiisu Sensor Hub

**Location**: `applications/kiisu_sensor_hub/`
**Category**: Tools
**Purpose**: Explore onboard sensors with interactive dashboard

**Features**:

#### Dashboard
- 🕐 Time/date display
- 📊 5 sensor widgets:
  - Accelerometer (gravity kitties sim)
  - Magnetometer (compass)
  - Temperature (dual sensors + average)
  - Humidity (RH%)
  - Light (percentage + voltage)

#### Accelerometer Pages
1. **Kitties (Gravity Sim)**
   - 5 sprites bounce with real gravity
   - Responds to device orientation
   - Fun physics simulation

2. **Level Tool**
   - Bullseye mode (face-up)
   - Bar mode (upright)
   - Auto-snaps to 0° within ±3°
   - Large degree readout

3. **Raw Data**
   - X/Y/Z in g units
   - Pitch/Roll/Yaw angles
   - Calibration trigger (press OK)

#### Magnetometer Pages
1. **Compass**
   - Tilt-compensated heading
   - 360° dial with static needle
   - 8-wind labels (N, NE, E, SE, S, SW, W, NW)
   - Vector smoothing

2. **Raw Values**
   - X/Y/Z in μT (microtesla)
   - Min/max during calibration
   - Calibration status display
   - Calibration trigger (press OK)

#### Temperature Page
- **GXCAS**: GXHTC3C sensor reading
- **ST**: LSM303 die temperature
- **Average**: Centered average of both

#### Humidity Page
- Live RH% from GXHTC3C
- Range: 0-100%

#### Light Page
- Percentage (0-100%)
- Millivolts from ADC

**Calibration**:

**Accelerometer Leveling**:
```
1. Go to Accelerometer → Raw
2. Press OK
3. Place device flat
4. Press OK to start 3-second average
5. Confirmation sound plays
6. Stored in RAM for session
```

**Magnetometer (Hard/Soft Iron)**:
```
1. Go to Magnetometer → Raw
2. Press OK
3. Flat baseline:
   - Place device flat
   - Press OK for 3-second average
4. Figure-eight:
   - Press OK to start
   - Move device in figure-eight for ~10s
   - Keep away from metal/magnets
5. Computes offsets/scales
6. Writes calibration to LSM303
7. Shows "Cal:OK"
```

**Sensor Details**:
- **LSM303**: Accel + Mag + Die temp (I2C)
- **GXHTC3C**: Temp + Humidity (I2C, SHTC3-class)
- **Light**: ADC via `kiisu_light_adc`

---

## Hardware Specifications

### Companion MCU
- **Type**: STM32 or compatible
- **Flash**: ~48 KiB available for firmware
- **I2C Address**: 0x30
- **Bootloader**: Supports OTA updates via I2C
- **Recovery**: USB DFU or ST-LINK

### Sensors

#### LSM303 (Accelerometer + Magnetometer)
- **Interface**: I2C
- **Accelerometer**: ±2g/±4g/±8g/±16g selectable
- **Magnetometer**: ±1.3 to ±8.1 gauss
- **Features**:
  - 3-axis acceleration
  - 3-axis magnetic field
  - Die temperature sensor
  - Tilt compensation
  - Hard/soft iron calibration

#### GXHTC3C (Temperature + Humidity)
- **Interface**: I2C
- **Temperature**: -40°C to +125°C
- **Humidity**: 0-100% RH
- **Accuracy**: ±0.2°C, ±2% RH
- **Compatibility**: SHTC3-class sensor

#### Light Sensor
- **Interface**: ADC
- **Type**: Photodiode or phototransistor
- **Output**: Analog voltage
- **Range**: 0-100% relative

### RGB LED
- **Type**: Common cathode or WS2812-style
- **Control**: Via companion MCU
- **Features**:
  - 8 colors (Red, Green, Blue, Yellow, Cyan, Magenta, White, Purple)
  - Brightness control (5-100%)
  - Auto brightness mode
  - Rainbow charging effect
  - Startup color selection

---

## I2C Communication

### Accessing Companion MCU

```c
#include <furi_hal_i2c.h>

#define KIISU_I2C_ADDR 0x30

// Acquire I2C bus
furi_hal_i2c_acquire(FuriHalI2cBusExternal);

// Check if device present
if(furi_hal_i2c_is_device_ready(FuriHalI2cBusExternal, KIISU_I2C_ADDR, 100)) {
    FURI_LOG_I("Kiisu", "Companion MCU detected");
}

// Write command
uint8_t cmd[] = {0x01, 0x02};
bool success = furi_hal_i2c_tx(
    FuriHalI2cBusExternal,
    KIISU_I2C_ADDR,
    cmd,
    sizeof(cmd),
    100
);

// Read response
uint8_t response[16];
success = furi_hal_i2c_rx(
    FuriHalI2cBusExternal,
    KIISU_I2C_ADDR,
    response,
    sizeof(response),
    100
);

// Release I2C bus
furi_hal_i2c_release(FuriHalI2cBusExternal);
```

### Sensor Access (LSM303 Example)

```c
#include <furi_hal_i2c.h>

#define LSM303_ACCEL_ADDR 0x19
#define LSM303_MAG_ADDR   0x1E

void read_lsm303_accel(int16_t* x, int16_t* y, int16_t* z) {
    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    // Read accelerometer registers
    uint8_t reg = 0x28 | 0x80;  // AUTO_INCREMENT
    uint8_t data[6];

    bool success = furi_hal_i2c_trx(
        FuriHalI2cBusExternal,
        LSM303_ACCEL_ADDR,
        &reg, 1,
        data, 6,
        100
    );

    if(success) {
        *x = (int16_t)((data[1] << 8) | data[0]);
        *y = (int16_t)((data[3] << 8) | data[2]);
        *z = (int16_t)((data[5] << 8) | data[4]);
    }

    furi_hal_i2c_release(FuriHalI2cBusExternal);
}
```

---

## Development Guide

### Creating Kiisu Apps

**Manifest** (`application.fam`):
```python
App(
    appid="my_kiisu_app",
    name="My Kiisu App",
    apptype=FlipperAppType.EXTERNAL,
    entry_point="my_kiisu_app_main",
    requires=["gui"],
    stack_size=4 * 1024,
    fap_category="Tools",
    fap_icon="icon.png",
    fap_author="YourName",
    fap_version="1.0.0",
)
```

**Basic Structure**:
```c
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_i2c.h>
#include <gui/gui.h>

#define KIISU_I2C_ADDR 0x30

typedef struct {
    Gui* gui;
    ViewPort* view_port;
} MyKiisuApp;

void my_kiisu_app_draw(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, "Kiisu App");
}

bool my_kiisu_app_input(InputEvent* event, void* ctx) {
    // Handle input
    return true;
}

int32_t my_kiisu_app_main(void* p) {
    UNUSED(p);

    MyKiisuApp* app = malloc(sizeof(MyKiisuApp));

    // Initialize GUI
    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();

    view_port_draw_callback_set(app->view_port, my_kiisu_app_draw, app);
    view_port_input_callback_set(app->view_port, my_kiisu_app_input, app);

    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    // Main loop
    while(true) {
        furi_delay_ms(100);
        view_port_update(app->view_port);
    }

    // Cleanup
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    free(app);

    return 0;
}
```

### Best Practices

1. **I2C Communication**:
   - Always acquire/release bus
   - Use timeouts (100-1000ms typical)
   - Check device presence before operations
   - Handle communication errors gracefully

2. **Power Management**:
   - Check USB power for firmware updates
   - Use companion MCU sleep modes
   - Implement auto power-off timers

3. **Sensor Calibration**:
   - Provide calibration UI for magnetometer
   - Store calibration in persistent storage
   - Validate calibration data

4. **LED Control**:
   - Respect user brightness settings
   - Use appropriate colors for feedback
   - Implement smooth transitions

5. **Error Handling**:
   - Detect missing companion MCU
   - Handle I2C timeouts gracefully
   - Provide user feedback on errors

---

## Companion Firmware

### Enhanced Kiisu4 Firmware

**Repository**: [enhanced-kiisu4-fw](https://github.com/twoelw/enhanced-kiisu4-fw/releases)

**Features**:
- Settings management (sleep, LED, power-off)
- OTA update protocol
- Sensor interface
- LED pattern control
- I2C command protocol

**Update Process**:
1. Download latest .bin from releases
2. Copy to SD card
3. Use Kiisu Companion Bridge app
4. Follow on-screen instructions

**Recovery**:
- USB DFU mode (built-in bootloader)
- ST-LINK (requires soldering or adapter)

---

## Troubleshooting

### Common Issues

**Companion MCU not detected**:
- Check companion firmware version
- Verify I2C connections
- Try power cycle (unplug battery)
- Check I2C address (should be 0x30)

**Sensor readings invalid**:
- Calibrate sensors (magnetometer especially)
- Check I2C bus for conflicts
- Verify sensor power supply
- Try firmware update

**LED not working**:
- Check brightness setting (not 0%)
- Verify LED mode (not "Off" for startup color)
- Update companion firmware
- Check power management settings

**Firmware update fails**:
- Ensure USB power connected
- Check firmware file integrity
- Try different USB cable/port
- Use recovery mode (DFU)

---

## Integration with Momentum

Kiisu apps integrate seamlessly with Momentum Firmware:

- **Power Management**: Coordinate auto power-off with Momentum settings
- **File System**: Store calibration on SD card
- **GUI**: Use Momentum UI components
- **Services**: Access storage, notification, power services

**Recommendation**: Disable Kiisu Manager auto power-off and use Momentum's built-in setting for consistency.

---

## Future Development

Potential enhancements:
- Additional sensor support (GPS, barometer)
- Wireless communication (BLE, LoRa)
- Battery monitoring
- Advanced motion algorithms
- Machine learning on-device
- Custom sensor fusion

---

## Related Documentation

- [FuriHalAPI.md](FuriHalAPI.md) - HAL API reference
- [hardware/gpio.md](hardware/gpio.md) - GPIO guide
- [I2C Communication](hardware/i2c.md) - I2C protocol details
- [CLAUDE.md](CLAUDE.md) - Project overview
- [enhanced-kiisu4-fw](https://github.com/twoelw/enhanced-kiisu4-fw) - Companion firmware

---

**Oracle Documentation** - Kiisu platform mastery complete.
