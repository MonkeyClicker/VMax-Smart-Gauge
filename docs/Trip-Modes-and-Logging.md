# Trip Modes and Logging

Feature requirements confirmed: 2026-10-06. Technical implementation proposals
remain subject to validation. Documentation only; firmware behavior is unchanged.

## Purpose and confirmed requirements

The Trip screen supports two modes selected in Settings:

- **Manual:** a trip continues until the user confirms Reset Trip.
- **Automatic:** a trip groups recorded activity by calendar day and resets for the next day.
- Automatic totals survive multiple gauge starts and stops during the same day.
- Both modes write trip data to portable log files for later download or export.

Confirmed choices: local midnight using a Settings time zone; save the current trip
and start a new trip when changing modes; summary-only logs via local Wi-Fi download
and SD copy; separate attribution and logs for Manual and Automatic; engine-running
trip time; Automatic as the initial default; up to 30 seconds of recent data loss
on sudden power removal; logs retained until explicitly deleted. This document is
the detailed feature specification; existing UI and architecture documents describe
the wider system.

## Current implementation

The simulated-gauge target has a Trip page with elapsed time, integrated distance
and fuel, average economy, maximum speed/RPM, and best cruise economy. Reset Trip
requires a second tap within five seconds. Settings and totals are session-only;
SD and Wi-Fi services are inactive. The simulator currently uses display units
internally and increments elapsed time even during its Offline scenario.

The production feature needs a shared trip service, persistent recovery, a reliable
calendar clock, SD logging, and an export interface. Simulator behavior must be
adapted explicitly rather than treated as the production specification.

## Proposed user experience

### Settings

Add a Trip & Logs section with:

- Trip mode: **Automatic / Manual**, persisted across restart and OTA.
- Time zone and current local date/time, with clock-source/status indication.
- Log status, available SD space, and Open Log Downloads.
- Summary-only recording; no GPS positions or periodic reading exports.

Initial mode is Automatic. Subsequent starts use the user's saved mode.
Trip recording is required in either mode; optional raw network/performance logging
remains a separate setting.

### Trip screen

Keep the existing six metric cards and best-cruise detail. Add a compact mode/date
label and recording status in the existing footer area:

| Mode | Heading/detail | Primary action |
| --- | --- | --- |
| Manual | Manual Trip; start date or Time unavailable | Reset Trip |
| Automatic | Today's Trip; local date | View Logs |
| Automatic, unresolved date | Automatic; Waiting for date | View Logs |

Reset Trip is unavailable in Automatic mode. Explain daily reset in Settings:
“Combines today's recorded sessions. Starts a new trip at local midnight.”

Manual reset retains the existing two-tap confirmation and five-second expiry.
The confirmation says “Save this trip and reset totals.” Reset closes the old
record and starts a new one; it never deletes history or changes tank level,
lifetime engine hours, or unrelated Performance records.

Show explicit status messages: Recording, Waiting for date, Data incomplete,
SD unavailable, Log storage full, or Recovery warning. Historical totals remain
visible when live inputs are stale. Logging errors must not interrupt the gauge.

### Meaning of trip time

Label the time card **Engine Running Time**. In both modes, accumulate time only
while the selected engine is confirmed running. Include engine idle time; exclude
engine-off time, gauge-off time, and intervals with stale/unavailable running status.
Persist this duration with the other totals and combine it across same-day Automatic
sessions. Do not infer unobserved running time from a restart or calendar difference.

Proposed detection: fresh selected-engine RPM greater than zero means running;
fresh zero RPM means stopped. Require valid endpoints and a bounded monotonic
interval; never carry the last running state through a data outage. Validate RPM
behavior during cranking and engine shutdown on the boat before fixing detection
thresholds. Mark missing running-status coverage as incomplete rather than adding
estimated time. Keep observation time and valid running-status coverage separately
for diagnostics/export; neither replaces the displayed engine-running duration.

Distance and fuel retain independent input-validity rules. Wall-clock start/end
timestamps describe the record span, not its engine-running duration.

