# Testing and Validation Plan

## Purpose

Validate the custom gauge without disrupting the existing Yamaha/Garmin installation and confirm that displayed values match the boat's known instrumentation.

## Phase 1 - Bench/Power Test

Verify:

- Waveshare boots reliably from the intended USB power source
- display remains stable at normal brightness
- serial programming/recovery works
- microSD can initialize and write test records
- Wi-Fi can be enabled for future OTA testing

## Phase 2 - Listen-Only NMEA Test

Connect the unit as a normal NMEA 2000 drop and run the initial firmware in listen-only mode.

Verify:

- Garmin remains fully operational
- Yamaha data remains available on the existing displays
- custom gauge sees NMEA frames
- no unintended application messages are transmitted
- CAN error state remains healthy
- termination configuration has not been changed

Record:

- source addresses
- observed PGNs
- message counts
- approximate message rates
- engine instance

## Phase 3 - Engine Data Validation

Compare custom-gauge values against the existing Garmin/Yamaha display under multiple conditions.

Test:

- engine off / network on
- idle
- 1500-2500 RPM
- normal cruise
- trim changes
- higher-RPM run when safe and appropriate

Compare:

- RPM
- trim
- temperature
- voltage
- fuel flow
- engine hours
- warnings/status where observable

## Phase 4 - GPS/Fuel Validation

Compare:

- GPS speed
- course
- fuel tank level, if PGN 127505 is present
- calculated MPG

The MPG calculation should use synchronized/fresh speed and fuel-flow values.

## Phase 5 - Failure Modes

Exercise:

- NMEA cable disconnected
- engine data stops while GPS remains active
- GPS data stops while engine remains active
- SD card missing
- SD card write failure/full condition where practical
- Wi-Fi disabled
- unexpected gauge restart
- temporary power interruption

Expected behavior:

- stale values are not shown indefinitely as current
- unavailable values are visually distinguishable from zero
- normal gauge operation continues if logging fails
- reboot does not corrupt persistent configuration

## Phase 6 - OTA Validation

Before sealing the enclosure, verify:

- OTA service mode can be intentionally enabled
- update can be uploaded from a phone/laptop
- firmware reboots successfully
- settings persist
- failed/test firmware can be recovered or rolled back
- internal USB recovery remains usable

## Phase 7 - Enclosure Test

Before installing permanently:

- inspect gasket compression
- inspect screen perimeter seal
- inspect cable strain relief
- verify pressure vent placement
- perform controlled splash/water testing without claiming a formal IP rating
- re-open enclosure and inspect for moisture ingress

## Capture Documentation

Store sanitized network captures in `nmea/captures/` and summarize observed PGN behavior in `nmea/pgn-notes/`.

Avoid publishing actual GPS history unless intentionally desired.

## Acceptance for Version 1

Version 1 is ready for normal use when:

- RPM, trim, engine temperature, voltage, fuel flow, engine hours, and GPS speed have been validated against existing instrumentation
- stale-data behavior is verified
- the gauge does not disturb the NMEA 2000 network
- OTA/recovery works
- logging failures do not affect core display functionality
- enclosure splash resistance has been practically tested

For numbered USB captures and CAN-error diagnosis, follow [Scanner Diagnostics](Scanner-Diagnostics.md).
