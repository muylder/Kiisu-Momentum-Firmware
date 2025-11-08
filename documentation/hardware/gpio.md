# GPIO Hardware Guide

Complete guide to GPIO (General Purpose Input/Output) on Flipper Zero.

## Overview

Flipper Zero provides **8 external GPIO pins** accessible through the top connector, plus internal GPIOs used for buttons, LEDs, radios, and other peripherals.

**Header Location**: `targets/furi_hal_include/furi_hal_gpio.h`
**Resources**: `targets/f7/furi_hal/furi_hal_resources.h`
**Power**: External GPIOs share 3.3V rail (requires enabling via HAL)

---

## External GPIO Pinout

### Pin Mapping

| Pin Name | STM32 Pin | Default Function | Alternate Functions | ADC | PWM |
|----------|-----------|------------------|---------------------|-----|-----|
| **PC0** | PC0 | GPIO | USART6_TX | - | PWM |
| **PC1** | PC1 | GPIO | USART6_RX | - | - |
| **PC3** | PC3 | GPIO | LPUART1_RX | ADC1_IN4 | - |
| **PB2** | PB2 | GPIO | SPI2_MOSI | - | PWM |
| **PB3** | PB3 | GPIO | SPI2_MISO, SPI1_SCK | - | PWM |
| **PA4** | PA4 | GPIO | SPI1_SS, ADC | ADC1_IN9 | - |
| **PA6** | PA6 | GPIO | SPI1_MISO, ADC | ADC1_IN11 | PWM |
| **PA7** | PA7 | GPIO | SPI1_MOSI | ADC1_IN12 | PWM |
| **GND** | - | Ground | - | - | - |
| **3V3** | - | Power (req. enable) | - | - | - |
| **5V** | - | USB Power | - | - | - |

### Physical Connector

```
┌─────────────────┐
│  1  3  5  7  9  │  Top View
│  2  4  6  8 10  │  (Component side up)
└─────────────────┘

Pin 1: PC0        Pin 2: GND
Pin 3: PC1        Pin 4: GND
Pin 5: PC3        Pin 6: 3V3 (switchable)
Pin 7: PB2        Pin 8: GND
Pin 9: PB3        Pin 10: 5V (USB)
Pin 11: PA4       Pin 12: GND
Pin 13: PA6       Pin 14: GND
Pin 15: PA7       Pin 16: GND
Pin 17: SWCLK     Pin 18: SWDIO
```

**Important**: Always enable 3.3V rail before using external devices:
```c
furi_hal_power_enable_external_3_3v();
```

---

## GPIO Modes

### Available Modes

```c
typedef enum {
    GpioModeInput,                // Digital input (high-Z)
    GpioModeOutputPushPull,       // Digital output (0V/3.3V)
    GpioModeOutputOpenDrain,      // Open-drain output (0V/Hi-Z)
    GpioModeAltFunctionPushPull,  // Alternate function (UART, SPI, etc.)
    GpioModeAltFunctionOpenDrain, // Alt function open-drain
    GpioModeAnalog,               // Analog mode (for ADC)
    GpioModeInterruptRise,        // Interrupt on rising edge
    GpioModeInterruptFall,        // Interrupt on falling edge
    GpioModeInterruptRiseFall,    // Interrupt on both edges
    GpioModeEventRise,            // Event on rising edge (no ISR)
    GpioModeEventFall,            // Event on falling edge (no ISR)
    GpioModeEventRiseFall,        // Event on both edges (no ISR)
} GpioMode;
```

### Pull Resistors

```c
typedef enum {
    GpioPullNo,    // No pull resistor (floating)
    GpioPullUp,    // Pull-up (~40kΩ to 3.3V)
    GpioPullDown,  // Pull-down (~40kΩ to GND)
} GpioPull;
```

### Speed Settings

```c
typedef enum {
    GpioSpeedLow,        // 2 MHz (default for most)
    GpioSpeedMedium,     // 12.5 MHz
    GpioSpeedHigh,       // 50 MHz
    GpioSpeedVeryHigh,   // 100 MHz (use sparingly)
} GpioSpeed;
```

---

## Basic GPIO Operations

### Digital Output

```c
#include <furi_hal.h>
#include <furi_hal_resources.h>

// Get pin reference
const GpioPin* led_pin = &gpio_ext_pa7;

// Enable external 3.3V
furi_hal_power_enable_external_3_3v();

// Configure as output
furi_hal_gpio_init_simple(led_pin, GpioModeOutputPushPull);

// Set pin HIGH
furi_hal_gpio_write(led_pin, true);

// Set pin LOW
furi_hal_gpio_write(led_pin, false);

// Toggle pin
bool state = !furi_hal_gpio_read(led_pin);
furi_hal_gpio_write(led_pin, state);

// When done, reset to safe state
furi_hal_gpio_init_simple(led_pin, GpioModeAnalog);
furi_hal_power_disable_external_3_3v();
```