## Mode and lifecycle rules

Changing mode saves and closes the current trip with reason `mode_change`, then
starts a new trip ID in the selected mode. Restarting without changing modes resumes
that trip. No closed trip is resumed after a mode change.

Proposal: each Automatic selection creates a segment linked to a daily key
(device, local date, time zone). The Trip screen shows combined Automatic totals
for that day; exports retain segment summaries and a clearly labeled daily rollup.
Rollups combine sums, maxima, coverage and best qualifying economy; never average
segment economies. Rollups must not be added to their segments in analysis.

Manual periods contribute only to Manual trips; Automatic periods contribute only
to Automatic segments and their daily rollup. No background cross-mode accumulation.
Use separate `manual-trip-summary.csv` and `automatic-trip-summary.csv` files and
separate mode filters in downloads.

| Event | Manual | Automatic |
| --- | --- | --- |
| First use | Create persistent trip ID | Create daily trip after date is known |
| Restart, same local day | Resume saved trip | Resume the same daily trip |
| Restart, later local day | Resume saved trip | Finalize old day; open current day |
| Confirm Reset Trip | Finalize and checkpoint; open new trip | Action unavailable |
| Midnight while running | Continue same trip | Split observation interval; finalize day; open next day |
| Mode change | Save current trip; start selected mode with new ID | Save current segment; start selected mode with new ID |
| Power loss | Recover latest valid commit | Recover latest valid commit, then reconcile date |
| Days with no observations | No fabricated data | No empty daily trips required |

“Entire day” means all eligible observations assigned to that day, across all
power sessions. The gauge cannot reconstruct distance, fuel, or peak values while
powered off. Never bridge a shutdown or stale-data gap with the last received rate.

## Calendar and clock handling

Confirmed: days run from local midnight to the next local midnight using a Settings
time zone with daylight-saving rules. Store timestamps in UTC and retain each trip's
local date and time-zone identity. Do not assume every local day is 24 hours.

Use validated date/time from the boat network where available; confirm the actual
Garmin transmission during discovery. A user-set clock can be a fallback for the
current power session. The current design does not establish a battery-backed clock;
a saved timestamp alone cannot determine how long the gauge was powered off.

- Use a monotonic timer for integration; calendar corrections cannot add distance,
  fuel, or engine-running time.
- On boot without a trustworthy date, preserve saved totals and put new Automatic
  observations in a separate undated session. Do not assume it is the saved day.
- When time becomes valid, assign intervals only if the session has a trustworthy
  continuous monotonic timeline and a consistent calendar anchor. Split at midnight
  where necessary; keep ambiguous sessions undated and flag them for export.
- Loss of network time after synchronization may use the current session's running
  clock, with a degraded clock status. A restart requires a new trustworthy anchor.
- Forward/backward clock jumps record a correction event. They do not erase data
  or reset totals repeatedly. Suspend date assignment when inconsistent and resume
  only after validation; retain uncertain observations separately.
- A corrected date can refer to an existing daily record. Reuse its ID and append a
  revision instead of creating a second independent total for that date.
- Apply time-zone changes prospectively at the next daily boundary; retain the active
  day's zone until then and show the pending change. Do not rewrite historical dates.

## Metrics and data quality

The trip service consumes normalized BoatData and freshness information independently
of which UI page is open. Use SI units internally: meters, liters, seconds, RPM.

- Integrate fresh GPS speed for distance and fresh engine fuel rate for fuel.
  Distance and fuel validity are independent; one missing source does not stop the other.
- Use bounded monotonic intervals and trapezoidal integration when both endpoints
  are valid. Never integrate across boot, mode changes, stale gaps, or excessive
  scheduling delays. Split valid intervals at a daily boundary before accumulation.
- Maximum speed and RPM use valid observations only. Empty metrics are unavailable,
  not zero; measured stationary speed is a valid zero.
- Average economy is total distance divided by total fuel only when fuel is nonzero
  and coverage is adequate. Also track distance and fuel over intervals where both
  inputs are valid; use those paired totals for an explicitly labeled partial economy
  when coverage differs. Export both total and paired coverage.
