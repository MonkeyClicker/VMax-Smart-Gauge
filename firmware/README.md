# Firmware

This PlatformIO project has two independent build targets. The original NMEA
scanner remains in `src/main.cpp` unchanged, while the bench-test gauge is
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
The original scanner is not overwritten or modified.

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
