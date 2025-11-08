# Kiisu Application Development Guide

Complete guide for developing applications that interact with Kiisu hardware.

## Overview

This guide covers:
- Setting up Kiisu development environment
- Accessing companion MCU and sensors
- Creating sensor-based applications
- Implementing LED control
- Best practices and patterns

**Prerequisites**:
- Kiisu hardware with companion MCU
- Latest [enhanced-kiisu4-fw](https://github.com/twoelw/enhanced-kiisu4-fw) installed
- Flipper Zero development environment (FBT)
- Basic understanding of C and Flipper APIs

---

## Quick Start

### 1. Create App Structure

```bash
# Create app directory
mkdir applications/external/my_kiisu_app

# Create files
touch applications/external/my_kiisu_app/application.fam
touch applications/external/my_kiisu_app/my_kiisu_app.c
touch applications/external/my_kiisu_app/icon.png
```

### 2. Define App Manifest

**applications/external/my_kiisu_app/application.fam**:
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
    fap_author="Your Name",
    fap_version="1.0.0",
    fap_description="My awesome Kiisu app",
)
```

### 3. Build and Deploy

```bash
# Build specific app
./fbt fap_my_kiisu_app

# Deploy to Flipper
./fbt fap_deploy
```

---

## I2C Communication

### Basic I2C Setup

```c
#include <furi_hal_i2c.h>

#define KIISU_COMPANION_ADDR 0x30

// Check if companion MCU is present
bool kiisu_companion_present(void) {
    bool present = false;

    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    present = furi_hal_i2c_is_device_ready(
        FuriHalI2cBusExternal,
        KIISU_COMPANION_ADDR,
        100  // 100ms timeout
    );

    furi_hal_i2c_release(FuriHalI2cBusExternal);

    return present;
}
```

### Write Command to Companion MCU

```c
typedef enum {
    CMD_SET_LED_BRIGHTNESS = 0x01,
    CMD_SET_LED_COLOR = 0x02,
    CMD_GET_STATUS = 0x03,
    // ... add your commands
} KiisuCommand;

bool kiisu_send_command(uint8_t cmd, const uint8_t* data, size_t len) {
    bool success = false;

    // Prepare packet: [CMD][DATA...]
    uint8_t buffer[32];
    buffer[0] = cmd;
    memcpy(&buffer[1], data, len);

    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    success = furi_hal_i2c_tx(
        FuriHalI2cBusExternal,
        KIISU_COMPANION_ADDR,
        buffer,
        len + 1,
        1000  // 1 second timeout
    );

    furi_hal_i2c_release(FuriHalI2cBusExternal);

    return success;
}

// Example: Set LED brightness
void kiisu_set_led_brightness(uint8_t percent) {
    uint8_t data = percent;
    kiisu_send_command(CMD_SET_LED_BRIGHTNESS, &data, 1);
}
```

### Read Data from Companion MCU

```c
bool kiisu_read_data(uint8_t cmd, uint8_t* buffer, size_t len) {
    bool success = false;

    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    // Write command, then read response
    success = furi_hal_i2c_trx(
        FuriHalI2cBusExternal,
        KIISU_COMPANION_ADDR,
        &cmd, 1,          // TX: command byte
        buffer, len,      // RX: response buffer
        1000
    );

    furi_hal_i2c_release(FuriHalI2cBusExternal);

    return success;
}
```

---

## Sensor Integration

### LSM303 Accelerometer

**Location**: `applications/kiisu_sensor_hub/src/drivers/lsm303.h`

```c
#include "../kiisu_sensor_hub/src/drivers/lsm303.h"

// Initialize LSM303
bool lsm303_init(void) {
    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    // Check device IDs
    bool accel_present = furi_hal_i2c_is_device_ready(
        FuriHalI2cBusExternal,
        LSM303_ACCEL_ADDR,
        100
    );

    bool mag_present = furi_hal_i2c_is_device_ready(
        FuriHalI2cBusExternal,
        LSM303_MAG_ADDR,
        100
    );

    if(accel_present && mag_present) {
        // Configure accelerometer
        uint8_t accel_config[] = {
            0x20, 0x27,  // CTRL_REG1_A: 10Hz, all axes enabled
            0x23, 0x00,  // CTRL_REG4_A: ±2g, continuous update
        };

        for(size_t i = 0; i < sizeof(accel_config); i += 2) {
            furi_hal_i2c_tx(
                FuriHalI2cBusExternal,
                LSM303_ACCEL_ADDR,
                &accel_config[i],
                2,
                100
            );
        }

        // Configure magnetometer
        uint8_t mag_config[] = {
            0x00, 0x10,  // CRA_REG_M: 15Hz output rate
            0x01, 0x20,  // CRB_REG_M: ±1.3 gauss
            0x02, 0x00,  // MR_REG_M: Continuous conversion
        };

        for(size_t i = 0; i < sizeof(mag_config); i += 2) {
            furi_hal_i2c_tx(
                FuriHalI2cBusExternal,
                LSM303_MAG_ADDR,
                &mag_config[i],
                2,
                100
            );
        }
    }

    furi_hal_i2c_release(FuriHalI2cBusExternal);

    return accel_present && mag_present;
}

