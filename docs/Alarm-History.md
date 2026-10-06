# Alarm Logging and History

Feature request documented: 2026-10-06. Documentation only; firmware is unchanged.
Detailed behaviors below are proposed implementation defaults.

## Requirements and scope

Log all alarms detected by the gauge and provide an on-screen Alarm History view.
Include engine/network alarms, locally evaluated threshold alarms, and gauge/system
faults at Advisory, Warning and Critical severity. Record each condition independently,
even when another alarm has priority or audible output is disabled/silenced.

“All alarms” means conditions the gauge actually receives or detects; it cannot
recover alarms that occurred while powered off or decode unsupported engine warnings.
Touch-feedback chirps are not alarms. Label simulator alarms and dockside tests
separately so they cannot be mistaken for real engine incidents.

Extend [Audible Warnings](Audible-Warnings.md) without changing detection thresholds,
priority, acknowledgement, or silence rules. Logging records detector decisions;
it does not make provisional warning mappings authoritative.

## Occurrences and events

Assign each occurrence a durable unique ID. Repeated messages reporting the same
active condition update its last-observed context without creating new occurrences
or logging every network frame. Once confirmed cleared, recurrence creates a new ID.
Use the alarm detector's hysteresis/debounce rules; the logger adds no independent
alarm activation delay.

Record lifecycle transitions with unique sequence numbers:

| Event | Recorded behavior |
| --- | --- |
| Activated | New occurrence, severity, source and initial context |
| Severity changed | Old/new severity on the same occurrence |
| Acknowledged | Operator acknowledgement; condition remains active |
| Silenced | Scope, start, expiry and reason; condition remains active |
| Silence ended/overridden | Expiry, operator action or higher-priority override |
| Cleared | Confirmed detector clear and duration where measurable |
| Observation interrupted | Restart, stale source or unavailable input; not a confirmed clear |
| Observation resumed | Condition reconfirmed or cleared after interruption |

If Silence applies to several active alarms, record the shared action ID and affected
occurrence IDs. Preserve lower-priority alarms while a higher-priority sound plays.
Explicitly distinguish detector-disabled conditions from detected alarms with muted
sound: the former cannot be logged as observed without an active detector.

## Record contents and timing

Store schema/device/occurrence ID, event sequence, condition code and readable name,
category, severity, lifecycle state, acknowledgement/silence state, source engine or
tank identity, source address and PGN/status bit where applicable, and firmware version.
Capture triggering value, units, threshold/configuration revision and data freshness
when meaningful. Keep unavailable context blank rather than inventing measurements.

Use UTC timestamps plus clock quality and the configured local time zone for display.
Also retain boot/session ID and monotonic offset. Without valid calendar time, show
“Time unavailable” with session/order information. Do not invent a date or calculate
continuous duration across an unknown power-off interval. Calendar corrections do
not reorder event sequences or silently alter historical records.

Trip ID and mode may be included as optional context, but alarm history is independent
of trips. Reset, rollover, fuel adjustments and mode changes never clear alarm history.
No GPS positions are required.

## On-screen history

Diagnostics provides an **Alarm History** button, also available from the active
alarm detail panel. Show active conditions separately from past occurrences and
default history ordering to newest occurrence first.

List entries show severity, alarm name, start time, and state: Active, Cleared,
or Observation interrupted. Provide severity/category and date filters, including
undated records. Use a paginated or virtualized list to bound LVGL memory.

Touch an entry to view its occurrence timeline, source, triggering values, clear
time, known duration, and acknowledgement/silence actions. Unknown duration or
missing history is explicit. Opening history or marking a record reviewed does not
acknowledge an active alarm. Active acknowledgement/silence actions retain the
existing live alarm controls; deleting history cannot clear or silence a condition.

Screen standby does not stop alarm evaluation or logging. New Warning/Critical
conditions use the wake policy in [Screen Sleep and Wake](Screen-Sleep-and-Wake.md).
Viewing history counts as touch activity; merely leaving the page open does not
permanently prevent screen sleep.

## Persistence and failure handling

Use a versioned, checksummed append journal on microSD with a bounded durable internal
pending queue. Record transitions promptly and commit asynchronously without delaying
alarm presentation, CAN reception, buzzer scheduling or UI refresh. Proposed durability
target: commit within one second under healthy storage; validate this on device.
Unexpected power loss can still lose an uncommitted event; do not promise zero loss.

Restore committed history and last-known active occurrence identities after restart
and OTA. Mark previously active alarms as observation interrupted until detectors
evaluate fresh data. Do not assert they remained active or cleared while the gauge
was off. If reconfirmed active, append a resumed event to that occurrence; after a
confirmed clear, later activation creates a new occurrence.

Replay queued events exactly once using device/sequence IDs. Ignore a truncated tail,
preserve valid records and expose recovery problems. An old/swapped SD card cannot
roll state backward. History queries use indexed committed records and bounded caches;
they cannot require loading the whole log into memory.

If SD is missing/full, use the bounded internal queue and show **Alarm log unavailable**
or **Alarm log incomplete**. When exhausted, record a lost-event counter/sequence gap
in reserved metadata when possible. Do not create one new logging-failure alarm per
failed write: use one latched storage fault and prevent recursive event generation.
Alarm detection and live warnings continue even if all logging fails.

## Export and retention

Proposed defaults follow the existing trip-log preferences: retain history until
explicitly deleted, without automatic expiration or overwriting. Settings > Logs
provides Alarm History download over the temporary protected local Wi-Fi service
and Export to SD for direct card copy.

Provide `alarm-history.csv` with one latest occurrence summary per ID and
`alarm-events.csv` with lifecycle transitions. These are alarm-specific records;
they do not add periodic readings to the summary-only trip logs. Include schema,
explicit units, clock quality and snapshot time; export from a consistent committed
snapshot while live recording continues.

Deletion requires confirmation showing scope. Delete only closed historical
occurrences; retain active/interrupted occurrences until resolved. Apply a durable
deletion marker and update export/index views so recovery does not resurrect deleted
history. Old files already copied outside the gauge are unaffected. Full storage
prompts for export/deletion instead of silently overwriting records.

## Acceptance and implementation

Add `AlarmHistoryService` consuming transitions from the shared alarm manager,
with a storage adapter, index and exporter. UI reads snapshots and queues commands;
touch callbacks perform no file operations. Keep real and simulated/test history
separate in storage and downloads.

| Scenario | Required result |
| --- | --- |
| Same active alarm repeats in network messages | One occurrence; no event flood |
| Multiple alarms, muted sound or priority preemption | Each occurrence recorded independently |
| Acknowledge/silence/override/clear/recurrence | Correct timeline; recurrence gets a new ID |
| Input stale or gauge restarts during alarm | Interrupted state; no fabricated clear/duration |
| Clock unknown or corrected | Durable event order; honest timestamp quality |
| Screen off, trip reset, mode change or OTA | Logging continues where running; history persists |
| Missing/full SD, interrupted write or duplicate replay | Gauge remains responsive; gaps visible; no duplicate records |
| Long history and concurrent CSV download | Bounded memory and consistent snapshots |
| Delete then restart/recover | Selected closed history stays deleted; live alarms unaffected |
| Simulator or dockside test | Clearly separated from real incidents |

Validate transition deduplication and recovery with deterministic tests; verify
storage durability, bounded queues, long-history UI performance and real engine
warning mappings on the device/boat before release.