- Best cruise economy requires fresh speed/flow/RPM and a stable observation window.
  Proposed initial rule: RPM at least 2000, nonzero flow, and a 10-second stable window;
  stability tolerances require boat validation. Record RPM and trim with the result.
- Trip peak/best values reset with their trip. Performance-page learning has its own
  lifecycle and must not be reset inadvertently by the trip service.
- Record observed, valid-distance, valid-fuel, paired-valid, and missing-input durations.
  Also record engine-running duration and valid running-status coverage.
  Mark estimated, undated, recovered, and incomplete records explicitly.

## Persistence and interrupted-power recovery

Separate small durable trip state from larger SD history. Proposed design:

1. Persist mode/time-zone/log preferences in NVS; write only when changed.
2. Keep versioned, checksummed trip snapshots in alternating slots in a dedicated
   internal-flash journal. Include sequence, trip IDs, current totals, quality flags,
   clock assignment, and the last committed log sequence.
3. Keep an SD append journal of summary revisions and lifecycle events for recovery. Bound every
   record with length/sequence/checksum so a partial tail can be detected and ignored.
4. On boot, restore the newest valid snapshot and replay newer committed records
   exactly once. Reconcile checkpoint and SD sequences; never add cumulative totals
   as deltas. An older or swapped SD card cannot roll internal state backward.

Accepted recovery target: at most 30 seconds of recent uncommitted observations
may be lost on sudden power removal during normal storage operation. Checkpoint/flush
at intervals no longer than 30 seconds, accounting for write latency, and at reset,
mode change, daily rollover, export snapshot, and controlled reboot/OTA. Validate
actual durability and flash endurance before selecting the implementation cadence.
Storage faults can exceed this target and must produce a visible degraded status.

Reset and rollover are idempotent journal transactions containing the closed trip,
new trip identity, and boundary reason. Commit the transition before publishing a
zeroed display. Repeated recovery must not close twice or create duplicate trips.
Retain the previous snapshot until the replacement passes validation. If neither
snapshot is valid, attempt SD reconstruction and show a recovery warning.

Use a bounded internal pending-summary journal during SD failure. Export/replay buffered
records when SD returns, deduplicated by device ID and sequence. If its capacity is
exhausted, preserve aggregate state and record an explicit summary-history gap; never
promise unlimited history without SD. Missing/full/read-only cards must leave UI,
CAN reception, and trip calculations operational.

Internal partition allocation, flash write budget, buffer capacity, and maximum
recovery time are implementation gates. Version trip state and journal schema for
OTA; unsupported schemas must be preserved and reported rather than silently reset.

## Log files and export

Confirmed scope: summary-only logs, with no periodic readings or GPS positions.
Store a versioned recovery journal under `/trips/` on microSD and generate portable
`manual-trip-summary.csv` and `automatic-trip-summary.csv`. Internal recovery events
support correctness but are not a
separate user-facing detailed log.

Each row includes schema/device/trip ID, record kind (`trip_segment` or
`daily_rollup`), daily key where applicable, mode, local date, time zone, UTC
start/end, status/revision, close reason, session count, engine-running seconds,
observation seconds, valid running-status seconds,
distance/fuel totals, maxima, best economy/RPM/trim, paired totals, coverage,
clock quality, recovery status and missing-history flags. Export one latest
revision per ID, including active/incomplete records. Clearly label daily rollups
to prevent double counting. Persist active segment and daily rollup recovery state.

Use column names with explicit SI units, ISO 8601 UTC timestamps, decimal points,
standard CSV quoting, and blank unavailable values. Display-unit changes leave
stored data and exports unchanged. CSV is a view of committed summary state, not
the recovery source. Checkpoints recover totals without storing individual readings.

Proposed retrieval paths:

- **Local Wi-Fi:** Settings > Trip & Logs > Open Log Downloads starts a temporary,
  protected local service, with date/mode filtering and CSV download to phone/laptop.
  Firmware upload remains separately enabled through OTA service mode.