// Read accelerometer data
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} AccelData;

bool lsm303_read_accel(AccelData* data) {
    bool success = false;

    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    // Read 6 bytes starting at OUT_X_L_A (0x28)
    // Use AUTO_INCREMENT bit (0x80)
    uint8_t reg = 0x28 | 0x80;
    uint8_t buffer[6];

    success = furi_hal_i2c_trx(
        FuriHalI2cBusExternal,
        LSM303_ACCEL_ADDR,
        &reg, 1,
        buffer, 6,
        100
    );

    if(success) {
        data->x = (int16_t)((buffer[1] << 8) | buffer[0]);
        data->y = (int16_t)((buffer[3] << 8) | buffer[2]);
        data->z = (int16_t)((buffer[5] << 8) | buffer[4]);
    }

    furi_hal_i2c_release(FuriHalI2cBusExternal);

    return success;
}

// Convert raw to g units
void accel_to_g(const AccelData* raw, float* x, float* y, float* z) {
    // LSM303 @±2g: 1mg/LSB = 1/1000 g per bit
    // Full scale: 16-bit signed = ±32768
    // At ±2g: 1g = 16384 LSB
    const float scale = 1.0f / 16384.0f;

    *x = raw->x * scale;
    *y = raw->y * scale;
    *z = raw->z * scale;
}
```

### LSM303 Magnetometer

```c
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} MagData;

bool lsm303_read_mag(MagData* data) {
    bool success = false;

    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    // Read magnetometer (MSB first on LSM303)
    uint8_t reg = 0x03;  // OUT_X_H_M
    uint8_t buffer[6];

    // Note: magnetometer doesn't support auto-increment
    // Read X, Y, Z separately
    for(int axis = 0; axis < 3; axis++) {
        reg = 0x03 + (axis * 2);  // X, Y, Z registers
        success = furi_hal_i2c_trx(
            FuriHalI2cBusExternal,
            LSM303_MAG_ADDR,
            &reg, 1,
            &buffer[axis * 2], 2,
            100
        );
        if(!success) break;
    }

    if(success) {
        // MSB first (big-endian)
        data->x = (int16_t)((buffer[0] << 8) | buffer[1]);
        data->y = (int16_t)((buffer[2] << 8) | buffer[3]);
        data->z = (int16_t)((buffer[4] << 8) | buffer[5]);
    }

    furi_hal_i2c_release(FuriHalI2cBusExternal);

    return success;
}

// Calculate heading (compass)
float mag_calculate_heading(const MagData* mag, const AccelData* accel) {
    // Convert to g
    float ax, ay, az;
    accel_to_g(accel, &ax, &ay, &az);

    // Normalize accelerometer
    float norm = sqrtf(ax*ax + ay*ay + az*az);
    if(norm < 0.01f) return 0.0f;  // Freefall, can't compute

    ax /= norm;
    ay /= norm;
    az /= norm;

    // Tilt compensation
    float mx = mag->x;
    float my = mag->y;
    float mz = mag->z;

    float pitch = asinf(-ax);
    float roll = atan2f(ay, az);

    float cos_pitch = cosf(pitch);
    float sin_pitch = sinf(pitch);
    float cos_roll = cosf(roll);
    float sin_roll = sinf(roll);

    // Tilt-compensated magnetic field
    float mx_comp = mx * cos_pitch + mz * sin_pitch;
    float my_comp = mx * sin_roll * sin_pitch + my * cos_roll - mz * sin_roll * cos_pitch;

    // Calculate heading (0-360°)
    float heading = atan2f(my_comp, mx_comp) * (180.0f / M_PI);
    if(heading < 0) heading += 360.0f;

    return heading;
}
```

### GXHTC3C Temperature & Humidity

**Location**: `applications/kiisu_sensor_hub/src/drivers/gxhtc3c.h`

```c
#define GXHTC3C_ADDR 0x70

