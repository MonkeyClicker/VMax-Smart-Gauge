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

The 800x480 single-engine screen includes RPM, speed, trim, engine temperature,
oil pressure, battery voltage, fuel flow, engine hours, and engine status. The
simulator follows an approximately 45-second idle-to-cruise-to-idle cycle.

Display stack:

- Waveshare ESP32-S3-Touch-LCD-5, 800x480 variant
- Espressif `ESP32_Display_Panel` 1.0.0
- LVGL 8.3.11
- 16 MB flash and 8 MB OPI PSRAM configuration

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
