# Garmin-only one-minute raw test - October 13, 2026

File: [garmin-only-raw-1min-20261013T220635Z.log](garmin-only-raw-1min-20261013T220635Z.log)

- Start: 22:06:35 UTC / 6:06:35 PM America/New_York.
- User requested Garmin only; engine was not started for this test.
- Raw logging enabled for approximately 60 seconds, then stopped; queued records
  were saved for approximately ten additional seconds.
- Original serial output preserved: 3794394 bytes.
- Firmware: scanner-diag-3; 250 kbit/s, LISTEN_ONLY; USB COM6 at 115200 baud.
- Both MODE raw=1 and MODE raw=0 acknowledgements were saved.

## Capture verification

- 44929 valid checksummed serial records, zero damaged/unframed records,
  zero missing record sequences.
- 44488 raw CAN frames, zero raw capture-sequence gaps, rawDrops=0.
- All raw frames: PGN 129025 (position rapid update), source address 0.
- Distinct raw payloads: 1.
- Cumulative CAN missed frames: 149987 to 212204.
- Cumulative CAN bus errors: 9143 to 9143.

The raw payload bytes can be decoded offline. PGN 129025 contains location
information; review before publishing. High CAN frame loss and the repeated
position frames mean this is not a validated healthy or complete network capture.
Retained diagnostic PGN rows can contain history from devices that are no longer
active; only source 0 / PGN 129025 appeared in this raw sample. No engine dynamic
payloads were captured. Raw logging was stopped after the requested test.
