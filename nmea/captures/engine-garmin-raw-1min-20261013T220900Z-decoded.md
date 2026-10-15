# Decoded engine + Garmin raw capture - October 13, 2026

Source: [engine-garmin-raw-1min-20261013T220900Z.log](engine-garmin-raw-1min-20261013T220900Z.log)

Observed values and ranges from complete messages. Reserved padding is omitted. NA means encoded unavailable; ERROR and RESERVED are distinct sentinel states. Enum names follow the reference definitions. Satellite fields aggregate all satellites; the JSON keeps individual satellite entries. Proprietary field meanings are not inferred. These are received values, not independently verified instrument measurements.

Decoder references: [CANboat PGN definitions](https://github.com/canboat/canboat/blob/master/docs/canboat.json) and [NMEA2000 parser](https://github.com/ttlappalainen/NMEA2000/blob/master/src/N2kMessages.cpp).

Serial integrity previously passed: 5842 valid records, no gaps or damaged records. CAN errors still increased; this report cannot recover frames rejected by the controller.

## PGN 65314 - Proprietary - unidentified (source 2)

Raw frames: 548; decoded complete messages: 0; rejected fragments/messages: 0.

Field names and decoded values: unknown. Example raw frame payloads (not necessarily complete messages):

- `469F01E02D0EFFFF`
- `469F01E0350EFFFF`
- `469F01E03D0EFFFF`

## PGN 65344 - Proprietary - unidentified (source 2)

Raw frames: 120; decoded complete messages: 0; rejected fragments/messages: 0.

Field names and decoded values: unknown. Example raw frame payloads (not necessarily complete messages):

- `469F01FFFF7FA00C`
- `469F01FFFF7FAA0C`
- `469F01FFFF7FB40C`

## PGN 65345 - Proprietary - unidentified (source 2)

Raw frames: 60; decoded complete messages: 0; rejected fragments/messages: 0.

Field names and decoded values: unknown. Example raw frame payloads (not necessarily complete messages):

- `469F01FFFFFF0000`

## PGN 65346 - Proprietary - unidentified (source 2)

Raw frames: 60; decoded complete messages: 0; rejected fragments/messages: 0.

Field names and decoded values: unknown. Example raw frame payloads (not necessarily complete messages):

- `469F01E0C00000FF`

## PGN 126720 - Proprietary - unidentified (source 0)

Raw frames: 1083; decoded complete messages: 0; rejected fragments/messages: 0.

Field names and decoded values: unknown. Example raw frame payloads (not necessarily complete messages):

- `C16F12C3406F12C3`
- `C240B6C307410000`
- `C300000000000000`

## PGN 126993 - Heartbeat (source 0)

Raw frames: 1; decoded complete messages: 1; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| Data transmit offset | 60 | s |
| Sequence Counter | 10 |  |
| Controller 1 State | NA |  |
| Controller 2 State | NA |  |
| Equipment Status | Operational |  |

## PGN 127258 - Magnetic Variation (source 0)

Raw frames: 10; decoded complete messages: 10; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| SID | NA |  |
| Source | Automatic Table |  |
| Age of service | 2026-10-13 |  |
| Variation | -5.8728174 | deg |

## PGN 127488 - Engine Parameters, Rapid Update (source 2)

Raw frames: 548; decoded complete messages: 548; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| Instance | 1 |  |
| Speed | 0 to 1445.25 | rpm |
| Boost Pressure | 5.45341894 to 14.6778191 | psi |
| Tilt/Trim | 81 to 83 | % |

## PGN 127489 - Engine Parameters, Dynamic (source 2)

Raw frames: 480; decoded complete messages: 120; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| Instance | 1 |  |
| Oil pressure | 1.11679058 to 68.4288047 | psi |
| Oil temperature | NA |  |
| Temperature | 114.8 to 118.4 | deg F |
| Alternator Potential | 9.48 to 14.19 | V |
| Fuel Rate | 0 to 7.9 | L/h |
| Total Engine hours | 135.566667 to 135.579444 | hours |
| Coolant Pressure | NA |  |
| Fuel Pressure | NA |  |
| Discrete Status 1 | 0x0000 (no flags set) |  |
| Discrete Status 2 | 0xFFE0 (flag interpretation unverified) |  |
| Engine Load | NA |  |
| Engine Torque | NA |  |

## PGN 128259 - Speed (source 2)

Raw frames: 60; decoded complete messages: 60; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| SID | NA |  |
| Speed Water Referenced | NA |  |
| Speed Ground Referenced | NA |  |
| Speed Water Referenced Type | NA |  |
| Speed Direction | NA |  |

## PGN 129025 - Position, Rapid Update (source 0)

Raw frames: 561; decoded complete messages: 561; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| Latitude | 27.4049888 to 27.4050043 | deg |
| Longitude | -82.331292 to -82.3312601 | deg |

## PGN 129026 - COG & SOG, Rapid Update (source 0)

Raw frames: 240; decoded complete messages: 240; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| SID | NA |  |
| COG Reference | True |  |
| COG | 0 | deg |
| SOG | 0 | m/s |

## PGN 129029 - GNSS Position Data (source 0)

Raw frames: 247; decoded complete messages: 17; rejected fragments/messages: 25.

| Field | Decoded value / range | Unit |
|---|---|---|
| SID | NA |  |
| Date | 2026-10-13 |  |
| Time | 22:09:19.031 UTC, 22:09:20.031 UTC, 22:09:22.031 UTC, 22:09:24.031 UTC, 22:09:27.031 UTC, 22:09:28.031 UTC, 22:09:29.031 UTC, 22:09:30.031 UTC, 22:09:31.031 UTC, 22:09:32.031 UTC, 22:09:33.031 UTC, 22:09:34.031 UTC, 22:09:35.031 UTC, 22:09:36.023 UTC, 22:09:37.031 UTC, 22:09:39.031 UTC, 22:09:40.031 UTC |  |
| Latitude | 27.4049994 to 27.4050038 | deg |
| Longitude | -82.3312913 to -82.3312684 | deg |
| Altitude | -47.73948 to -45.132484 | m |
| GNSS type | GPS+GLONASS |  |
| Method | GNSS fix |  |
| Integrity | No integrity checking |  |
| Number of SVs | 16 to 18 |  |
| HDOP | 0.61 to 0.72 |  |
| PDOP | 1.1 to 1.42 |  |
| Geoidal Separation | -28.77 | m |
| Reference Stations | 0 |  |

## PGN 129283 - Cross Track Error (source 0)

Raw frames: 48; decoded complete messages: 48; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| SID | NA |  |
| XTE mode | Autonomous |  |
| Navigation Terminated | NA |  |
| XTE | NA |  |

## PGN 129284 - Navigation Data (source 0)

Raw frames: 235; decoded complete messages: 35; rejected fragments/messages: 57.

| Field | Decoded value / range | Unit |
|---|---|---|
| SID | NA |  |
| Distance to Waypoint | NA |  |
| Course/Bearing reference | NA |  |
| Perpendicular Crossed | NA |  |
| Arrival Circle Entered | No |  |
| Calculation Type | Great Circle |  |
| ETA Time | NA |  |
| ETA Date | NA |  |
| Bearing, Origin to Destination Waypoint | NA |  |
| Bearing, Position to Destination Waypoint | NA |  |
| Origin Waypoint Number | NA |  |
| Destination Waypoint Number | NA |  |
| Destination Latitude | NA |  |
| Destination Longitude | NA |  |
| Waypoint Closing Velocity | NA |  |

## PGN 129539 - GNSS DOPs (source 0)

Raw frames: 33; decoded complete messages: 33; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| SID | NA |  |
| Desired Mode | 3D |  |
| Actual Mode | Auto |  |
| HDOP | 0.61 to 0.72 |  |
| VDOP | 0.91 to 1.23 |  |
| TDOP | 0.52 to 0.76 |  |

## PGN 129540 - GNSS Sats in View (source 0)

Raw frames: 975; decoded complete messages: 0; rejected fragments/messages: 923.

| Field | Decoded value / range | Unit |
|---|---|---|

No complete payload was available for reliable field decoding.

## PGN 130310 - Environmental Parameters (obsolete) (source 0)

Raw frames: 106; decoded complete messages: 106; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| SID | NA |  |
| Water Temperature | 79.484 to 79.502 | deg F |
| Outside Ambient Air Temperature | NA |  |
| Atmospheric Pressure | NA |  |

## PGN 130312 - Temperature (source 0)

Raw frames: 3; decoded complete messages: 3; rejected fragments/messages: 0.

| Field | Decoded value / range | Unit |
|---|---|---|
| SID | NA |  |
| Instance | 0 |  |
| Source | Sea Temperature |  |
| Actual Temperature | 79.502 | deg F |
| Set Temperature | NA |  |

Unfinished fast-packet messages at capture end: 8.
