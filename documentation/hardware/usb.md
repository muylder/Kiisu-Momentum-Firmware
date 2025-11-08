# USB Hardware Guide

Complete guide to USB functionality on Flipper Zero.

## Overview

Flipper Zero provides **USB 2.0 Full Speed** connectivity with multiple device modes:

- **VCP (Virtual COM Port)**: Serial communication
- **HID (Human Interface Device)**: Keyboard, Mouse, Consumer Control
- **CCID (Smart Card)**: Smart card emulation
- **Mass Storage** (via apps): USB drive mode
- **U2F**: Universal 2nd Factor authentication

**Hardware**: STM32WB55 USB 2.0 Full Speed controller
**Speed**: 12 Mbps (Full Speed)
**Power**: USB provides 5V to Flipper (charging + 5V GPIO pin)

---

## USB Modes

### Available Modes

```c
#include <furi_hal_usb.h>

// Single VCP (Virtual COM Port)
extern FuriHalUsbInterface usb_cdc_single;

// Dual VCP (two independent serial ports)
extern FuriHalUsbInterface usb_cdc_dual;

// HID (Keyboard + Mouse + Consumer Control)
extern FuriHalUsbInterface usb_hid;

// HID + U2F
extern FuriHalUsbInterface usb_hid_u2f;

// CCID (Smart Card emulation)
extern FuriHalUsbInterface usb_ccid;
```

### Switching USB Modes

```c
// Check if mode switch is locked
if(furi_hal_usb_is_locked()) {
    FURI_LOG_W("USB", "Mode switch locked");
    return false;
}

// Switch to HID mode
if(!furi_hal_usb_set_config(&usb_hid, NULL)) {
    FURI_LOG_E("USB", "Failed to set USB mode");
    return false;
}

// Lock mode (prevent other apps from changing it)
furi_hal_usb_lock();

// ... use USB ...

// Unlock mode
furi_hal_usb_unlock();
```

### Getting Current Mode

```c
FuriHalUsbInterface* current_mode = furi_hal_usb_get_config();

if(current_mode == &usb_hid) {
    FURI_LOG_I("USB", "HID mode active");
} else if(current_mode == &usb_cdc_single) {
    FURI_LOG_I("USB", "VCP mode active");
}
```

---

## VCP (Virtual COM Port)

Virtual COM Port provides serial communication over USB.

### Single VCP Mode

```c
#include <furi_hal_usb.h>
#include <furi_hal_vcp.h>

// Switch to VCP mode
furi_hal_usb_set_config(&usb_cdc_single, NULL);

// Wait for USB enumeration
furi_delay_ms(100);

// Check if VCP is connected
if(!furi_hal_vcp_is_connected()) {
    FURI_LOG_W("VCP", "Not connected");
}

// Send data
const char* message = "Hello from Flipper!\r\n";
furi_hal_vcp_tx((uint8_t*)message, strlen(message));

// Receive data (non-blocking)
uint8_t buffer[64];
size_t received = furi_hal_vcp_rx(buffer, sizeof(buffer));
if(received > 0) {
    // Process received data
}
```

### Dual VCP Mode

Dual VCP provides two independent serial ports:

```c
// Switch to dual VCP
furi_hal_usb_set_config(&usb_cdc_dual, NULL);

// Port 0: Primary VCP
furi_hal_vcp_tx(0, data, size);

// Port 1: Secondary VCP
furi_hal_vcp_tx(1, data, size);
```

### VCP with Callback

```c
typedef struct {
    bool running;
    FuriMessageQueue* queue;
} VcpApp;

void vcp_rx_callback(void* context) {
    VcpApp* app = context;

    uint8_t buffer[128];
    size_t received = furi_hal_vcp_rx(buffer, sizeof(buffer));

    if(received > 0) {
        // Send received data to queue for processing
        for(size_t i = 0; i < received; i++) {
            furi_message_queue_put(app->queue, &buffer[i], 0);
        }
    }
}

void setup_vcp(VcpApp* app) {
    // Set RX callback
    furi_hal_vcp_set_rx_callback(vcp_rx_callback, app);
}
```

