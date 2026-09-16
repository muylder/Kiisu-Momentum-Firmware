# Kiisu Momentum Firmware

Community firmware for **Kiisu v4b/v4br**, combining Momentum and Kiisu-MNTM with Kiisu-specific apps, radio stability changes, and a consistent **Lykoi** default device name.

This is an independent community fork, not an official Momentum or stock Kiisu release.

**Current version:** [`kiisu-compilation`](https://github.com/muylder/Kiisu-Momentum-Firmware/tree/kiisu-compilation) · [Report an issue](https://github.com/muylder/Kiisu-Momentum-Firmware/issues) · [Releases](https://github.com/muylder/Kiisu-Momentum-Firmware/releases)

## Highlights

- **Momentum foundation:** customization, applications, and Kiisu integration inherited from the upstream projects.
- **Kiisu tools:** [Kiisu Manager](applications/system/kiisu-manager/README.md) for companion MCU settings and [Kiisu Sensor Hub](applications/system/kiisu-sensor-hub/README.md) for compass, motion, temperature, humidity, and light readings. Hardware and companion firmware requirements are documented with each app.
- **Consistent device identity:** `Lykoi` is the fallback name; saved custom names take precedence. BLE identity is refreshed when the saved name or MAC changes.
- **BLE stability changes:** bounded RPC transmission waits, conservative initial packet sizes, and fixes for advertising timers and connection parameter negotiation.
- **NFC and LF RFID changes:** improved event handling, initialization failure handling, and cancellation behavior.
- **Additional apps:** bundled FAP resources across GPIO, games, infrared, media, NFC, Sub-GHz, tools, and USB, plus custom BLE service/profile code.
- **Inherited Kiisu-MNTM features:** Kiisu-themed assets, U2F, and rolling-code support. Availability depends on the hardware and protocol; support is not universal.

## Status and limitations

This branch contains development changes that still require physical-device validation. Firmware and updater linking were reported successful in the [radio stability notes](documentation/kiisu_radio_stability.md); this does not establish that every app, connection, or tag operation works on every board.

The BLE Hardware Remote app is experimental: it sends a Space key over BLE HID when GPIO A4 is connected to GND. Its current loop has no Back-button exit handler, so restarting the device may be necessary to leave the app.

**No GitHub release packages are currently published in this repository.** Build this branch using the instructions below. Packages from other forks do not necessarily contain these changes. There is no fixed release schedule.

## Build

Clone this branch with its submodules:

```bash
git clone --branch kiisu-compilation --recurse-submodules --jobs 8 https://github.com/muylder/Kiisu-Momentum-Firmware.git
cd Kiisu-Momentum-Firmware
```

Build a firmware update package on Linux or macOS:

```bash
./fbt updater_package
```

On Windows PowerShell:

```powershell
.\fbt.cmd updater_package
```

Generated files are placed in `dist/f7-C/`, including an update `.tgz` package. The filename suffix depends on the branch, commit, or configured build suffix. See the [build tool documentation](documentation/fbt.md) for prerequisites and additional targets.

To build and install directly over USB, connect the Kiisu and close qFlipper first:

```bash
./fbt flash_usb_full
```

On Windows, use `.\fbt.cmd flash_usb_full`.

To build and launch an individual app:

```bash
./fbt launch APPSRC=your_appid
```

## Install a built package

1. Back up your SD card files and settings before changing firmware.
2. Build the `.tgz` package from `kiisu-compilation` as described above.
3. Connect your Kiisu and open qFlipper.
4. Choose **Install from file** and select the generated update `.tgz` from `dist/f7-C/`.
5. After installation, check the device name, Bluetooth connection, and the apps you use.

If the installer reports incompatible hardware, verify the board revision and package target before proceeding. Include the exact error when reporting an issue.

## Testing and feedback

The [radio stability validation guide](documentation/kiisu_radio_stability.md) covers BLE reconnects, interrupted transfers, advertising timeouts, SD remounts, and NFC/RFID cancellation and writes.

When [reporting a problem](https://github.com/muylder/Kiisu-Momentum-Firmware/issues), include:

- Kiisu board revision and firmware commit or package filename.
- Steps to reproduce, expected behavior, and actual behavior.
- Phone/OS and Bluetooth state for connection problems.
- Tag type and operation for NFC/RFID problems.
- Crash text or serial logs, if available.

## Credits and license

This fork builds on the work of:

- [Momentum Firmware / Next-Flip](https://github.com/Next-Flip/Momentum-Firmware).
- [Kiisu-MNTM / HiennNek](https://github.com/HiennNek/kiisu-mntm).
- [Unleashed Firmware / DarkFlippers](https://github.com/DarkFlippers/unleashed-firmware) and the wider Flipper/Kiisu contributor community.
- The authors of the included applications, libraries, and assets.

See [LICENSE](LICENSE) for the repository's GNU GPL v3 license. Included dependencies and assets retain their respective license notices.
