# System Requirements

## Purpose

Build a custom single-engine NMEA 2000 smart gauge for a 2024 Yamaha V MAX SHO 250 using an ESP32-S3 touchscreen platform. The unit is inspired by the information density and usability of the Yamaha 6YC but is not intended to copy Yamaha firmware or proprietary behavior.

## Existing Boat Environment

- 2024 Yamaha V MAX SHO 250
- Existing NMEA 2000 network
- Garmin 106sv already connected
- Yamaha engine data already present on the network
- Available open NMEA 2000 drop connection
- Garmin powered from a LiFePO4 battery system
- LiFePO4 battery provides regulated USB power suitable for the prototype gauge

## Core Functional Requirements

The gauge shall display, when available on the network:

- engine RPM
- engine trim
- engine temperature
- alternator/battery voltage
- instantaneous fuel flow
- engine hours
- engine warning/status information
- GPS speed
- course over ground
- fuel tank level
- calculated fuel economy

## Calculated Values

The firmware should derive additional values from NMEA data, including:

- MPG from GPS speed and fuel flow
- trip fuel used
- trip distance
- average trip MPG
- best cruise MPG
- best observed RPM/trim combination
- maximum speed
- WOT RPM

## Display Requirements

- Single-engine display only for the initial implementation
- 5-inch 800x480 capacitive touchscreen
- Main screen optimized for quick helm visibility
- RPM is the most visually prominent value
- Speed, trim, fuel flow, temperature, fuel level, voltage, and hours visible without excessive navigation
- Day and night brightness presets
- Manual brightness override
- Clearly distinguish unavailable, stale, and valid zero values

## Audible Warning Requirements

- Use an external active piezo buzzer; the selected display board has no onboard audio device
- Switch the buzzer through isolated digital output DO0, not directly from an ESP32 GPIO
- Provide distinct patterns for touch feedback, advisory, warning, and critical conditions
- Generate patterns with a non-blocking state machine
- Higher-priority warnings shall preempt lower-priority sounds
- Allow acknowledgement and temporary silence without clearing the visual warning
- A new higher-priority warning shall cancel temporary silence
- Returning conditions shall sound again after clearing
- Provide a dockside test for every warning pattern
- Persist alarm settings across restart and OTA
- Keep thresholds configurable and provisional until validated
- Treat audible alarms as supplemental until compared against Yamaha/Garmin warning behavior

Detailed behavior is defined in [Audible Warning System](Audible-Warnings.md).

## NMEA 2000 Requirements

- Connect as a normal NMEA 2000 drop device
- Do not add a third backbone terminator
- Initial firmware shall operate as receive-first/listen-only during discovery
- Discover actual Yamaha and Garmin source addresses rather than hard-code assumptions
- Track PGN/source message counts and approximate rates
- Support NMEA 2000 fast-packet messages where required

Initial PGNs of interest:

- 127488 - Engine Parameters, Rapid Update
- 127489 - Engine Parameters, Dynamic
- 127505 - Fluid Level
- 129025 - Position, Rapid Update
- 129026 - COG & SOG, Rapid Update
- 129029 - GNSS Position Data

## Logging Requirements

- microSD logging
- portable log format, initially CSV
- periodic flush strategy
- logging failure must not stop normal gauge operation
- support trip/performance analysis
- avoid committing sensitive GPS history to the public repository by accident

## OTA Requirements

- OTA is a core feature because the finished enclosure should remain sealed during normal firmware updates
- Wi-Fi OTA disabled during normal operation
- OTA service mode enabled intentionally from the touchscreen
- Temporary local access point should be supported for dockside updates
- Firmware upload from phone or laptop
- Dual OTA partitions and rollback strategy
- Settings must persist across OTA updates
- Internal USB shall remain available for recovery/service

## Enclosure Requirements

- Target a highly water-resistant marine installation
- Do not claim a formal IP rating without test evidence
- ASA is the preferred 3D-print material for the final housing
- Expose the touchscreen glass through the bezel rather than covering it with another clear window
- Seal the touchscreen perimeter
- Use a gasketed removable rear cover
- Use a hydrophobic/ePTFE pressure equalization vent
- Consider conformal coating as secondary PCB protection
- Provide sealed cable entry or waterproof connectors
- Mount the buzzer without creating an uncontrolled water path and verify installed sound level

## Safety and Reliability Requirements

- Track age of important data fields
- Never indefinitely display stale engine data as if current
- Show NMEA offline/data unavailable state
- Handle brownouts and restarts cleanly
- Preserve trip data safely
- Default the buzzer output to off during boot, reset, OTA, and recovery
- Audible warning processing must not delay CAN reception, display refresh, logging, or watchdog service
- Treat the custom display as supplemental until warning/status behavior has been validated against the existing Yamaha/Garmin installation