### Digital Input

```c
const GpioPin* button_pin = &gpio_ext_pc0;

// Enable external 3.3V
furi_hal_power_enable_external_3_3v();

// Configure as input with pull-up
furi_hal_gpio_init(button_pin, GpioModeInput, GpioPullUp, GpioSpeedLow);

// Read pin state
bool button_pressed = !furi_hal_gpio_read(button_pin);  // Active LOW

// Configure with pull-down (active HIGH)
furi_hal_gpio_init(button_pin, GpioModeInput, GpioPullDown, GpioSpeedLow);
bool button_pressed = furi_hal_gpio_read(button_pin);

// Reset when done
furi_hal_gpio_init_simple(button_pin, GpioModeAnalog);
furi_hal_power_disable_external_3_3v();
```

### Open-Drain Output

Useful for I2C, 1-Wire, and level shifting:

```c
const GpioPin* od_pin = &gpio_ext_pb2;

// Configure as open-drain with pull-up
furi_hal_gpio_init(od_pin, GpioModeOutputOpenDrain, GpioPullUp, GpioSpeedLow);

// Pull LOW (active)
furi_hal_gpio_write(od_pin, false);

// Release (HIGH via pull-up)
furi_hal_gpio_write(od_pin, true);
```

---

## GPIO Interrupts

### Setting Up Interrupts

```c
void gpio_isr_callback(void* context) {
    // WARNING: This runs in ISR context!
    // Keep this VERY SHORT - no delays, no logging

    MyApp* app = context;

    // Signal event to main thread
    furi_thread_flags_set(furi_thread_get_id(app->thread), FLAG_GPIO_EVENT);
}

void setup_gpio_interrupt(void) {
    const GpioPin* int_pin = &gpio_ext_pc1;

    // Enable power
    furi_hal_power_enable_external_3_3v();

    // Configure as interrupt input
    furi_hal_gpio_init(int_pin, GpioModeInterruptRise, GpioPullDown, GpioSpeedLow);

    // Register callback
    furi_hal_gpio_add_int_callback(int_pin, gpio_isr_callback, &app_context);
}

void cleanup_gpio_interrupt(void) {
    const GpioPin* int_pin = &gpio_ext_pc1;

    // Remove callback
    furi_hal_gpio_remove_int_callback(int_pin);

    // Reset pin
    furi_hal_gpio_init_simple(int_pin, GpioModeAnalog);
    furi_hal_power_disable_external_3_3v();
}
```

### Interrupt Best Practices

1. **Keep ISRs SHORT** - No delays, no logging, no heavy processing
2. **Signal threads** - Use thread flags or message queues
3. **Avoid shared data** - Or use critical sections
4. **Clean up** - Always remove callbacks before freeing context

### Example: Interrupt-Driven App

```c
typedef struct {
    FuriThread* thread;
    volatile uint32_t event_count;
} GpioApp;

#define FLAG_GPIO_EVENT (1 << 0)

void gpio_isr(void* context) {
    GpioApp* app = context;
    app->event_count++;
    furi_thread_flags_set(furi_thread_get_id(app->thread), FLAG_GPIO_EVENT);
}

int32_t gpio_app(void* p) {
    GpioApp app = {
        .thread = furi_thread_get_current(),
        .event_count = 0,
    };

    const GpioPin* pin = &gpio_ext_pc0;

    furi_hal_power_enable_external_3_3v();
    furi_hal_gpio_init(pin, GpioModeInterruptRiseFall, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_add_int_callback(pin, gpio_isr, &app);

    while(true) {
        uint32_t flags = furi_thread_flags_wait(FLAG_GPIO_EVENT, FuriFlagWaitAny, 1000);

        if(flags & FLAG_GPIO_EVENT) {
            FURI_LOG_I("GPIO", "Events: %lu", app.event_count);
        }
    }

    furi_hal_gpio_remove_int_callback(pin);
    furi_hal_gpio_init_simple(pin, GpioModeAnalog);
    furi_hal_power_disable_external_3_3v();

    return 0;
}
```

---

## Advanced GPIO Features

### PWM Output

Some pins support PWM via timers:

```c
#include <furi_hal_pwm.h>

// PWM available on: PC0, PB2, PB3, PA6, PA7
const GpioPin* pwm_pin = &gpio_ext_pa7;

// Start PWM (50% duty cycle, 1 kHz)
furi_hal_pwm_start(FuriHalPwmOutputIdLptim2PA4, 1000, 50);

// Update duty cycle (0-100%)
furi_hal_pwm_set_params(FuriHalPwmOutputIdLptim2PA4, 1000, 25);

// Stop PWM
furi_hal_pwm_stop(FuriHalPwmOutputIdLptim2PA4);
```

