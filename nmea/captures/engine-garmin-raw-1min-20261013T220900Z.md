# Engine and Garmin one-minute raw test - October 13, 2026

File: [engine-garmin-raw-1min-20261013T220900Z.log](engine-garmin-raw-1min-20261013T220900Z.log)

- Start: 22:09:00 UTC / 6:09:00 PM America/New_York.
- User requested engine and Garmin together. Exact ignition/start/stop times
  were not independently confirmed; source 0 and source 2 raw traffic was received.
- Approximately 60 seconds of raw logging plus ten seconds to drain queued records.
- Original serial bytes preserved: 477704 bytes.
- Firmware: scanner-diag-3; 250 kbit/s, LISTEN_ONLY; USB COM6 at 115200 baud.
- Raw mode was stopped after the test.

## Verification

- Serial record validation: valid=5842, damaged_or_unframed=0, missing_records=0.
- Raw CAN records: 5418; raw capture-sequence gaps: 0.
- Missed-frame total: 51612 to 51612.
- CAN bus-error total: 1842 to 24103.
- Numeric diagnostic RPM range: 0.0 to 781.25 RPM.

| PGN | Source | Raw frame count |
|---:|---:|---:|
| 65314 | 2 | 548 |
| 65344 | 2 | 120 |
| 65345 | 2 | 60 |
| 65346 | 2 | 60 |
| 126720 | 0 | 1083 |
| 126993 | 0 | 1 |
| 127258 | 0 | 10 |
| 127488 | 2 | 548 |
| 127489 | 2 | 480 |
| 128259 | 2 | 60 |
| 129025 | 0 | 561 |
| 129026 | 0 | 240 |
| 129029 | 0 | 247 |
| 129283 | 0 | 48 |
| 129284 | 0 | 235 |
| 129539 | 0 | 33 |
| 129540 | 0 | 975 |
| 130310 | 0 | 106 |
| 130312 | 0 | 3 |

Payload bytes and timestamps are saved for offline decoding. Multi-frame messages must be reassembled and checked for completeness before interpretation. This file contains navigation/location payloads. Serial-record integrity and CAN reception completeness are separate checks.
