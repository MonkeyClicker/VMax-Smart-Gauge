# UI Design

## Design Goal

Create a single-engine marine gauge optimized for quick reading at the helm. The interface is inspired by the usability of Yamaha multifunction gauges but should use an original layout and visual treatment.

The initial product is intentionally a single-engine display for the 2024 Yamaha V MAX SHO 250.

## Display Platform

- 5-inch touchscreen
- 800x480 resolution
- capacitive touch
- landscape orientation

## Main Screen Priorities

RPM is the dominant value. The remaining primary values should be visible without changing screens.

Recommended home-screen fields:

- RPM
- GPS speed
- trim
- fuel flow
- engine temperature
- fuel level
- voltage
- engine hours
- network/status indicator

Calculated economy should also be available prominently when underway.

## Suggested Main Layout

```text
+------------------------------------------------------+
| V MAX SMART GAUGE                  NMEA OK    14.2 V |
+------------------------------+-----------------------+
|                              | SPEED          38.4   |
|          RPM                 | MPH                   |
|                              +-----------------------+
|          4250                | TRIM             32%  |
|                              +-----------------------+
|   large graphical tach      | FLOW          11.6GPH |
|                              +-----------------------+
|                              | TEMP            154F  |
+------------------------------+-----------------------+
| FUEL 68% | HOURS 286.7 | ECON 3.31 MPG | STATUS OK  |
+------------------------------------------------------+
```

## Interaction Model

Primary navigation should remain simple and usable in motion. Planned pages:

1. Home / Engine
2. Performance
3. Trip
4. Diagnostics
5. Settings / System

The home screen should require no touch interaction for normal operation.

## Performance Screen

Show values useful for prop and cruise analysis:

- RPM
- speed
- fuel flow
- MPG
- trim
- current versus best observed economy

Potential table:

```text
RPM     MPH     GPH     MPG     TRIM
3500    28.7    8.1     3.54    24%
3750    31.9    8.8     3.63    28%
4000    35.1    9.5     3.69    31%
```

## Trip Screen

Show:

- elapsed trip time
- distance
- fuel used
- average MPG
- maximum speed
- maximum RPM
- best cruise economy

## Diagnostics Screen

Show:

- CAN/NMEA state
- firmware version
- engine source address
- engine instance
- detected PGNs
- PGN message counts
- approximate update rates
- SD status
- Wi-Fi/OTA status
- data age/staleness information
- buzzer DO0 commanded state
- active alarm priority/pattern phase
- alarm acknowledgement and silence timer
- recent alarm history and source PGN

## Settings Screen

Planned settings:

- MPH / knots
- Fahrenheit / Celsius
- gallons / liters
- day brightness
- night brightness
- automatic day/night mode
- logging options
- OTA service mode
- engine source/instance selection if needed
- master audible alarms
- touch-feedback chirp
- night quiet-mode behavior
- alarm silence duration
- provisional alarm thresholds
- dockside buzzer-pattern test

## Stale and Missing Data

The UI must distinguish three cases:

- valid zero
- unavailable/not transmitted
- stale/disconnected

Examples:

- `0 MPH` is valid while stationary
- `--.- GPH` may indicate unavailable data
- stale engine values should visibly change state rather than remain frozen

## Warning Presentation

Engine warning/status data should be visually distinct from ordinary UI notices. The display must not imply that a warning has been validated until its corresponding NMEA status behavior has been confirmed on the boat.

| Priority | Audible pattern | Visual treatment |
| --- | --- | --- |
| Feedback | One 75 ms chirp | Normal touch feedback |
| Advisory | 250 ms on, 1750 ms off | Amber banner/icon |
| Warning | Three 250 ms pulses, repeated every 3 seconds | Red warning banner |
| Critical | Continuous, with a 100 ms gap every 2 seconds | Full red critical overlay |

Touching a warning opens its detail panel. A Silence action temporarily stops its sound but leaves the warning visible. A new higher-priority condition overrides silence. An acknowledged condition sounds again if it clears and later returns.

See [Audible Warning System](Audible-Warnings.md) for complete priority, persistence, and validation rules.

## Day/Night Behavior

Day mode should prioritize sunlight readability. Night mode should substantially reduce backlight and bright screen area to protect night vision. Manual override should always be available. Quiet mode may suppress feedback and selected advisories, but must not silently suppress validated critical alarms.

## Mockup Direction

The current preferred mockup direction is a single large tachometer with stacked engine metrics on the right and a compact status strip along the bottom. Multi-engine layouts are explicitly out of scope for the first version.

## Multi-page simulated prototype

The simulated-gauge target implements the five planned pages with fixed, large
bottom touch tabs. A shared top bar identifies the current page and labels all
input as simulated. A persistent status banner opens Diagnostics, including a
visual acknowledgement control for the explicit demo warning scenario.

- Engine retains the large tachometer and puts primary engine/navigation values
  beside it, with fuel, voltage, hours, and economy in the bottom summary strip.
- Performance shows current readings plus best observed cruise economy and a
  six-band RPM table. These are generated observations, not prop recommendations.
- Trip integrates speed and flow over time. Its two-tap reset expires after five
  seconds and preserves lifetime hours and remaining tank level.
- Diagnostics distinguishes generated input from the inactive CAN, SD, Wi-Fi,
  OTA, and buzzer services. It also shows real device uptime/memory and UI events.
- Settings selects US, marine, or metric units, a day/night palette, and explicit
  normal, warning, offline, or unavailable-tank scenarios.

Offline inputs appear as `STALE` rather than frozen current values, while historical
trip totals remain visible. Distance and fuel accumulation pause across input gaps.
Unavailable tank level appears as `--`; a valid stationary speed remains zero.

Only the active page's objects exist at a time to bound LVGL memory use. All
creation, navigation, and updates use the existing LVGL lock. Hardware initialization
and the NMEA scanner target are unchanged. This prototype does not persist settings
or trip data, change physical backlight brightness, or activate connected services.

On 2026-10-05, the multi-page prototype built and uploaded successfully, passed
on-device simulator and label-layout checks with stable memory, and was confirmed
by the user to look good and respond to touch navigation.
