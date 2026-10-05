# Waveshare display setup and troubleshooting

Verified on 2026-10-05 for VMax Smart Gauge's Waveshare ESP32-S3-Touch-LCD-5,
800 x 480 panel, ESP32-S3-WROOM-1-N16R8 (16 MB flash, 8 MB OPI PSRAM).
This guide records the working bench configuration and separates observations
from hypotheses. The simulated gauge was subsequently built, uploaded, and confirmed visibly working by the user.

## Arduino IDE settings

Use the standard **ESP32S3 Dev Module**, rather than the **Octal (WROOM2)** variant
selected during the unsuccessful earlier session.

| Setting | Baseline value | Evidence |
| --- | --- | --- |
| Board | ESP32S3 Dev Module | Visible in successful diagnostic screenshots |
| PSRAM | OPI PSRAM | Changing this was followed by 8,388,608 bytes reported |
| USB CDC On Boot | Enabled | Required baseline for native USB `Serial` output |
| USB Mode | Hardware CDC and JTAG | Baseline carried from troubleshooting |
| Upload Mode | UART0 / Hardware CDC | Baseline carried from troubleshooting |
| Flash Size | 16 MB | Diagnostic reported 16,777,216 bytes |
| Flash Mode / Frequency | QIO / 80 MHz | Waveshare recommendation; full final Tools menu not captured |
| Partition Scheme | 16M Flash (3MB APP), or suitable 16 MB layout | Waveshare recommendation; final selection not recorded |
| Serial Monitor | 115200 baud | Visible in diagnostic screenshots |

The successful tests establish the runtime results below. Not every menu setting
was independently captured in a final screenshot. Record Arduino-ESP32 core and
installed display/expander library versions on the next reproduction; those exact
Arduino versions were not captured during this session.

## USB upload and serial recovery

1. Use a USB data cable. A power-only cable previously prevented enumeration.
2. Upload the sketch with USB CDC enabled, then close Serial Monitor.
3. If the program does not start, press RESET once without holding BOOT.
4. Wait for enumeration, reselect the current port, then reopen Serial Monitor.
5. Expect repeated diagnostic lines, not just a startup banner.

COM6 worked on this computer, but port numbers are not fixed. A reset can drop and
re-enumerate native USB; restart PlatformIO's monitor if it does not reconnect.
Changing the selected Arduino board can change its remembered Tools settings;
check USB CDC and PSRAM again after changing boards.

If the USB port disappears, Waveshare documents holding BOOT while connecting USB,
then releasing BOOT to enter download mode. Press RESET after programming to run
the application. Do not leave BOOT held during a normal application reset.

## Verified diagnostic sequence

### 1. Boot-only test

Run without LCD, LVGL, touch, or I2C initialization. Start Serial at 115200, wait
at most five seconds for a host, and print uptime, reset reason, flash size,
PSRAM size, and free PSRAM once per second.

Observed progression:

- Serial output recovered and uptime continued beyond 23 seconds.
- Flash size was `16777216`, but initially `PSRAM=0`.
- After selecting OPI PSRAM and uploading again, `PSRAM=8388608` and
  free PSRAM was approximately `8386096`; uptime continued beyond two minutes.

A stable boot-only test with zero PSRAM is insufficient for the RGB display test.
The captured reset reason value was 11; this is a previous-reset diagnostic field,
not evidence of recurring resets. Increasing uptime demonstrated continuous operation.

### 2. Color-bar test

The downloaded `08_DrawColorBar` sketch defines expander pins but never initializes
CH422G or I2C. Its `waveshare_lcd_port.cpp` initializes the RGB LCD directly and
asserts `lcd->begin()`. Defining pin constants alone does not enable the backlight
or pulse the panel reset.

The successful diagnostic added this sequence:

| Signal | Connection / expander pin | Action |
| --- | --- | --- |
| I2C SDA | ESP32 GPIO8 | CH422G bus |
| I2C SCL | ESP32 GPIO9 | CH422G bus |
| TP_RST | CH422G EXIO1 | HIGH, release touch reset |
| LCD_BL | CH422G EXIO2 | LOW during initialization, HIGH afterward |
| LCD_RST | CH422G EXIO3 | LOW for 10 ms, HIGH, then wait 100 ms |
| SD_CS | CH422G EXIO4 | HIGH, deselect SD |
| USB_SEL | CH422G EXIO5 | LOW, retain USB routing |

