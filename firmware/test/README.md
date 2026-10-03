# Firmware Tests

Run portable scanner regression tests from the repository root:

```bash
bash firmware/test/host/run.sh
```

A C++17 compiler is required. CI runs the same tests and builds both ESP32 targets.
The host tests compile the production `Scanner.h` and `BoatData.h` directly; no
Arduino or TWAI stubs are needed. They cover parser and statistics behavior,
including unavailable data, signed fuel values, identity changes, truncated
payloads, rollover, offline rates, and full-table eviction.

Serial backpressure, TWAI loss counters, and actual display behavior still need
hardware validation. Sanitized boat inputs belong under `nmea/sample-data/`.