---

## HID (Human Interface Device)

HID mode allows Flipper to emulate keyboard, mouse, and consumer control devices.

### HID Configuration

```c
#include <furi_hal_usb_hid.h>

// Custom HID configuration (optional)
FuriHalUsbHidConfig hid_config = {
    .vid = 0x046D,  // Logitech VID (default)
    .pid = 0xC529,  // Logitech PID (default)
    .manuf = "Flipper Devices",
    .product = "Flipper Zero",
};

// Switch to HID mode with custom config
furi_hal_usb_set_config(&usb_hid, &hid_config);

// Or use defaults
furi_hal_usb_set_config(&usb_hid, NULL);
```

### Keyboard Emulation

#### Basic Key Press

```c
// Check HID connection
if(!furi_hal_hid_is_connected()) {
    FURI_LOG_W("HID", "Not connected");
    return;
}

// Press a key
furi_hal_hid_kb_press(HID_KEYBOARD_A);

// Wait for key to register
furi_delay_ms(20);

// Release key
furi_hal_hid_kb_release(HID_KEYBOARD_A);
```

#### Typing Text

```c
void hid_type_string(const char* text) {
    if(!furi_hal_hid_is_connected()) return;

    for(size_t i = 0; text[i] != '\0'; i++) {
        uint16_t keycode = HID_ASCII_TO_KEY(text[i]);

        if(keycode != HID_KEYBOARD_NONE) {
            // Press key (includes modifiers like SHIFT)
            furi_hal_hid_kb_press(keycode);
            furi_delay_ms(20);

            // Release all
            furi_hal_hid_kb_release_all();
            furi_delay_ms(20);
        }
    }
}

// Usage
hid_type_string("Hello, World!");
```

#### Key Combinations

```c
// Press CTRL+C (Copy)
furi_hal_hid_kb_press(HID_KEYBOARD_C | KEY_MOD_LEFT_CTRL);
furi_delay_ms(50);
furi_hal_hid_kb_release_all();

// Press CTRL+ALT+DEL
furi_hal_hid_kb_press(HID_KEYBOARD_DELETE | KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_ALT);
furi_delay_ms(50);
furi_hal_hid_kb_release_all();

// Press WIN+R (Run dialog on Windows)
furi_hal_hid_kb_press(HID_KEYBOARD_R | KEY_MOD_LEFT_GUI);
furi_delay_ms(50);
furi_hal_hid_kb_release_all();
```

#### Special Keys

```c
// Function keys
furi_hal_hid_kb_press(HID_KEYBOARD_F1);
furi_hal_hid_kb_press(HID_KEYBOARD_F12);

// Navigation
furi_hal_hid_kb_press(HID_KEYBOARD_HOME);
furi_hal_hid_kb_press(HID_KEYBOARD_END);
furi_hal_hid_kb_press(HID_KEYBOARD_PAGE_UP);
furi_hal_hid_kb_press(HID_KEYBOARD_PAGE_DOWN);

// Arrows
furi_hal_hid_kb_press(HID_KEYBOARD_UP_ARROW);
furi_hal_hid_kb_press(HID_KEYBOARD_DOWN_ARROW);
furi_hal_hid_kb_press(HID_KEYBOARD_LEFT_ARROW);
furi_hal_hid_kb_press(HID_KEYBOARD_RIGHT_ARROW);

// Other
furi_hal_hid_kb_press(HID_KEYBOARD_ESCAPE);
furi_hal_hid_kb_press(HID_KEYBOARD_RETURN);  // Enter
furi_hal_hid_kb_press(HID_KEYBOARD_TAB);
furi_hal_hid_kb_press(HID_KEYBOARD_DELETE);
furi_hal_hid_kb_press(HID_KEYBOARD_BACKSPACE);
```

#### Modifier Keys

