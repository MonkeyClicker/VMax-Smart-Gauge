# Firmware

This PlatformIO project has two independent build targets. The NMEA scanner is in `src/main.cpp`, with portable decoding and statistics
in `include/Scanner.h`. The bench-test gauge is
isolated under `src/simulated_gauge/`.

## Choose a target

The simulated gauge is the default and requires no boat connection:

```bash
pio run -e simulated-gauge
pio run -e simulated-gauge -t upload
pio device monitor -b 115200
```

Build and upload the original receive-only NMEA scanner with:

```bash
pio run -e nmea-scanner
pio run -e nmea-scanner -t upload
pio device monitor -b 115200
```

| Target | Entry point | Purpose |
|---|---|---|
| `nmea-scanner` | `src/main.cpp` | Original receive-only TWAI/NMEA scanner |
| `simulated-gauge` | `src/simulated_gauge/main.cpp` | LCD UI using generated engine data |

PlatformIO's `build_src_filter` compiles exactly one `setup()`/`loop()` pair.
The targets remain independent; simulated values are never fed into the scanner.

## Simulated screen

The 800x480 single-engine demo uses five touch-selectable pages with a persistent
bottom navigation bar and status banner. The simulator follows an approximately
45-second idle-to-cruise-to-idle cycle.

| Page | Content and controls |
|---|---|
| Engine | Large tachometer; speed, trim, flow, temperature, oil, course, tank level, voltage, hours, economy |
| Performance | Current RPM/speed/flow/economy, best cruise RPM and trim, observed best economy by RPM band |
| Trip | Elapsed time, integrated distance/fuel, average economy, maximum speed/RPM, two-tap trip reset |
| Diagnostics | Honest demo input/service states, data age, uptime, memory, touch events, visual warning acknowledgement |
| Settings | US/marine/metric units, day/night palette, normal/warning/offline/unavailable demo scenarios |

Tap the status banner to open Diagnostics. Acknowledging a demo warning leaves it
visible; no buzzer is driven. Offline mode marks live values `STALE` and pauses
distance/fuel integration. Unavailable mode hides tank data with `--` while the
remaining inputs continue. Trip reset requires a second tap within five seconds;
it preserves the simulated tank level and lifetime engine hours.

All values are generated. The example tank is 60 US gallons, starting at 68%; this
is not the boat's configured capacity. Cruise observations begin above 2000 RPM.
Settings and trip totals are not persisted. Night mode changes the palette only,
not physical backlight brightness. CAN/NMEA, SD, Wi-Fi/OTA, and buzzer services
are inactive in this target.

Display stack:

- Waveshare ESP32-S3-Touch-LCD-5, 800x480 variant
- Espressif `ESP32_Display_Panel` 1.0.0
- LVGL 8.4.0
- 16 MB flash and 8 MB OPI PSRAM configuration

See [Waveshare display setup and troubleshooting](../docs/Waveshare-Setup.md)
for the verified bench configuration, USB recovery steps, and display diagnostics.

The simulated target enables native USB CDC with hardware CDC/JTAG
(`ARDUINO_USB_CDC_ON_BOOT=1`, `ARDUINO_USB_MODE=1`). Mode 1 is confirmed by
the installed Arduino-ESP32 core. Startup
messages report flash/PSRAM sizes and each display initialization stage, followed
by a `Gauge alive` message every five seconds. If output does not resume after
upload, close the monitor, press RESET without BOOT, reselect the USB port, and
reopen the monitor at 115200 baud. Confirm the gauge values change through a
45-second cycle; a successful build alone does not verify the display.

The target also selects the Waveshare board explicitly in its build flags so
the display library receives the same board selection as the application.
Relying on discovery of the project configuration header alone can produce
`Display board init failed` even after a clean build.

Bench verification on 2026-10-05: the firmware built and uploaded successfully,
the user confirmed the visible gauge and changing values, and serial heartbeats
continued beyond 55 seconds with stable memory readings. CAN/NMEA and SD logging
were not validated by this simulated-gauge test.

### Multi-page bench checks