bool gxhtc3c_read(float* temp, float* humidity) {
    bool success = false;

    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    // Wake up sensor
    uint8_t wakeup_cmd[] = {0x35, 0x17};
    furi_hal_i2c_tx(FuriHalI2cBusExternal, GXHTC3C_ADDR, wakeup_cmd, 2, 100);

    // Wait for wake-up
    furi_delay_ms(1);

    // Trigger measurement (normal mode)
    uint8_t measure_cmd[] = {0x78, 0x66};
    furi_hal_i2c_tx(FuriHalI2cBusExternal, GXHTC3C_ADDR, measure_cmd, 2, 100);

    // Wait for measurement (max 12.1ms for normal mode)
    furi_delay_ms(15);

    // Read data (6 bytes: temp MSB, temp LSB, temp CRC, hum MSB, hum LSB, hum CRC)
    uint8_t data[6];
    success = furi_hal_i2c_rx(
        FuriHalI2cBusExternal,
        GXHTC3C_ADDR,
        data,
        6,
        100
    );

    if(success) {
        // Parse temperature
        uint16_t temp_raw = (data[0] << 8) | data[1];
        *temp = -45.0f + 175.0f * (temp_raw / 65535.0f);

        // Parse humidity
        uint16_t hum_raw = (data[3] << 8) | data[4];
        *humidity = 100.0f * (hum_raw / 65535.0f);

        // TODO: Validate CRC (data[2] and data[5])
    }

    // Sleep sensor
    uint8_t sleep_cmd[] = {0xB0, 0x98};
    furi_hal_i2c_tx(FuriHalI2cBusExternal, GXHTC3C_ADDR, sleep_cmd, 2, 100);

    furi_hal_i2c_release(FuriHalI2cBusExternal);

    return success;
}
```

### Light Sensor (ADC)

**Location**: `applications/kiisu_sensor_hub/src/drivers/kiisu_light_adc.h`

```c
#include <furi_hal_adc.h>