```c
enum HidKeyboardMods {
    KEY_MOD_LEFT_CTRL   = (1 << 8),
    KEY_MOD_LEFT_SHIFT  = (1 << 9),
    KEY_MOD_LEFT_ALT    = (1 << 10),
    KEY_MOD_LEFT_GUI    = (1 << 11),   // Windows/Command key
    KEY_MOD_RIGHT_CTRL  = (1 << 12),
    KEY_MOD_RIGHT_SHIFT = (1 << 13),
    KEY_MOD_RIGHT_ALT   = (1 << 14),
    KEY_MOD_RIGHT_GUI   = (1 << 15),
};
```

### Keyboard LED State

```c
// Get keyboard LED state (Num Lock, Caps Lock, Scroll Lock)
uint8_t leds = furi_hal_hid_get_led_state();

if(leds & HID_KB_LED_CAPS) {
    FURI_LOG_I("HID", "Caps Lock ON");
}

if(leds & HID_KB_LED_NUM) {
    FURI_LOG_I("HID", "Num Lock ON");
}

if(leds & HID_KB_LED_SCROLL) {
    FURI_LOG_I("HID", "Scroll Lock ON");
}
```

### Mouse Emulation

```c
// Check connection
if(!furi_hal_hid_is_connected()) return;

// Move mouse (relative coordinates)
furi_hal_hid_mouse_move(10, -5);  // Move right 10, up 5

// Click left button
furi_hal_hid_mouse_press(HID_MOUSE_BTN_LEFT);
furi_delay_ms(50);
furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);

// Right click
furi_hal_hid_mouse_press(HID_MOUSE_BTN_RIGHT);
furi_delay_ms(50);
furi_hal_hid_mouse_release(HID_MOUSE_BTN_RIGHT);

// Middle click (wheel button)
furi_hal_hid_mouse_press(HID_MOUSE_BTN_WHEEL);
furi_delay_ms(50);
furi_hal_hid_mouse_release(HID_MOUSE_BTN_WHEEL);

// Scroll wheel
furi_hal_hid_mouse_scroll(1);   // Scroll down
furi_hal_hid_mouse_scroll(-1);  // Scroll up
```

#### Mouse Buttons

```c
enum HidMouseButtons {
    HID_MOUSE_BTN_LEFT  = (1 << 0),
    HID_MOUSE_BTN_RIGHT = (1 << 1),
    HID_MOUSE_BTN_WHEEL = (1 << 2),
};
```

### Consumer Control

Consumer control keys for media playback and system control:

```c
// Volume
furi_hal_hid_consumer_key_press(HID_CONSUMER_VOLUME_INCREMENT);
furi_delay_ms(50);
furi_hal_hid_consumer_key_release(HID_CONSUMER_VOLUME_INCREMENT);

furi_hal_hid_consumer_key_press(HID_CONSUMER_VOLUME_DECREMENT);
furi_hal_hid_consumer_key_press(HID_CONSUMER_MUTE);

// Media control
furi_hal_hid_consumer_key_press(HID_CONSUMER_PLAY_PAUSE);
furi_hal_hid_consumer_key_press(HID_CONSUMER_SCAN_NEXT_TRACK);
furi_hal_hid_consumer_key_press(HID_CONSUMER_SCAN_PREVIOUS_TRACK);
furi_hal_hid_consumer_key_press(HID_CONSUMER_STOP);

// Brightness (OS-specific)
furi_hal_hid_consumer_key_press(HID_CONSUMER_BRIGHTNESS_INCREMENT);
furi_hal_hid_consumer_key_press(HID_CONSUMER_BRIGHTNESS_DECREMENT);

// Release all consumer keys
furi_hal_hid_consumer_key_release_all();
```

### HID Connection Callback

```c
void hid_state_callback(bool connected, void* context) {
    MyApp* app = context;

    if(connected) {
        FURI_LOG_I("HID", "Device connected");
        app->hid_connected = true;
    } else {
        FURI_LOG_I("HID", "Device disconnected");
        app->hid_connected = false;
    }
}

// Set callback
furi_hal_hid_set_state_callback(hid_state_callback, app_context);
```

---

## HID + U2F Mode

Combines HID functionality with U2F (Universal 2nd Factor) authentication:

```c
// Switch to HID + U2F mode
furi_hal_usb_set_config(&usb_hid_u2f, NULL);

// HID functions work normally
furi_hal_hid_kb_press(HID_KEYBOARD_A);

// U2F is handled by dedicated U2F app/service
```

