# Project Plan

## Objective

Deliver a reliable single-engine ESP32-S3 NMEA 2000 smart gauge for the 2024 Yamaha V MAX SHO 250, integrated with the existing Garmin/NMEA 2000 installation.

## Current implementation

The [first live-data firmware, v0.1.0](Live-Firmware-v0.1.md), implements the
standard engine/GPS decoding and main display, session trip calculations,
CAN diagnostics and raw USB capture. The five-second versioned boot screen is
implemented. v0.1.1 adds persistent tank-capacity setup and network-derived
remaining US gallons. v0.1.2 adds a resettable starting amount, independent fuel
inventory, and automatic priority for fresh network tank levels. This does not mark the milestones fully accepted: physical CAN
errors, on-device live-gauge acceptance, verified alarm interpretation, SD logging,
persistent trips, dedicated refueling controls, and OTA remain outstanding.

## Milestone 1 - NMEA Discovery

Build and validate a receive-first scanner.

Deliverables:

- TWAI/CAN receive path
- PGN/source statistics
- decode PGN 127488
- decode GPS PGN 129026
- identify actual Yamaha and Garmin source addresses
- collect first boat capture

## Milestone 2 - Full Engine Decode

Add support for NMEA 2000 fast-packet handling and PGN 127489.

Deliverables:

- engine temperature
- voltage
- fuel flow
- engine hours
- engine warning/status bits
- field availability documentation

## Milestone 3 - Main Gauge UI

Implement the single-engine 800x480 LVGL display.

Deliverables:

- large RPM/tach presentation
- speed
- trim
- fuel flow
- temperature
- fuel level
- voltage
- hours
- economy
- stale/unavailable states
- five-second branded boot screen with running firmware version; see [feature request](Boot-Screen.md)

## Milestone 4 - Diagnostics

Deliver an engineering/diagnostic screen with:

- PGNs
- source addresses
- rates/counts
- engine instance
- CAN status
- firmware version
- SD state
- Wi-Fi/OTA state

## Milestone 5 - Logging and Performance

Add microSD logging and trip calculations.

Deliverables:

- CSV logging
- trip fuel
- trip distance
- average MPG
- max speed/RPM
- cruise optimization data

## Milestone 6 - OTA and Settings

Deliver:

- persistent settings
- Wi-Fi service mode
- OTA upload
- dual-partition strategy
- rollback/recovery
- day/night brightness settings

## Milestone 7 - Marine Enclosure

Design and validate:

- ASA housing
- sealed touchscreen perimeter
- gasketed rear cover
- sealed cable entry
- pressure equalization vent
- conformal coating plan

## Milestone 8 - Boat Validation

Compare all important values to existing instrumentation and validate disconnect/error scenarios.

## Milestone 9 - Electrical Hardening

After the onboard TJA1051 prototype has proven the concept, review whether the final unit should use:

- galvanically isolated CAN
- improved transient/ESD protection
- dedicated marine-grade power conversion
- upgraded sealed connectors

## Version 1 Scope

Version 1 is a single-engine display. Twin-engine UI, cloud services, and remote monitoring are not required for the initial release.
