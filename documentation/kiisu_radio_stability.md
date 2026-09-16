# Kiisu radio stability validation

## Changes

- BLE RPC checks the GATT send result and limits confirmation waits to 10 seconds.
  A failed transmission invalidates the RPC stream and requests recovery in the
  Bluetooth service thread. Advertising resumes only if Bluetooth is enabled.
  A queued recovery is ignored if the profile has changed or a new RPC session
  has already cleared the disconnected flag.
- BLE starts with a 20-byte payload and resets that limit after disconnection.
  Negotiated payload sizes are capped at the serial service limit.
- NFC consumes notification flags only once. A second clear after a wait could
  erase an interrupt, timeout or cancellation delivered after the wait returned.
  Error return values are no longer interpreted as hardware event bitmasks.
- NFC initialization stops if acquisition fails, avoiding SPI access without
  ownership.
- RFID gives cancellation priority when start and stop arrive together, and
  active operations also honor a request to stop the worker thread.

These changes address identifiable software failure paths. They do not establish
the cause of every reported reset or guarantee recovery of a partially written tag.

## Device checks still required

### BLE connection and identity revision

- Advertising timer commands are ignored once connected or explicitly stopped.
  Low-power advertising does not restart itself every minute. A rejected start
  no longer reports advertising success.
- Failed/stale connection events do not overwrite the current link parameters.
  Negotiation accepts the whole configured interval range, is limited to two
  requests per connection, and decodes the response using the correct event ID.
- Scan-response data is cleared between profiles using the full 31-byte buffer
  required by the HCI wrapper (which copies 31 bytes even for an empty payload).
- Loading a changed device name or MAC from the SD card recreates the default
  serial profile. Unchanged settings do not require a restart, and custom profiles
  retain their application-controlled identity.
- This build uses the requested `Lykoi` fallback name, so it no longer changes
  randomly to names such as `Asian` on reboot/remount. Saved names take precedence.
  Its memory is now owned consistently, avoiding an
  invalid free on a subsequent remount. Short custom names are copied without
  reading beyond the end of their allocation.

For this revision, connect just before the 60-second advertising timeout and
remain connected for several minutes. Repeat with Bluetooth disabled just before
the timeout. Boot with a delayed SD mount and a custom name, then inspect both
the advertised name and GAP Device Name. Remount with unchanged settings and
verify the active link survives. Repeat with a changed name and verify the
intentional profile restart announces the new identity. Test one-, two- and
three-character names and repeated boots without a custom-name file.

Record the board revision, firmware commit, phone/OS, tag type, battery level and
exact operation for each failure. Capture the crash text or serial log if possible.

1. Connect/disconnect BLE at least 20 times, including reconnecting with another
   phone. Transfer files and exercise RPC with and without MTU negotiation.
2. Disconnect the phone during a transfer. Verify the device remains responsive
   and can reconnect. Inject a failed GATT update or missing confirmation in a
   debug build: recovery must occur without an indefinite wait or a reboot.
3. Enter and immediately leave RFID read/emulate/write screens repeatedly.
   Repeat NFC read/emulate/cancel cycles, including removing the tag mid-read.
4. Repeat the radio operations both with BLE connected and disconnected.
5. Use only expendable, compatible tags for write validation. Save an original
   dump, write known data, remove and re-present the tag, then read it independently
   and compare the relevant data. Do not treat the write screen alone as proof.
6. If a reset persists, capture its log before changing timings, antenna settings,
   or write parameters. Do not disable assertions to hide a crash.

## Build status

The earlier custom BLE compilation errors have been corrected. Firmware and
updater linking pass; the updated package is being assembled. A successful build
does not replace testing connection retention and writes on the physical device.
