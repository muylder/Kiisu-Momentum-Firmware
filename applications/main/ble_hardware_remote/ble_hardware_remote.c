#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <bt/bt_service/bt.h>
#include <ble_profile/extra_profiles/hid_profile.h>

#define TAG "BleHwRemote"

static void ble_hw_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "BLE Hardware Remote");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 25, "Connect pin A4 to GND");
    canvas_draw_str(canvas, 2, 35, "to send a SPACE press");
}

int32_t ble_hardware_remote_app(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Starting BLE Hardware Remote");

    // Setup GUI
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, ble_hw_draw_callback, NULL);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Setup GPIO (Pin A4 is used as an example button)
    furi_hal_gpio_init(&gpio_ext_pa4, GpioModeInput, GpioPullUp, GpioSpeedLow);

    // Request BT HID Profile using the new dynamic profile API
    Bt* bt = furi_record_open(RECORD_BT);

    BleProfileHidParams hid_cfg = {
        .device_name_prefix = NULL,
        .mac_xor = 0
    };

    FuriHalBleProfileBase* ble_hid_profile = bt_profile_start(bt, ble_profile_hid, &hid_cfg);
    furi_check(ble_hid_profile);
    furi_hal_bt_start_advertising();

    bool last_state = true; // pull-up means true is unpressed

    while(1) {
        bool current_state = furi_hal_gpio_read(&gpio_ext_pa4);

        if (current_state == false && last_state == true) {
            FURI_LOG_I(TAG, "Button pressed! Sending HID input...");

            // Send HID Spacebar press, then release (0x2c is spacebar HID usage page)
            ble_profile_hid_kb_press(ble_hid_profile, 0x2C);
            furi_delay_ms(20);
            ble_profile_hid_kb_release(ble_hid_profile, 0x2C);
        }

        last_state = current_state;
        furi_delay_ms(50); // debounce loop
    }

    // Cleanup
    bt_profile_restore_default(bt);
    furi_record_close(RECORD_BT);

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}
