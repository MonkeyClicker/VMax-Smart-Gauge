# Fuel Capacity and Starting Amount

Updated user direction: October 15, 2026. **Network readings override the manually
entered starting amount.** This supersedes the earlier proposed separate
Network/Manual selector and the rule that network data must not change a manual
estimate.

## Live Gauge v0.1.2

Settings provides two numeric-keypad screens:

- Tank Capacity: positive whole US gallons, 1-9999, with Save/Cancel.
- Set / Reset Fuel Amount: whole US gallons from zero to capacity, with Save/Cancel
  and a FULL shortcut. This replaces current contents; it does not add gallons.

Configure capacity before setting fuel amount. No default size or full-tank
assumption is made. Zero is a known empty tank. Values above capacity, blank,
negative and fractional entries are rejected. Leaving without Save cancels edits.
Saving an amount does not reset trip totals, engine hours, or source selection.
Trip reset and source redetection leave the fuel inventory intact.

## Network priority

A fresh, valid level from the selected fuel tank is authoritative:

```
remaining US gallons = configured capacity US gallons * network percentage / 100
```

This applies even immediately after a manual amount reset, including FULL. A valid
zero-percent reading overrides the manual amount with empty. Encoded unavailable,
invalid, out-of-range or stale network values do not override an estimate.
Network-advertised capacity does not replace configured capacity.

Each new network measurement also establishes the estimate's latest baseline.
Measured engine fuel flow is subtracted between measurements. While the network
level is fresh (up to five seconds), the Engine card shows its reported amount
with NET. When it becomes stale/unavailable, the card uses the latest baseline
minus observed fuel consumption and shows EST; it never restores an older manual
starting value. Without either a manual baseline or valid network level, remaining
fuel stays unknown.

The Engine card displays US gallons under every display-unit preset. Diagnostics
retains the raw network percentage and identifies the active fuel source.

## Consumption and uncertainty

Inventory tracking runs on every gauge update, including boot and all pages,
independently of trip calculations and GPS reception. It integrates selected-engine
fuel flow trapezoidally over adjacent intervals no longer than one second. CAN
must be healthy and flow fresh/valid. Fresh zero flow is valid; a fresh stopped
engine can establish zero consumption when flow is absent. Negative flow is
rejected. Inventory is bounded below by zero.

On missing/unreliable inputs or a long update gap, consumption pauses and the
estimate is marked EST?. With otherwise live inputs, the status banner also
asks the user to confirm/reset the fuel amount. Resumed data does not fill a missed interval or clear
that uncertainty. A new authoritative network level or an explicit manual reset
can establish a new baseline. NET identifies fresh reported contents, not proof
that every historical flow sample was captured.

Changing capacity preserves an existing estimated volume. If a proposed capacity
is smaller than that amount, first reset fuel contents to a valid quantity. With
a fresh network percentage, changing capacity recalculates the authoritative
volume immediately from the new capacity.

## Persistence

Capacity and amount are saved as one versioned, validated record in internal
flash. Save commits immediately and reports a failure without accepting the edit
if storage fails. v0.1.1's capacity setting is imported when no inventory record
is present. Invalid records cannot manufacture known fuel contents.

Changed running inventory is checkpointed at most once every 30 seconds, reducing
flash writes. Abrupt power loss may lose changes since the last checkpoint. On
restart, a recovered estimate is marked EST? because consumption while the gauge
was off cannot be reconstructed; a fresh network level clears that uncertainty.
Checkpoint write failures are shown in Diagnostics. Normal trip resets never
clear inventory or capacity.

## Validation

Portable tests cover amount validation, zero/full/partial values, integration,
network priority (including reset while a fresh reading is cached), network
loss and recovery, unavailable/invalid values, capacity changes, restore/corrupt
records, CAN faults, time gaps, negative flow, and timer rollover. Tests passed.
Physical keypad, Save/Cancel, restart recovery, and comparison with actual fuel
instruments remain on-device acceptance checks.

Dedicated Add Fuel and eighth-tank shortcuts remain future features. The current
Set / Reset Fuel Amount control can replace the current baseline after refueling.
