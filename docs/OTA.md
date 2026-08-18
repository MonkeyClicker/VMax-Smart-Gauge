# OTA Firmware Update Design

## Goal

Allow firmware updates without opening the water-resistant enclosure.

## Operating Modes

### Normal Mode

- Wi-Fi may remain off or be used only for approved features.
- Firmware upload endpoint is disabled.
- Gauge performs its normal NMEA/display/logging functions.

### Service / OTA Mode

Entered explicitly from the touchscreen.

Planned behavior:
- start a temporary Wi-Fi access point or connect to configured Wi-Fi
- expose a local firmware upload page
- display current firmware version and update progress
- automatically leave service mode after a timeout or reboot

## Partition Strategy

Use a dual-application OTA partition layout:

```text
bootloader
partition table
NVS / persistent settings
otadata
OTA app slot A
OTA app slot B
filesystem / other data as required
```

A new firmware image is written to the inactive app slot.

## Validation and Rollback

After an OTA reboot, validate software health before permanently accepting the image.

Initial health criteria should include:
- basic application startup completed
- configuration loaded
- CAN/TWAI initialization completed
- no immediate fatal fault

Do not require the Yamaha engine to be running or NMEA engine data to be present for firmware validation.

## Persistent Configuration

OTA replacement must preserve:
- unit preferences
- brightness/day-night settings
- selected engine/tank preferences
- Wi-Fi/service settings
- logging configuration

## Security Direction

Prototype:
- OTA disabled unless explicitly enabled locally
- service-mode timeout
- password/token protection

Future/productized version:
- signed firmware
- evaluate ESP32-S3 Secure Boot and flash encryption

## Recovery

Retain physical USB access inside the removable rear enclosure for recovery if both OTA software and normal startup fail.