Automated verification on 2026-10-05: build/upload and simulator regression checks
passed on the Waveshare. All five pages passed label text-fit/bounds checks, as did
the checked metric/marine layouts and warning/offline/unavailable/night states.
Repeated page changes returned to the same Engine-page LVGL allocation; free
device heap stayed at 118052 bytes and free PSRAM at 6829964 bytes during the run,
with continuous heartbeat uptime past 65 seconds. The user also confirmed that
the multi-page version looks good and touch navigation is responsive.

1. Tap all five navigation tabs and confirm responsive navigation and legible values.
2. Select each unit preset; speed, distance, fuel, temperature, oil pressure, and
   economy should convert consistently across pages.
3. Select Warning, tap the banner, and acknowledge it in Diagnostics. The warning
   stays visible. Return to Normal to clear the demonstration condition.
4. Select Offline and confirm stale fields and frozen trip distance/fuel. Return
   to Normal and confirm accumulation resumes without inventing totals for the gap.
5. Select Unavailable and confirm only tank level becomes unavailable.
6. Try a trip reset once, wait five seconds, and confirm no reset occurred. Then
   tap reset twice within five seconds and confirm totals reset without refilling
   the tank or resetting engine hours.
7. Repeatedly navigate and change units/theme while monitoring memory and uptime.

Optional serial commands at 115200: `1`-`5` select pages; `N/W/O/U` choose demo
scenarios; `u/k/m` choose unit presets; `d/n` select day/night; `a` acknowledges;
two `r` commands within five seconds reset the trip. `t` runs portable simulator
regression checks; `v` checks current label bounds/text fit and reports LVGL memory.
The same simulator checks can run on a host with a C++17 compiler:

```bash
c++ -std=c++17 -Wall -Wextra -Werror firmware/test/host/simulated_gauge_test.cpp firmware/src/simulated_gauge/SimulatedEngineData.cpp -o /tmp/vmax-simulated-test
/tmp/vmax-simulated-test
```

## First boat test safety

1. Build and flash `nmea-scanner`, not `simulated-gauge`.
2. Leave the Waveshare CAN termination resistor disabled.
3. Power the Waveshare from USB.
4. Connect CAN-H, CAN-L, and network reference/ground to the NMEA drop cable.
5. Open the serial monitor at 115200 baud.
6. Power the NMEA 2000 network and verify traffic before starting the engine.

This is development instrumentation, not a replacement for required Yamaha
warning, control, or safety instrumentation.

## Scanner diagnostics

The scanner emits a snapshot every five seconds rather than printing every CAN
frame. CAN reception and serial output run separately: a blocked serial host does
not block reception. If logging falls behind, the queue keeps the newest pending
snapshot; serial snapshots are not a lossless raw-frame capture.

Each measurement distinguishes unknown, unavailable (`NA`), stale (`STALE`), and
valid numeric values. `fresh()` requires an available value. The scanner uses a
five-second freshness timeout for its diagnostic display; future gauge fields
may need individual timeouts. Last valid values may remain stored for history,
but unavailable values are never reported as fresh.

The single-engine/tank/navigation snapshot follows the latest source (and
instance where applicable). Changing identity clears the previous measurements.
This prevents cross-device mixing; it is not a persistent source-selection UI.

PGN/source rows show CAN **frame** counts, rates over the preceding reporting
window, and last-seen age in milliseconds. Fast-packet fragments are counted
individually, not as reassembled NMEA messages. Rates become zero for offline
sources. The 64-row table evicts the least recently seen pair when full, reports
a cumulative eviction count, and restarts counts for the replacement pair.

The CAN line reports driver state, queued frames, missed frames, hardware
receive overruns, bus errors, and the receive error counter. Missed/overrun/bus
error counters are cumulative since driver initialization. Increasing losses mean
the capture is incomplete and require investigation; no automatic recovery is
implemented by this scanner.

## Regression tests

Run from the repository root with a C++17 compiler installed:

```bash
bash firmware/test/host/run.sh
```

These tests cover signed fuel decoding, unavailable values, source/instance
changes, truncated payloads, timestamp rollover, reporting windows, and statistics
table eviction. They do not replace ESP32 or boat testing. Before boat use, check
serial backpressure under load and confirm CAN loss counters stay acceptable.
