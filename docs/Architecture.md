# System Architecture

## Goal

Build a custom ESP32-S3 based NMEA 2000 multifunction display inspired by the Yamaha 6YC, for a 2024 Yamaha V MAX SHO 250 on an existing NMEA 2000 network with a Garmin 106sv.

## High-Level Data Flow

```text
Yamaha Engine ─┐
               ├── NMEA 2000 Backbone ──> Waveshare CAN ──> ESP32-S3
Garmin 106sv ──┘                                      │
                                                     ▼
                                              NMEA Parser
                                                     │
                                                     ▼
                                                BoatData
                         ┌───────────────────────────┼──────────────────────────┐
                         ▼                           ▼                          ▼
                       GUI                       Logger                     Diagnostics
                         │                           │                          │
                         ▼                           ▼                          ▼
                    LVGL Screen                 microSD                 PGN/source stats

ESP32 Wi-Fi ──> OTA service mode / future local diagnostics
```

## Architectural Rules

1. NMEA parsing is separate from UI rendering.
2. Internal data is stored in native engineering units where practical.
3. Every important data point carries freshness information.
4. Missing/unavailable values are distinct from zero.
5. The initial firmware is receive-first and should not intentionally publish application data.
6. The display must continue operating if SD logging fails.
7. OTA configuration and user settings must survive firmware replacement.
8. UI code should not hard-code a single NMEA source address.
9. Internal engine structures should remain capable of supporting multiple engine instances later.

## Major Software Modules

### NMEA Layer

Responsibilities:
- CAN/TWAI initialization
- NMEA 2000 message handling
- PGN decoding
- source address tracking
- message counters and update rates
- data freshness timestamps

### Data Model

`BoatData` is the boundary between network input and application features.

Expected data includes:
- engine RPM
- trim
- coolant temperature
- alternator voltage
- fuel rate
- engine hours
- engine load/torque when available
- engine warning/status bits
- GPS speed/course
- position
- fuel level/capacity

### UI Layer

Planned screens:
1. Main Engine
2. Performance
3. Trip
4. Diagnostics
5. Settings/System

### Logging Layer

Writes portable trip/performance data to microSD and must tolerate missing cards or interrupted power.

### OTA Layer

OTA is disabled during normal operation and enabled explicitly through a temporary service mode.

## Initial Development Order

1. Serial-only NMEA scanner
2. Decode Yamaha engine PGNs
3. Decode Garmin GPS/fuel data
4. Add PGN/source diagnostics
5. Bring up LCD/touch
6. Main engine UI
7. SD logging
8. OTA
9. enclosure integration and boat validation