---

## CCID (Smart Card)

Smart card emulation mode for cryptographic operations:

```c
#include <furi_hal_usb_ccid.h>

// Switch to CCID mode
furi_hal_usb_set_config(&usb_ccid, NULL);

// CCID functionality is typically used by dedicated apps
// (e.g., OpenPGP, PIV/PKCS#11 emulation)
```

---

## USB State Management

### USB State Events

```c
typedef enum {
    FuriHalUsbStateEventReset,              // USB reset
    FuriHalUsbStateEventWakeup,             // USB wakeup from suspend
    FuriHalUsbStateEventSuspend,            // USB suspend
    FuriHalUsbStateEventDescriptorRequest,  // Descriptor requested
} FuriHalUsbStateEvent;
```

### USB State Callback

```c
void usb_state_callback(FuriHalUsbStateEvent event, void* context) {
    MyApp* app = context;

    switch(event) {
        case FuriHalUsbStateEventReset:
            FURI_LOG_I("USB", "Reset");
            app->usb_configured = false;
            break;

        case FuriHalUsbStateEventWakeup:
            FURI_LOG_I("USB", "Wakeup");
            break;

        case FuriHalUsbStateEventSuspend:
            FURI_LOG_I("USB", "Suspend");
            break;

        case FuriHalUsbStateEventDescriptorRequest:
            FURI_LOG_I("USB", "Descriptor requested");
            app->usb_configured = true;
            break;
    }
}

// Set callback
furi_hal_usb_set_state_callback(usb_state_callback, app_context);
```

### USB Control

```c
// Disable USB
furi_hal_usb_disable();

// Enable USB
furi_hal_usb_enable();

// Reinitialize USB (soft reset)
furi_hal_usb_reinit();
```

---

## Complete HID Keyboard Example

```c
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>

int32_t hid_demo_app(void* p) {
    UNUSED(p);

    // Check if USB is locked
    if(furi_hal_usb_is_locked()) {
        FURI_LOG_E("HID", "USB is locked");
        return -1;
    }

    // Switch to HID mode
    furi_hal_usb_set_config(&usb_hid, NULL);

    // Wait for enumeration
    furi_delay_ms(200);

    // Wait for connection
    for(int i = 0; i < 50 && !furi_hal_hid_is_connected(); i++) {
        furi_delay_ms(100);
    }

    if(!furi_hal_hid_is_connected()) {
        FURI_LOG_W("HID", "Not connected, but continuing...");
    }

    // Type "Hello, World!"
    const char* message = "Hello, World!";
    for(size_t i = 0; message[i] != '\0'; i++) {
        uint16_t key = HID_ASCII_TO_KEY(message[i]);
        if(key != HID_KEYBOARD_NONE) {
            furi_hal_hid_kb_press(key);
            furi_delay_ms(50);
            furi_hal_hid_kb_release_all();
            furi_delay_ms(50);
        }
    }

    // Press Enter
    furi_hal_hid_kb_press(HID_KEYBOARD_RETURN);
    furi_delay_ms(50);
    furi_hal_hid_kb_release_all();

    FURI_LOG_I("HID", "Demo complete");

    return 0;
}
```

---

## Complete HID Mouse Example

```c
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>

int32_t hid_mouse_demo(void* p) {
    UNUSED(p);

    // Setup HID
    furi_hal_usb_set_config(&usb_hid, NULL);
    furi_delay_ms(200);

    // Wait for connection
    for(int i = 0; i < 50 && !furi_hal_hid_is_connected(); i++) {
        furi_delay_ms(100);
    }

    if(!furi_hal_hid_is_connected()) {
        FURI_LOG_E("HID", "Not connected");
        return -1;
    }

    // Draw a square with mouse
    const int side = 100;
    const int step = 5;

    // Right
    for(int i = 0; i < side; i += step) {
        furi_hal_hid_mouse_move(step, 0);
        furi_delay_ms(10);
    }

    // Down
    for(int i = 0; i < side; i += step) {
        furi_hal_hid_mouse_move(0, step);
        furi_delay_ms(10);
    }

    // Left
    for(int i = 0; i < side; i += step) {
        furi_hal_hid_mouse_move(-step, 0);
        furi_delay_ms(10);
    }

    // Up
    for(int i = 0; i < side; i += step) {
        furi_hal_hid_mouse_move(0, -step);
        furi_delay_ms(10);
    }

    // Click in center
    furi_hal_hid_mouse_press(HID_MOUSE_BTN_LEFT);
    furi_delay_ms(50);
    furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);

    FURI_LOG_I("HID", "Mouse demo complete");
    return 0;
}
```