### ADC Input

Pins with ADC capability (PA4, PA6, PA7, PC3):

```c
#include <furi_hal_adc.h>

const GpioPin* adc_pin = &gpio_ext_pa4;

// Enable 3.3V and ADC
furi_hal_power_enable_external_3_3v();
furi_hal_adc_acquire();

// Configure pin for analog
furi_hal_gpio_init(adc_pin, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

// Read ADC value (12-bit: 0-4095)
uint16_t raw = furi_hal_adc_read(FuriHalAdcChannel16);  // PA4

// Convert to voltage (assuming 2.5V VREF)
float voltage = (raw / 4095.0f) * 2.5f;

// Cleanup
furi_hal_adc_release();
furi_hal_gpio_init_simple(adc_pin, GpioModeAnalog);
furi_hal_power_disable_external_3_3v();
```

---

## Internal GPIO Resources

### Buttons

```c
extern const GpioPin gpio_button_up;
extern const GpioPin gpio_button_down;
extern const GpioPin gpio_button_left;
extern const GpioPin gpio_button_right;
extern const GpioPin gpio_button_ok;
extern const GpioPin gpio_button_back;

// Read button state (use InputService instead for apps)
bool up_pressed = !furi_hal_gpio_read(&gpio_button_up);  // Active LOW
```

### Vibration Motor

```c
extern const GpioPin gpio_vibro;  // PA8

// Use HAL API instead of direct GPIO
furi_hal_vibro_on(true);
furi_delay_ms(200);
furi_hal_vibro_on(false);
```

### iButton/1-Wire

```c
extern const GpioPin gpio_ibutton;  // PB14

// Use HAL API for 1-Wire protocol
furi_hal_ibutton_start();
// ... 1-Wire operations ...
furi_hal_ibutton_stop();
```

### Infrared

```c
extern const GpioPin gpio_infrared_rx;  // PA0
extern const GpioPin gpio_infrared_tx;  // PB9

// Use HAL API for IR
furi_hal_infrared_async_rx_start();
// ...
furi_hal_infrared_async_rx_stop();
```

### NFC

```c
extern const GpioPin gpio_nfc_cs;        // PE4 - Chip select
extern const GpioPin gpio_nfc_irq_rfid_pull;  // PA2 - IRQ/RFID pull
```

### Sub-GHz Radio

```c
extern const GpioPin gpio_subghz_cs;   // PD0 - CC1101 chip select
extern const GpioPin gpio_cc1101_g0;   // PA1 - CC1101 GDO0
extern const GpioPin gpio_rf_sw_0;     // PC4 - RF switch
```

### Display

```c
extern const GpioPin gpio_display_cs;      // PC11 - Chip select
extern const GpioPin gpio_display_rst_n;   // PB0 - Reset
extern const GpioPin gpio_display_di;      // PB1 - Data/Instruction
```

### SD Card

```c
extern const GpioPin gpio_sdcard_cs;   // PC12 - Chip select
extern const GpioPin gpio_sdcard_cd;   // PC10 - Card detect
```

---

## Power Management

### External 3.3V Control

The external GPIO 3.3V rail is **switchable** and **shared** with the SD card:

```c
// Enable 3.3V (required for external devices)
furi_hal_power_enable_external_3_3v();

// Wait for power stabilization
furi_delay_ms(10);

// ... use external GPIO ...

// Disable when done (saves power)
furi_hal_power_disable_external_3_3v();
```

**Important**:
- External 3.3V is OFF by default
- SD card operations automatically enable/disable 3.3V
- Multiple components can use 3.3V simultaneously
- Always disable when done to save battery

### Current Limits

- **External 3.3V**: Max 1A total (shared between SD card and GPIO)
- **5V USB**: Max 500mA (when USB connected)
- **Per GPIO**: Max 25mA source/sink (STM32 limit)

---

## GPIO Resources Table

### External Pins with All Capabilities

| Pin | GPIO | UART | SPI | ADC | PWM | Notes |
|-----|------|------|-----|-----|-----|-------|
| PC0 | ✓ | TX6 | - | - | ✓ | USART6_TX, PWM |
| PC1 | ✓ | RX6 | - | - | - | USART6_RX |
| PC3 | ✓ | LP_RX | - | ✓ | - | LPUART1_RX, ADC1_IN4 |
| PB2 | ✓ | - | MOSI2 | - | ✓ | SPI2_MOSI, PWM |
| PB3 | ✓ | - | MISO2/SCK1 | - | ✓ | SPI2_MISO or SPI1_SCK, PWM |
| PA4 | ✓ | - | SS1 | ✓ | - | SPI1_SS, ADC1_IN9 |
| PA6 | ✓ | - | MISO1 | ✓ | ✓ | SPI1_MISO, ADC1_IN11, PWM |
| PA7 | ✓ | - | MOSI1 | ✓ | ✓ | SPI1_MOSI, ADC1_IN12, PWM |

