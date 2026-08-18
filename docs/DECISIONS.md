# Design Decisions

## 2026-08-18 - Project Goal

Build a custom ESP32-S3 NMEA 2000 multifunction gauge inspired by the Yamaha 6YC for a 2024 Yamaha V MAX SHO 250.

The unit will initially be receive-first and coexist with the existing Garmin 106sv and Yamaha engine connection on the boat's NMEA 2000 backbone.

## 2026-08-18 - Main Hardware

Selected Waveshare ESP32-S3-Touch-LCD-5, 800x480 capacitive-touch version.

Reasons:

- ESP32-S3
- 5 inch 800x480 display
- capacitive touch
- onboard TJA1051 CAN transceiver
- microSD support
- Wi-Fi for OTA
- sufficient flash and PSRAM for LVGL UI

## 2026-08-18 - NMEA Connection

Use the existing available NMEA 2000 drop connection.

The gauge is a drop device and will not add another 120-ohm terminator. The onboard CAN termination option should remain disabled.

Initial CAN connections:

- NMEA NET-H -> CAN-H
- NMEA NET-L -> CAN-L
- NMEA NET-C -> CAN reference/ground
- NMEA NET-S is not used to power the prototype

## 2026-08-18 - Power

Power the Waveshare from the regulated USB output of the LiFePO4 battery that also powers the Garmin.

Do not power the prototype from NMEA NET-S.

## 2026-08-18 - Firmware Strategy

Start with a receive-first NMEA scanner before building the production UI.

First identify the actual PGNs, source addresses, engine instance, update rates, and fields transmitted by the Yamaha and Garmin.

## 2026-08-18 - OTA

OTA firmware updates are a core requirement.

Planned behavior:

- OTA normally disabled
- enable service/update mode from touchscreen
- temporary Wi-Fi access point if needed
- firmware update from phone or laptop
- retain an internal USB connection for recovery
- use OTA partitioning and rollback protection

## 2026-08-18 - Enclosure

Target a highly water-resistant marine enclosure; do not claim a formal IP rating without testing.

Current direction:

- ASA 3D-printed housing
- exposed touchscreen glass sealed around its perimeter
- removable rear cover
- recessed silicone rear gasket
- stainless fasteners with uniform gasket compression
- sealed cable entry/connectors
- hydrophobic/ePTFE pressure equalization vent
- conformal coating as a secondary PCB protection layer

## 2026-08-18 - Planned UI

Initial screens:

1. Main Engine
2. Performance
3. Trip
4. NMEA Diagnostics
5. Settings/System

The interface will be inspired by the Yamaha 6YC but can add functionality such as cruise optimization and detailed logging.