// Configure light sensor ADC
void kiisu_light_init(void) {
    // ADC pin is typically PA4, PA6, or PA7 (check hardware)
    const GpioPin* adc_pin = &gpio_ext_pa4;

    // Configure pin for analog input
    furi_hal_gpio_init(adc_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

// Read light sensor
float kiisu_light_read(void) {
    // Acquire ADC
    furi_hal_adc_acquire();

    // Read ADC value (12-bit: 0-4095)
    uint16_t raw = furi_hal_adc_read(FuriHalAdcChannel16);  // PA4

    // Release ADC
    furi_hal_adc_release();

    // Convert to voltage (assuming 2.5V reference)
    float voltage = (raw / 4095.0f) * 2.5f;

    // Convert to percentage (adjust based on sensor characteristics)
    float percent = (voltage / 2.5f) * 100.0f;

    return percent;
}
```

---

## LED Control

### Basic LED Commands

```c
typedef enum {
    LED_COLOR_OFF = 0,
    LED_COLOR_RED = 1,
    LED_COLOR_GREEN = 2,
    LED_COLOR_BLUE = 3,
    LED_COLOR_YELLOW = 4,
    LED_COLOR_CYAN = 5,
    LED_COLOR_MAGENTA = 6,
    LED_COLOR_WHITE = 7,
    LED_COLOR_PURPLE = 8,
} LedColor;

void kiisu_led_set_color(LedColor color) {
    uint8_t cmd_data[] = {0x02, color};  // CMD_SET_LED_COLOR
    kiisu_send_command(0x02, &cmd_data[1], 1);
}

void kiisu_led_set_brightness(uint8_t percent) {
    if(percent > 100) percent = 100;

    uint8_t cmd_data[] = {0x01, percent};  // CMD_SET_LED_BRIGHTNESS
    kiisu_send_command(0x01, &cmd_data[1], 1);
}

// Example: Rainbow cycle
void kiisu_led_rainbow_demo(void) {
    LedColor colors[] = {
        LED_COLOR_RED,
        LED_COLOR_YELLOW,
        LED_COLOR_GREEN,
        LED_COLOR_CYAN,
        LED_COLOR_BLUE,
        LED_COLOR_MAGENTA,
    };

    for(size_t i = 0; i < COUNT_OF(colors); i++) {
        kiisu_led_set_color(colors[i]);
        furi_delay_ms(500);
    }

    kiisu_led_set_color(LED_COLOR_OFF);
}
```

---

## Complete Example: Motion Detector

```c
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_i2c.h>
#include <gui/gui.h>
#include <notification/notification_messages.h>

#define LSM303_ACCEL_ADDR 0x19
#define MOTION_THRESHOLD  0.5f  // g units

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    NotificationApp* notifications;
    bool motion_detected;
    float last_x, last_y, last_z;
} MotionDetectorApp;

// Initialize LSM303
bool init_accelerometer(void) {
    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    if(!furi_hal_i2c_is_device_ready(FuriHalI2cBusExternal, LSM303_ACCEL_ADDR, 100)) {
        furi_hal_i2c_release(FuriHalI2cBusExternal);
        return false;
    }

    // Configure: 100Hz, all axes enabled
    uint8_t config[] = {0x20, 0x57};
    furi_hal_i2c_tx(FuriHalI2cBusExternal, LSM303_ACCEL_ADDR, config, 2, 100);

    furi_hal_i2c_release(FuriHalI2cBusExternal);
    return true;
}

// Read acceleration
bool read_accel(float* x, float* y, float* z) {
    furi_hal_i2c_acquire(FuriHalI2cBusExternal);

    uint8_t reg = 0x28 | 0x80;  // OUT_X_L_A with auto-increment
    uint8_t data[6];

    bool success = furi_hal_i2c_trx(
        FuriHalI2cBusExternal,
        LSM303_ACCEL_ADDR,
        &reg, 1,
        data, 6,
        100
    );

    furi_hal_i2c_release(FuriHalI2cBusExternal);

    if(success) {
        int16_t x_raw = (int16_t)((data[1] << 8) | data[0]);
        int16_t y_raw = (int16_t)((data[3] << 8) | data[2]);
        int16_t z_raw = (int16_t)((data[5] << 8) | data[4]);

        *x = x_raw / 16384.0f;  // ±2g scale
        *y = y_raw / 16384.0f;
        *z = z_raw / 16384.0f;
    }

    return success;
}

// Draw callback
void motion_detector_draw(Canvas* canvas, void* ctx) {
    MotionDetectorApp* app = ctx;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 20, 20, "Motion Detector");

    canvas_set_font(canvas, FontSecondary);

    if(app->motion_detected) {
        canvas_draw_str(canvas, 30, 35, "MOTION!");
        canvas_draw_icon(canvas, 50, 40, &I_Warning_30x23);
    } else {
        canvas_draw_str(canvas, 30, 35, "No motion");
    }

    // Show acceleration values
    char buf[32];
    snprintf(buf, sizeof(buf), "X: %.2f g", app->last_x);
    canvas_draw_str(canvas, 5, 50, buf);

    snprintf(buf, sizeof(buf), "Y: %.2f g", app->last_y);
    canvas_draw_str(canvas, 5, 60, buf);

    snprintf(buf, sizeof(buf), "Z: %.2f g", app->last_z);
    canvas_draw_str(canvas, 5, 70, buf);
}

// Input callback
bool motion_detector_input(InputEvent* event, void* ctx) {
    UNUSED(ctx);

    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        return false;  // Exit app
    }

    return true;
}