Create `esp_expander::CH422G(9, 8, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS)`,
check `init()` and `begin()`, then call `enableAllIO_Output()`. These EXIO numbers
are expander pins, not ESP32 GPIO numbers. Use the library's CH422G address constant;
the chip occupies multiple I2C addresses, so do not assume it is a conventional
single-address expander.

After the expander setup and LCD reset pulse, initialize the RGB LCD, draw color
bars, and enable the backlight. The test used the demo's 800 x 480 configuration,
16-bit RGB, 16 MHz pixel clock, and ten-row bounce buffer. It disabled FPS and
draw-finish callback printing, added stage messages, and replaced the
`lcd->begin()` assertion with a repeating failure message.

**User verified visible color bars and continuing alive messages beyond 140 seconds.**
This validates the LCD/backlight path with the tested configuration. Touch input,
CAN/NMEA, SD logging, LVGL, and the complete gauge were not validated by this test.

## PlatformIO settings to reconcile

At documentation time, `firmware/platformio.ini` specifies:

- `esp32-s3-devkitc-1`, Arduino framework, and 115200 monitor speed.
- 16 MB flash, `board_build.arduino.memory_type = qio_opi`, and `BOARD_HAS_PSRAM`.
- **`ARDUINO_USB_CDC_ON_BOOT=0`**, which differs from the working native USB baseline.
- Arduino-ESP32 3.1.1; ESP32_Display_Panel v1.0.0;
  ESP32_IO_Expander v1.1.0; esp-lib-utils v0.2.0; LVGL v8.4.0.

For native USB diagnostics, reconcile the CDC flag with the working configuration
(`ARDUINO_USB_CDC_ON_BOOT=1`, hardware CDC/JTAG uses `ARDUINO_USB_MODE=1`).
The simulated-gauge target now enables CDC and hardware CDC/JTAG while preserving the existing shared configuration.

`firmware/include/esp_panel_board_supported_conf.h` enables
`ESP_PANEL_BOARD_DEFAULT_USE_SUPPORTED` and `BOARD_WAVESHARE_ESP32_S3_TOUCH_LCD_5`.
The gauge uses the display library's Board abstraction. Check its board-specific
expander/reset/backlight setup before adding a second manual initialization path.

A previous `No default board configuration detected` error was resolved by a
clean rebuild and upload, according to the handoff. The header had already matched
the library copy; stale artifacts were implicated. Clean and rebuild after board
configuration changes before diagnosing a runtime result from an older binary.

## Findings and limits

- The old eight-second disconnect cycle alone did not establish a watchdog reset.
- A CH422G handshake hang cannot explain the original color-bar sketch: it made
  no CH422G/I2C initialization calls.
- Missing expander setup was present in that sketch; the modified test works.
  Multiple settings changed across uploads, so no single cause for all earlier
  symptoms was isolated.
- Wall-charger substitution did not resolve the earlier symptoms, but does not
  conclusively rule out all power faults. No capacitor modification was needed
  for the successful test.
- An earlier library/API compilation failure was reported in the handoff.
  Do not install the latest display library blindly: use a matched example and
  dependency set, and distinguish compile failures from runtime failures.

## Reference documentation

- [Waveshare board configuration table](https://github.com/waveshareteam/ESP32-S3-Touch-LCD-5/blob/master/examples/Arduino/libraries/ESP32_Display_Panel/docs/board/board_waveshare.md)
- [Waveshare user guide: expander and USB recovery](https://docs.waveshare.com/ESP32-S3-Touch-LCD-5/Instructions-For-Use)
- [Espressif Arduino USB CDC flashing](https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/cdc_dfu_flash.html)

These links support baseline setup guidance; the runtime observations above come
from user-provided serial screenshots and the user's visible color-bar confirmation.