- **SD copy:** summary CSV exports can also be copied directly from the card.
  Refresh them at trip closure and on explicit Export to SD; indicate snapshot time.

Create a consistent committed snapshot for an active-trip download; ongoing recording
continues through a bounded worker queue. Export file access cannot block CAN or LVGL.
Label active/partial exports and include the snapshot time. No cloud dependency.

Confirmed retention: retain logs until explicitly deleted; never silently overwrite
old trips when storage fills. Offer selected-log deletion only through an explicit
confirmation. No age-based or capacity-based automatic deletion. Deletion must update
the journal and CSV views so recovery cannot restore deleted records; remove deleted
segments from derived daily rollups. An active record cannot be deleted until closed.

## Module boundaries and implementation sequence

- `TripService`: modes, integration, daily assignment, reset, immutable UI snapshots.
- `ClockService`: trustworthy UTC, time-zone rules, monotonic/calendar anchoring.
- `TripStore`: versioned checkpoints, transactions, recovery and pending records.
- `TripLogger`: SD append journal, quality/events, bounded asynchronous writes.
- `TripExporter`: committed CSV snapshots, SD export and local download service.
- Settings/Trip UI: commands and status only; no storage I/O inside touch callbacks.

Implement in stages: pure trip/clock state machines; persistence and recovery;
SD journal/CSV export; settings and Trip UI; local downloads; simulator scenarios
and boat validation. Keep generated simulator history separate from real boat logs
and label its origin in every export.

## Acceptance and validation

| Scenario | Required result |
| --- | --- |
| Three starts/stops on the same date | One Automatic trip; sums committed observations without duplicates |
| Boot tomorrow after yesterday's shutdown | Yesterday preserved; today's totals start at zero |
| Running across midnight | Valid interval split correctly; old day retained |
| Manual across midnight and several restarts | Same trip until confirmed reset |
| Manual reset and reboot during transaction | Old trip saved; exactly one new trip; unrelated values preserved |
| Change mode and restart | Preference restored; saved trip, new segment and daily rollup recover correctly |
| Alternate Manual and Automatic on one day | Separate mode files; Automatic rollup excludes all Manual activity |
| Engine idle, stopped, stale RPM, and restart | Running time includes valid idle; excludes stopped/off/unknown intervals |
| New device and later restart | Automatic initially; saved mode honored thereafter |
| Boot with no valid date | No destructive rollover; new data remains undated until safely assignable |
| Clock jump, time-zone change, DST boundary | No duplicate/reset loop; deterministic day assignment and events |
| Stale speed, stale flow, or valid zero | Independent integration and accurate unavailable/partial indicators |
| Sudden power removal or truncated record | Latest valid commit recovered; at most 30 seconds lost with healthy storage; no double counting |
| Old logs, full storage, explicit deletion, recovery | No automatic deletion/overwrite; user-deleted logs stay deleted after recovery |
| Missing/full/swapped SD or failed flash write | Gauge continues; visible error; explicit recovery/log-gap behavior |
| Download during recording | Consistent labeled snapshot; recording and display remain responsive |
| OTA and repeated recovery | Schema compatibility checked; state/history preserved or incompatibility reported |
| Units change and log export | Stored totals unchanged; documented CSV units and values |

Use deterministic host tests for integration, midnight/DST, mode transitions,
transaction replay, and CSV schema. Use device fault injection for interrupted writes,
card failures and queue limits. Measure flash wear estimates, record-loss window,
download responsiveness and recovery time. Compare fuel/distance with boat instruments;
document freshness thresholds and clock availability using actual network captures.

## Remaining engineering validation

No unanswered user preference questions remain. Implementation must validate network
clock availability, engine-running detection, flash allocation/endurance, write
durability, and recovery-buffer capacity. The segment/daily-rollup structure and
clock-reconciliation rules are technical design proposals supporting the confirmed
behavior; validate them with the acceptance scenarios before release.