// Main app entry
int32_t motion_detector_app(void* p) {
    UNUSED(p);

    MotionDetectorApp* app = malloc(sizeof(MotionDetectorApp));
    memset(app, 0, sizeof(MotionDetectorApp));

    // Initialize
    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, motion_detector_draw, app);
    view_port_input_callback_set(app->view_port, motion_detector_input, app);

    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    // Initialize accelerometer
    if(!init_accelerometer()) {
        FURI_LOG_E("MotionDetector", "Failed to initialize accelerometer");
        // Show error and exit
        gui_remove_view_port(app->gui, app->view_port);
        view_port_free(app->view_port);
        furi_record_close(RECORD_NOTIFICATION);
        furi_record_close(RECORD_GUI);
        free(app);
        return -1;
    }

    // Read initial values
    read_accel(&app->last_x, &app->last_y, &app->last_z);

    // Main loop
    bool running = true;
    while(running) {
        float x, y, z;

        if(read_accel(&x, &y, &z)) {
            // Calculate delta
            float dx = fabsf(x - app->last_x);
            float dy = fabsf(y - app->last_y);
            float dz = fabsf(z - app->last_z);

            // Detect motion
            if(dx > MOTION_THRESHOLD || dy > MOTION_THRESHOLD || dz > MOTION_THRESHOLD) {
                app->motion_detected = true;

                // Trigger notification
                notification_message(app->notifications, &sequence_single_vibro);

                FURI_LOG_I("MotionDetector", "Motion: dx=%.2f dy=%.2f dz=%.2f", dx, dy, dz);
            } else {
                app->motion_detected = false;
            }

            // Update last values
            app->last_x = x;
            app->last_y = y;
            app->last_z = z;
        }

        view_port_update(app->view_port);
        furi_delay_ms(100);  // 10Hz polling

        // Check if back button pressed
        if(!view_port_is_enabled(app->view_port)) {
            running = false;
        }
    }

    // Cleanup
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    free(app);

    return 0;
}
```

---

## Best Practices

### 1. I2C Bus Management

✅ **DO**:
- Always acquire/release I2C bus
- Use appropriate timeouts (100-1000ms)
- Check device presence before operations
- Handle communication errors gracefully

❌ **DON'T**:
- Hold I2C bus longer than necessary
- Assume device is always present
- Ignore return values
- Block indefinitely

### 2. Power Management

✅ **DO**:
- Enable external 3.3V when using sensors
- Disable power when done
- Use sleep modes for battery life
- Check USB power for firmware updates

❌ **DON'T**:
- Leave external power on indefinitely
- Drain battery with continuous polling
- Update firmware without USB power

### 3. Sensor Calibration

✅ **DO**:
- Provide calibration UI for magnetometer
- Store calibration in persistent storage
- Validate calibration data
- Document calibration procedures

❌ **DON'T**:
- Skip magnetometer calibration
- Assume factory calibration is sufficient
- Discard calibration data on app exit

### 4. Error Handling

✅ **DO**:
- Check all I2C return values
- Detect missing hardware gracefully
- Provide user feedback on errors
- Log errors for debugging

❌ **DON'T**:
- Crash on I2C failures
- Continue with invalid data
- Hide errors from user

### 5. Threading & Performance

✅ **DO**:
- Use appropriate stack sizes (4-6 KB typical)
- Poll sensors at reasonable rates (10-100 Hz)
- Update GUI efficiently
- Use async operations when possible

❌ **DON'T**:
- Block GUI thread with long operations
- Poll sensors faster than needed
- Redraw entire screen unnecessarily

---

## Debugging

### Common Issues

**I2C Communication Fails**:
```c
// Enable debug logging
FURI_LOG_E("App", "I2C error at address 0x%02X", addr);

// Check device presence
if(!furi_hal_i2c_is_device_ready(bus, addr, timeout)) {
    FURI_LOG_E("App", "Device not found");
}

// Verify power
furi_hal_power_enable_external_3_3v();
furi_delay_ms(10);  // Allow power to stabilize
```

**Sensor Data Invalid**:
```c
// Validate ranges
if(fabsf(accel_x) > 2.0f) {  // ±2g range
    FURI_LOG_W("App", "Accel out of range");
}

// Check for NaN/Inf
if(!isfinite(value)) {
    FURI_LOG_E("App", "Invalid sensor value");
}
```

**Stack Overflow**:
```c
// Increase stack size in application.fam
App(
    # ...
    stack_size=6 * 1024,  // 6 KB instead of 4 KB
)
```

---

## Resources

### Driver References

- LSM303 sensor: `applications/kiisu_sensor_hub/src/drivers/lsm303.c`
- GXHTC3C sensor: `applications/kiisu_sensor_hub/src/drivers/gxhtc3c.c`
- Light sensor: `applications/kiisu_sensor_hub/src/drivers/kiisu_light_adc.c`
- I2C utilities: `applications/kiisu_sensor_hub/src/drivers/i2c_bus.c`

### Documentation

- [KIISU_OVERVIEW.md](KIISU_OVERVIEW.md) - Kiisu platform overview
- [FuriHalAPI.md](FuriHalAPI.md) - HAL API reference
- [hardware/gpio.md](hardware/gpio.md) - GPIO guide
- [AppManifests.md](AppManifests.md) - Application manifests
- [fbt.md](fbt.md) - Build system

### External Resources

- [Enhanced Kiisu4 Firmware](https://github.com/twoelw/enhanced-kiisu4-fw) - Companion firmware
- LSM303 Datasheet (STMicroelectronics)
- SHTC3 Datasheet (Sensirion) - GXHTC3C compatible

---

## Example Projects

See existing Kiisu apps for reference:
- **Kiisu Manager**: Settings UI (`applications/kiisu-manager/`)
- **Companion Bridge**: Firmware updater (`applications/kiisu_companion_bridge/`)
- **Sensor Hub**: Complete sensor dashboard (`applications/kiisu_sensor_hub/`)

---

**Oracle Documentation** - Kiisu development mastery complete.
