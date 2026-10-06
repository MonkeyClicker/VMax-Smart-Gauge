# Fuel Capacity and Manual Fuel Tracking

Feature requirements confirmed: 2026-10-06. Technical implementation proposals
remain subject to validation. Documentation only; firmware behavior is unchanged.

## Confirmed feature scope

- Settings provides a Fuel Configuration screen with configurable tank capacity
  in gallons.
- The Engine screen computes and displays remaining gallons.
- Fuel Configuration can set the current level in eighth-tank increments.
- The user can add a specified number of gallons or select FULL.
- Manual tracking supports operation without network fuel-level data.
- Settings explicitly selects Network or Manual; received network data never
  overwrites the Manual estimate or automatically changes the selected source.
- Capacity and additions accept whole US gallons only.
- The Engine screen shows remaining gallons only, without percentage.

This feature is separate from the Manual/Automatic **trip modes**. Tank contents
must survive trip reset, midnight rollover, mode changes, restart, and OTA.
See [Trip Modes and Logging](Trip-Modes-and-Logging.md).

## Existing behavior and implementation gap

The simulated gauge displays tank percentage, assumes a 60 US gallon tank, and
subtracts generated fuel consumption from a fixed starting level. It provides no
capacity, level-setting, or refueling controls and no persistent tank estimate.
BoatData already separates network tank percentage/capacity from engine fuel rate.
The production design must preserve that raw data while adding user configuration
and an independent fuel inventory service.

## Confirmed choices and technical proposals

Source selection, whole-US-gallon inputs, and gallons-only Engine presentation are
confirmed. Technical behavior below is proposed unless stated as confirmed. No
unanswered user preference questions remain.

## Fuel Configuration screen

Navigate from Settings > Fuel Configuration. Use large touch targets and a numeric
keypad; split configuration and refueling into subpanels if needed on the 800x480
screen rather than crowding the existing Settings page.

Show tank capacity, selected source, current remaining amount/percentage, and
estimate/data quality. Proposed controls:

| Control | Behavior |
| --- | --- |
| Tank Capacity (gal) | Enter a positive finite value; Save applies persistently |
| Fuel Source | Network or Manual, independent of trip mode |
| Set Current Level | Empty, 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, Full |
| Add Fuel (gal) | Enter a positive amount and preview resulting inventory |
| FULL | Set remaining amount to configured capacity |

Manual controls require a configured capacity and Manual source. In Network source,
explain that the displayed contents come from the tank sensor and offer a switch
to Manual instead of accepting edits that the next sensor update would overwrite.
Network values remain visible in Diagnostics even when Manual is selected.

Set Current Level replaces the estimate; it does not add fuel. FULL means the tank
is full **now**, not “add one tank's worth.” Show a concrete before/after preview
and an Apply/Cancel action for every inventory adjustment. Apply must commit once;
repeated taps or recovery cannot add the amount twice.

Examples for an illustrative 60 gal configured tank:

- Set 3/8: remaining becomes 22.5 gal.
- Add 10 gal to 22.5 gal: remaining becomes 32.5 gal.
- FULL at any known or unknown level: remaining becomes 60 gal.

Do not assume a default capacity or start with a full tank on first boot. Manual
remaining starts unknown until Set Current Level or FULL establishes a baseline.
Add Fuel cannot establish total contents from an unknown baseline; prompt for a
level or FULL first. Empty is a valid known zero.

Accept positive whole-US-gallon capacity and addition amounts only; the keypad has
no decimal key and validation rejects fractional inputs. Retain full internal
calculation precision: eighth-tank baselines and consumed/remaining amounts can be
fractional. Proposed display precision is 0.1 gal. Reject blank, non-finite, negative, and zero
capacity/add amounts. If an addition exceeds capacity, do not silently discard
the excess: show the conflict and offer FULL or correction of the entry.

Changing capacity preserves an established remaining volume, not its old fraction.
Recompute percentage using the new capacity. If the new capacity is below remaining
volume, require the user to correct remaining level or explicitly apply FULL with
the new capacity in the same transaction. No silent clipping. With no baseline,
capacity changes leave remaining unknown. Network gallons recompute immediately
from the selected tank percentage and new configured capacity.

## Engine screen and source rules

The fuel card shows **Fuel Remaining**, e.g. `32.5 gal`, without percentage.
Use a compact `EST` or `NET` source indicator plus stale/incomplete styling; these
identify data quality without adding a percentage reading. The requested remaining
amount uses US gallons regardless of the existing display-unit preset; label US
gallons explicitly in configuration. Percentage remains available in configuration
and Diagnostics, not on the Engine fuel card.

For fresh valid network percentage:

```text
remaining gallons = configured capacity gallons × level percent / 100
```

Configured capacity is authoritative for this feature. Network-advertised capacity
is diagnostic information and may be offered as an explicit import; it must not
silently replace the user's setting. Use only the selected fuel tank instance/source,
not another tank/fluid on the network. Reject unavailable or out-of-range percentages;
valid zero means empty.

