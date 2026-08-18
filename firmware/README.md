# Firmware

## Milestone 1 Status

The first firmware is intentionally a simple receive-only NMEA 2000 scanner.

It uses the ESP32-S3 TWAI peripheral directly rather than adding the NMEA2000 library immediately. This lets us verify the physical CAN connection, source addresses, PGNs, and update rates with the smallest possible software stack.

## Current Decoding

The scanner currently decodes single-frame PGNs:

- 127488 - Engine Parameters, Rapid Update
  - engine instance
  - RPM
  - trim
- 127505 - Fluid Level
  - fuel tank instance
  - level
  - capacity
- 129026 - COG & SOG, Rapid Update
  - GPS speed
  - course

It also counts every observed PGN/source pair whether or not the PGN is decoded.

## Not Yet Decoded

PGN 127489 (Engine Parameters, Dynamic) is larger than one CAN frame and requires NMEA 2000 fast-packet reassembly. That is the next parser step after basic CAN reception is proven.

Expected future fields include:
- coolant temperature
- alternator voltage
- fuel rate
- engine hours
- engine load/torque where available
- engine status/warning bits

## Toolchain

Initial project:
- PlatformIO
- Arduino framework
- PlatformIO Espressif32 7.0.1
- generic `esp32-s3-devkitc-1` profile with Waveshare flash/PSRAM overrides

The Waveshare board is an ESP32-S3-WROOM-1-N16R8 (16 MB flash, 8 MB OPI PSRAM). If the initial PlatformIO profile does not correctly initialize the exact Waveshare memory configuration, replace it with a custom board JSON based on the Waveshare hardware before UI development.

## CAN Configuration

Waveshare onboard CAN:
- GPIO15 = TX
- GPIO16 = RX
- 250 kbit/s
- ESP32 TWAI listen-only mode

The Waveshare CAN termination switch must remain OFF when the unit is attached as a normal drop device to the existing NMEA 2000 backbone.

## Build

From the `firmware` directory:

```bash
pio run
```

Upload over USB:

```bash
pio run -t upload
```

Serial monitor:

```bash
pio device monitor
```

## First Boat Test

1. Leave the Waveshare CAN termination resistor disabled.
2. Power the Waveshare from USB.
3. Connect CAN-H, CAN-L, and network reference/ground to the NMEA drop cable.
4. Start the serial monitor at 115200 baud.
5. Power the boat NMEA 2000 network.
6. Verify PGNs begin appearing in the statistics output.
7. Turn the Yamaha ignition/engine on.
8. Confirm PGN 127488 appears and RPM agrees with the Garmin/Yamaha display.
9. Change trim and confirm the decoded trim value responds.
10. Save the serial output as the first real network capture.

## Safety

This is development instrumentation. Do not treat the custom display as a replacement for required Yamaha warning, control, or safety instrumentation.