---

## BadUSB Integration

Flipper's BadUSB app uses HID mode for USB Rubber Ducky-style attacks:

```
REM BadUSB Script Example
DELAY 1000
GUI r
DELAY 500
STRING notepad
ENTER
DELAY 1000
STRING Hello from Flipper Zero!
ENTER
```

See: `documentation/file_formats/BadUsbScriptFormat.md` for script format

---

## USB Power Detection

```c
// Get USB voltage
float usb_voltage = furi_hal_power_get_usb_voltage();

if(usb_voltage > 4.5f) {
    FURI_LOG_I("USB", "USB connected: %.2fV", usb_voltage);
} else {
    FURI_LOG_I("USB", "USB not connected");
}

// Check charging status
bool is_charging = furi_hal_power_is_charging();
```

---

## Troubleshooting

### Common Issues

**Device not recognized**:
- Wait 200-500ms after `furi_hal_usb_set_config()`
- Check USB cable (data lines must be connected)
- Try different USB port
- Check if mode is locked by another app

**HID keys not working**:
- Verify connection: `furi_hal_hid_is_connected()`
- Add delays between key press/release (20-50ms)
- Always call `furi_hal_hid_kb_release_all()` after press
- Check for OS-specific key code differences

**VCP data loss**:
- Check buffer size (VCP buffers are limited)
- Use flow control if available
- Increase polling rate for RX callback
- Add delays between transmissions

**Mode switch fails**:
- Check if USB is locked: `furi_hal_usb_is_locked()`
- Ensure no other app is using USB
- Try `furi_hal_usb_reinit()` before switch

---

## Best Practices

### USB Mode Management

1. **Check lock state before switching**:
   ```c
   if(furi_hal_usb_is_locked()) {
       // Handle locked state
   }
   ```

2. **Lock mode while in use**:
   ```c
   furi_hal_usb_lock();
   // Use USB
   furi_hal_usb_unlock();
   ```

3. **Wait after mode switch**:
   ```c
   furi_hal_usb_set_config(&usb_hid, NULL);
   furi_delay_ms(200);  // Allow enumeration
   ```

### HID Operations

1. **Check connection before sending**:
   ```c
   if(furi_hal_hid_is_connected()) {
       // Send HID reports
   }
   ```

2. **Release keys properly**:
   ```c
   furi_hal_hid_kb_press(key);
   furi_delay_ms(50);
   furi_hal_hid_kb_release_all();  // Always release
   ```

3. **Add delays between operations**:
   ```c
   furi_hal_hid_kb_press(key1);
   furi_delay_ms(20);
   furi_hal_hid_kb_release_all();
   furi_delay_ms(20);  // Before next operation
   ```

---

## USB Specifications

**USB Version**: 2.0 Full Speed
**Transfer Rate**: 12 Mbps
**Power**: 5V, max 500mA (USB 2.0 standard)
**Endpoints**: Configurable per mode
**Descriptors**: Custom per interface

---

## Related Documentation

- [FuriHalAPI.md](../FuriHalAPI.md) - Complete HAL API
- [BadUsbScriptFormat.md](../file_formats/BadUsbScriptFormat.md) - BadUSB scripts
- [furi_hal_usb.h](../../targets/furi_hal_include/furi_hal_usb.h) - USB API
- [furi_hal_usb_hid.h](../../targets/furi_hal_include/furi_hal_usb_hid.h) - HID API

---

**Oracle Documentation** - USB mastery complete.
