# V MAX Smart Gauge

ESP32-S3 based NMEA 2000 multifunction display inspired by the Yamaha 6YC gauge.

## Target Boat Setup

- 2024 Yamaha V MAX SHO 250
- Garmin 106sv
- Existing powered NMEA 2000 backbone
- Spare NMEA 2000 drop connection available

## Planned Hardware

- Waveshare ESP32-S3-Touch-LCD-5
- 5 inch 800x480 capacitive touchscreen
- ESP32-S3-WROOM-1-N16R8
- Onboard TJA1051 CAN transceiver
- microSD logging
- LiFePO4 battery USB power
- NMEA 2000 Micro-C drop connection

## Planned Features

- Engine RPM
- Trim
- Engine temperature
- Alternator voltage
- Fuel flow
- Engine hours
- Engine alarms/status
- GPS speed and course
- Fuel level
- Calculated MPG / fuel economy
- Trip computer
- Performance logging
- Cruise optimizer
- NMEA diagnostics
- OTA firmware updates
- Water-resistant marine enclosure

## Initial NMEA 2000 PGNs

- 127488 - Engine Parameters, Rapid Update
- 127489 - Engine Parameters, Dynamic
- 127505 - Fluid Level
- 129025 - Position, Rapid Update
- 129026 - COG & SOG, Rapid Update
- 129029 - GNSS Position Data

## Project Phases

1. Build a receive-first NMEA 2000 scanner.
2. Capture and identify the Yamaha and Garmin data actually present on the boat.
3. Build a diagnostics screen.
4. Build the Yamaha-style main engine display.
5. Add performance, trip, and logging screens.
6. Add OTA firmware updates.
7. Design and test the water-resistant enclosure.

## Repository Layout

- `docs/` - design and engineering documentation
- `firmware/` - ESP32 firmware
- `hardware/` - wiring, schematics, and component notes
- `nmea/` - captures and PGN notes
- `ui/` - UI mockups and assets
- `enclosure/` - CAD and enclosure design

## Status

Project initialized. Hardware selection and system architecture are defined; firmware implementation is next.