In Manual source, show the persistent estimate, not the received network percentage.
New network readings cannot overwrite it. In Network source, stale/missing level
displays a stale/unavailable indication; no automatic fallback or source switch.
A source change does not imply refueling, reset a
trip, or automatically replace the stored manual baseline. Warn if the retained
Manual estimate is uninitialized or incomplete before selecting it.

## Consumption and quality

Use the selected engine's fresh fuel rate to subtract fuel from manual inventory:

```text
fuel used liters = integral of fuel rate liters/hour over valid intervals
remaining liters = max(0, previous remaining liters − fuel used liters)
```

Keep liters internally and convert at input/display boundaries. Under the US gallon
definition, one gallon is 3.785411784 liters. Setting 1/8 means capacity multiplied
by 1/8; never repeatedly round stored contents to eighths or displayed tenths.

Fuel depletion runs independently of the open page and trip mode. A shared valid
fuel-consumption increment can feed both tank inventory and the active trip; never
subtract an entire cumulative trip total or subtract the same increment twice.
Resetting a trip or switching modes leaves tank inventory unchanged. Refueling or
correcting the tank does not alter historical trip fuel-used totals.

Proposed behavior: maintain an initialized Manual estimate in the background even
when Network source is selected, subtracting measured engine fuel consumption.
This keeps the estimate useful on a later explicit source switch. Refueling still
requires an explicit Manual adjustment; sensor increases do not imply an addition.
The estimate can therefore differ from the sensor and must be labeled accordingly.

Integrate only bounded valid intervals, with no bridging across boot or missing/stale
flow. Fresh zero flow is valid and causes no depletion. If flow becomes unavailable,
freeze the estimate and mark it incomplete; retain that flag after data resumes
until an explicit level/FULL correction establishes a new baseline. The gauge cannot
account for fuel consumed while powered off, leaks, or unrecorded refueling. After
an unobserved power interval, flag that the restored estimate may need verification.

Clamp the displayed estimate at zero and flag further measured consumption as an
inventory inconsistency. Zero estimated fuel does not mean the engine must stop.
No new audible alarm thresholds are introduced by this feature.

## Persistence and summaries

Persist capacity, source preference, selected tank identity, manual remaining liters,
baseline identity/time, consumed-since-baseline, quality flags, and adjustment sequence.
Store configuration only on change and inventory using versioned checksummed
checkpoints coordinated with the trip store. Apply capacity/level/add/FULL changes
as durable idempotent transactions; commit before showing success.

Use the trip feature's accepted maximum 30-second recent-data loss window during
normal storage operation. Inventory adjustment commits are immediate. After recovery,
never replay an addition twice or debit a consumption increment twice. Preserve
inventory through OTA and prevent reset defaults from implying a full tank.
SD failure cannot prevent maintaining the internal inventory estimate.

Keep the already agreed summary-only trip export scope: no sample/position logs.
Proposed additional summary fields are fuel source, start/end remaining liters and
their quality, capacity at start/end, and total manual additions during the record.
Tank adjustments are stored internally for recovery; no new public event-log file
is required. Attribute additions to the selected trip mode only. Snapshot tank
state when a trip closes without changing tank contents.

## Modules and acceptance

Add `FuelInventoryService` alongside TripService. It owns configured capacity and
manual inventory, consumes normalized fuel-rate increments, and publishes immutable
UI snapshots. UI callbacks send validated commands; they do not integrate fuel or
write storage directly. Raw network fields remain distinct from configured capacity,
manual contents, and selected display values.

| Scenario | Required result |
| --- | --- |
| First boot, no configuration | No invented capacity or remaining volume |
| Capacity 60 gal, network level 50% | 30 gal using configured capacity |
| Set each eighth including Empty/Full | Correct replacement volume, no additive behavior |
| Add 10 gal to 22.5 gal; repeat tap/reboot | Exactly 32.5 gal; one committed addition |
| FULL from unknown or partial level | Remaining equals configured capacity |
| Unknown baseline, invalid entry, excess addition | Clear corrective UI; no silent inventory mutation |
| Fractional capacity/addition entry | Rejected; fractional computed remaining gallons retained |
| Engine fuel card in any unit preset | Remaining US gallons only; no percentage; visible source/quality |
| Capacity edit above/below remaining volume | Preserve volume or require explicit reconciliation |
| Fresh flow consumes 2 gal | Manual inventory decreases by 2 gal; trip fuel stays independent |
| Stale/absent flow or gauge-off interval | No fabricated depletion; incomplete estimate visible |
| Network data returns while Manual selected | Manual contents unchanged; network visible diagnostically |
| Midnight/reset/mode change/OTA/restart | Tank state retained; no fabricated refueling or repeated debit |
| Missing SD or interrupted adjustment write | Gauge operates; recover last valid transaction and report errors |

Validate formulas and idempotent recovery with deterministic host tests. Verify
touch/keypad fit on device, network tank selection, fuel-flow availability and
freshness on the boat, and visual distinction between sensor data and estimates.