---

## Example Projects

### LED Blinker

```c
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_resources.h>

int32_t gpio_blink_app(void* p) {
    UNUSED(p);

    const GpioPin* led = &gpio_ext_pa7;

    // Enable power
    furi_hal_power_enable_external_3_3v();
    furi_delay_ms(10);

    // Configure pin
    furi_hal_gpio_init_simple(led, GpioModeOutputPushPull);

    // Blink 10 times
    for(int i = 0; i < 10; i++) {
        furi_hal_gpio_write(led, true);
        furi_delay_ms(500);
        furi_hal_gpio_write(led, false);
        furi_delay_ms(500);
    }

    // Cleanup
    furi_hal_gpio_init_simple(led, GpioModeAnalog);
    furi_hal_power_disable_external_3_3v();

    return 0;
}
```

### Button Reader

```c
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_resources.h>

int32_t gpio_button_app(void* p) {
    UNUSED(p);

    const GpioPin* button = &gpio_ext_pc0;
    const GpioPin* led = &gpio_ext_pa7;

    // Setup
    furi_hal_power_enable_external_3_3v();
    furi_delay_ms(10);

    furi_hal_gpio_init(button, GpioModeInput, GpioPullUp, GpioSpeedLow);
    furi_hal_gpio_init_simple(led, GpioModeOutputPushPull);

    // Read button for 30 seconds
    for(int i = 0; i < 300; i++) {
        bool pressed = !furi_hal_gpio_read(button);  // Active LOW
        furi_hal_gpio_write(led, pressed);
        furi_delay_ms(100);
    }

    // Cleanup
    furi_hal_gpio_init_simple(button, GpioModeAnalog);
    furi_hal_gpio_init_simple(led, GpioModeAnalog);
    furi_hal_power_disable_external_3_3v();

    return 0;
}
```

### PWM Dimmer

```c
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_pwm.h>

int32_t gpio_pwm_app(void* p) {
    UNUSED(p);

    // Start PWM on PA7 at 1 kHz
    furi_hal_power_enable_external_3_3v();
    furi_hal_pwm_start(FuriHalPwmOutputIdLptim2PA4, 1000, 0);

    // Fade in
    for(int duty = 0; duty <= 100; duty += 5) {
        furi_hal_pwm_set_params(FuriHalPwmOutputIdLptim2PA4, 1000, duty);
        furi_delay_ms(50);
    }

    // Fade out
    for(int duty = 100; duty >= 0; duty -= 5) {
        furi_hal_pwm_set_params(FuriHalPwmOutputIdLptim2PA4, 1000, duty);
        furi_delay_ms(50);
    }

    // Cleanup
    furi_hal_pwm_stop(FuriHalPwmOutputIdLptim2PA4);
    furi_hal_power_disable_external_3_3v();

    return 0;
}
```

---

## Troubleshooting

### Common Issues

**GPIO not working**:
- Verify `furi_hal_power_enable_external_3_3v()` called
- Wait 10ms after enabling power
- Check pin configuration (mode, pull, speed)
- Verify pin is not used by another peripheral

**Interrupt not firing**:
- Check edge configuration (rise/fall/both)
- Verify pull resistor matches expected signal
- Ensure callback is registered before event
- Keep ISR handler SHORT

**Erratic readings**:
- Add pull resistor (internal or external)
- Check for floating inputs
- Add debouncing (hardware or software)
- Verify power supply is stable

**ADC readings incorrect**:
- Configure pin as `GpioModeAnalog`
- Acquire ADC before reading
- Check voltage reference setting
- Verify input voltage within range (0-2.5V typical)

---

## Safety Guidelines

1. **Voltage**: External pins are **3.3V tolerant** - DO NOT apply 5V directly
2. **Current**: Max 25mA per pin - Use transistor for high-current loads
3. **ESD**: Use ESD protection when connecting external cables
4. **Power**: Always disable 3.3V when done to save battery
5. **Cleanup**: Reset pins to `GpioModeAnalog` when finished

---

## Related Documentation

- [FuriHalAPI.md](../FuriHalAPI.md) - Complete HAL API reference
- [ExpansionModules.md](../ExpansionModules.md) - Expansion module development
- [furi_hal_gpio.h](../../targets/furi_hal_include/furi_hal_gpio.h) - GPIO API header
- [furi_hal_resources.h](../../targets/f7/furi_hal/furi_hal_resources.h) - Pin definitions

---

**Oracle Documentation** - GPIO mastery complete.
