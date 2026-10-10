# Scanner diagnostics and captures

The `nmea-scanner` target remains receive-only at 250 kbit/s on GPIO15 TX /
GPIO16 RX. TIMING reports the configured clock source, quanta frequency, BRP,
segments, SJW, and sampling mode. BRP=0 means the driver derives its prescaler
from the quanta frequency; it is not a zero physical prescaler.

The diagnostics update does not change bitrate, termination, or
transmit mode. Flash using the existing `firmware/README.md` instructions.

## First repeat boat test

1. Open the USB serial monitor at 115200 baud and save its original output.
2. Reboot the scanner and retain the BOOT and TIMING records. BOOT is retried until BOOT/TIMING writes complete, repeated on an observed USB
   reconnect, and available on demand by sending lowercase `i`. BOOT identifies
   the diagnostic firmware, build date/time, SDK, reset reason, pins, and queues.
3. Run for at least one minute with GPS/network powered and the engine stopped.
4. Compare CAN `busDelta`, `busHz`, REC/TEC, LOSS deltas, and the alert mask.
5. Repeat with the engine idling and compare RPM/trim with existing instruments.
6. Save power/ignition state, termination-switch position, cable configuration,
   and observed instrument behavior beside the capture under `nmea/captures/`.

An increasing bus-error count means the local controller observes errors;
it does not prove which device or wiring component caused them. In listen-only
mode, zero REC/TEC and a RUNNING controller do not establish healthy reception.
The October 9 capture increased by roughly 305 errors/second, with no increase
in its missed/overrun counters. Compare these separately on the next test.

Inspect CAN-H/CAN-L and reference-ground connections, the gauge termination
switch (disabled for a drop), and backbone termination before changing timing.
Resistance checks require the network powered off. Do not enable transmission
as a diagnostic shortcut. Use the board photos and complete wiring guide.

## Records and logging integrity

After initialization, one task owns serial output. Each diagnostic record has:

`@<record-sequence> <payload> *<8-digit-uppercase-FNV1a-checksum>`

A leading newline on each write lets the next complete record resynchronize
after a partial write. Blank lines are ignored by the validator.

The scanner-diag-3 checksum covers `@<record-sequence> <payload>` together,
including the marker, decimal sequence, and separator. It excludes the leading
newline and the space before the checksum suffix, suffix, and final newline.
The validator now uses this format and intentionally rejects older
scanner-diag-2 payload-only checksums. This detects accidental capture corruption; it is
not a cryptographic checksum. Records are formatted into bounded buffers;
Serial writes retry partial acceptance with a 250 ms record deadline and a
shared 250 ms deadline for the entire metadata/snapshot/raw batch. Available-space
checks and a 10 ms USB write/lock timeout bound each transport operation.
Commands are serviced before records and during partial-write retries, so `s`
changes capture state while a USB write is stalled. Mode acknowledgements may
wait until the next batch. Scheduling and an in-flight USB operation can extend
wall-clock duration beyond the deadline; this is not a hard real-time guarantee. They execute outside
the CAN receive loop. Scanner core debug output is disabled to avoid interleaving
framework diagnostics with records after startup.

LOG records report cumulative `incompleteWrites`, `formatFailures`,
`budgetSkippedRecords`, `overwrittenSnapshots`, `rawDrops`, `alertPollFailures`,
and `statusFailures`. Budget exhaustion can leave a snapshot incomplete, which
the validator flags; skipped records are counted rather than silently extending
the batch.
These are separate from TWAI's hardware/driver counters. The one-entry snapshot
mailbox deliberately keeps the latest report when the host is slow; gaps in
SNAP sequence numbers count overwritten reports. SNAP begin/end markers expose
partial reports. A host can still lose bytes even when the device accepts a
complete write, so retain checksum validation and sequence checks.

Run `python tools/validate_scanner_log.py capture.log` on a saved capture.
It reports valid records, damaged/unframed lines or incomplete/mismatched
snapshot boundaries, and missing record sequences;
exit status is nonzero on damage, gaps, or no valid records. Initialization text
before the first numbered record is intentionally unframed. Validate only the
numbered portion for a clean result. Separate captures at each reboot; sequence
numbers reset at boot and the validator flags backward/repeated numbers.

## CAN and receive diagnostics

CAN reports cumulative bus errors plus interval delta and errors/second.
`deltaValid=0` means an adjacent status sample was unavailable; the delta/rate
are zero placeholders, not evidence of zero errors. Interval timing uses actual
elapsed milliseconds. Counters and times use unsigned subtraction for rollover.

LOSS reports cumulative missed frames/overruns and interval changes. Other
fields include cumulative unexpected receive failures (normal timeouts are
excluded), maximum loop duration for the interval, and a sampled receive-queue
high-water mark. Loop duration includes the intentional receive wait of up to
20 ms and scheduling gaps between loop entries. High-water sampling occurs after receives and can miss brief peaks.

The alert mask combines events observed during the reporting interval:

| Mask | TWAI alert |
| --- | --- |
| 0x00000008 | Below error-warning limit |
| 0x00000010 | Error active |
| 0x00000040 | Bus recovered |
| 0x00000100 | Above error-warning limit |
| 0x00000200 | Bus error |
| 0x00000800 | Receive queue full |
| 0x00001000 | Error passive |
| 0x00002000 | Bus off |

These are event flags, not event counts or specific bit/stuff/CRC/form/ACK
classifications. No automatic recovery or restart is introduced. The display
shows `RECEIVING - CAN ERRORS` in amber when fresh traffic is present and the
bus-error counter has increased within the last five seconds. Its rotating
indicator still means the receive loop runs, not that the bus is error-free.

## Optional raw-frame capture

Send lowercase `r` over USB to enable raw capture; send `s` to stop enqueueing.
Send `i` to request BOOT/TIMING metadata without rebooting.
The logger confirms commands in MODE records. Capture starts disabled at boot.
The queue holds 256 extended, non-RTR frames and never blocks reception.
Pending frames drain after `s`; the command does not erase them. Commands are
read in bounded groups without logging recursively from the input handler.

RAW records include capture sequence, microsecond timestamp, 29-bit CAN ID,
DLC, and hex payload. The capture sequence advances for every attempted enqueue,
so sequence gaps and cumulative `rawDrops` expose buffer loss. Timestamps wrap
at approximately 71.6 minutes; sequence numbers reset at boot. Raw and diagnostic
records share the serial logger, with bounded raw batches so snapshots and
commands receive service. Long snapshots, slow hosts, or high bus load can fill
the raw queue. This is a diagnostic capture, not a guaranteed lossless recorder.
Raw records include only successfully received frames, not malformed frames
rejected by the controller. PGN rates still count frames, including fast-packet
fragments; PGN 127489 is observed but its fast-packet engine fields are not yet
decoded. Source addresses alone do not verify Garmin/Yamaha device identity.

Raw navigation frames can contain location information. Review before publishing.
Hardware acceptance: test USB disconnect/reconnect, raw queue saturation, sustained
boat traffic, and comparison with existing instruments before relying on results.
