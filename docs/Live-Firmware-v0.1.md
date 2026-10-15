# Live Gauge v0.1.2

The first real-data gauge firmware for the Waveshare ESP32-S3-Touch-LCD-5
800x480. The `live-gauge` target is separate from the scanner and simulator;
no simulated values enter this build. Version: `firmware/include/LiveModel.h`.

## Build and install

From `firmware/`:

```console
pio run -e live-gauge
pio run -e live-gauge -t upload --upload-port COM6
```

Confirm the connected USB port before uploading. The default build is now
`live-gauge`; `simulated-gauge` and `nmea-scanner` remain explicit alternatives.
The resulting application is `.pio/build/live-gauge/firmware.bin`; use PlatformIO
upload to install the application with its bootloader and partition table.

This release is an initial boat-test version. Existing CAN bus errors remain
unresolved. A successful build or replay does not validate the physical bus,
LCD/touch operation, wiring, engine alarm interpretation, or safe underway use.

## Display

- Five-second navy/cyan animated startup screen, with the running firmware version.
  Timing starts after the first LVGL frame is submitted. CAN reception starts
  during the presentation. Initialization failures replace the splash; selected
  standard engine critical-status bits end the splash early.
- Engine: tachometer, GPS speed, trim, fuel flow, coolant temperature, oil pressure,
  true course, network fuel remaining in US gallons, charging voltage, engine hours, economy.
- Performance: current speed/RPM/flow/economy, sea-water temperature, session best
  economy after at least five seconds of steady RPM and speed with healthy CAN.
- Trip: valid running minutes, distance, fuel used, average economy, maximum
  speed/RPM, two-tap reset within five seconds. Totals are session-only.
- Diagnostics: firmware, selected source addresses/engine instance, received frame
  and complete engine-message counts, packet drops, CAN state/errors/losses,
  REC/TEC, raw-log on/off state and drops/write failures, raw status words,
  coordinates and uptime.
- Settings: US, marine or metric units; day/night colors. These two settings are
  saved in internal flash. Tank Capacity opens a numeric keypad with Save/Cancel;
  positive whole values from 1 to 9999 US gallons are saved independently.
  Set / Reset Fuel Amount accepts zero through capacity and includes FULL.
  No capacity or initial contents are assumed on first boot. Redetect Sources uses two-tap confirmation and also
  clears trip data so totals cannot combine different engines/GPS sources.

`--` means not received or a derived value cannot yet be calculated. `N/A` means
an encoded unavailable/error/reserved value or an invalid physical range.
`STALE` means an available received value is over five seconds old. Zero RPM,
speed, trim, and flow remain valid zeros. Magnetic COG is withheld because no
magnetic-variation correction is implemented. All gallons are US gallons.
Night mode changes colors, not the physical backlight brightness.

## Data and source selection

| PGN | Fields used |
|---|---|
| 127488 | Engine instance, RPM, signed trim |
| 127489 | Oil pressure/temperature, coolant temperature, voltage, fuel flow, engine hours, load/torque and raw status words |
| 129025 | Latitude, longitude |
| 129026 | GPS speed over ground and true course |
| 127505 | Fuel-tank level percentage and capacity |
| 130310 / 130312 | Sea-water temperature (130312 source type 0 only) |

Oil temperature, load/torque and tank capacity are retained by the decoder but
are not assigned gauge tiles in v0.1. Not every field is transmitted by this boat;
for example the saved engine capture encodes oil temperature as unavailable.
Settings > Tank Capacity saves the tank size in whole US gallons. Settings >
Set / Reset Fuel Amount sets current contents from zero through capacity and
provides FULL. Both use a numeric keypad with explicit Save/Cancel. Zero means
known empty. Capacity alone does not initialize contents.

Fresh valid network tank percentage overrides the manual amount and is labeled
NET. Engine fuel flow maintains an independent estimate between network updates.
If network levels stop, the estimate continues from the latest baseline and shows
EST; missing consumption coverage is marked EST?. The Engine fuel card uses US
gallons under every unit preset. It remains unknown without a network or manual
baseline. Invalid/unavailable levels never override known contents. Network-
advertised capacity never replaces the user's configuration.

Manual changes save immediately as one capacity/amount record. Changed running
inventory is checkpointed every 30 seconds; abrupt loss of power may lose the
latest uncheckpointed consumption. Recovered estimates are marked uncertain on
restart. Trips and GPS are independent from inventory; trip reset/source
redetection preserve it. See [fuel behavior and acceptance](Fuel-Configuration.md).
Dedicated Add Fuel and eighth-tank shortcuts remain future work.

The first engine source/instance, GPS source, and fuel tank source/instance are
selected until restart or manual redetection. Addresses are discovered rather
than hardcoded to the test capture's engine SA 2 / instance 1 and GPS SA 0.
If equipment changes address, use Redetect Sources. This first release does not
track device NAME/address claims or provide multi-engine source-selection menus.

Only PGN 127489 is reassembled as a fast packet. Eight bounded receive sessions
are keyed by complete CAN identifier and sequence, require ordered fragments,
26-byte length, and a maximum 250 ms gap. Missing, duplicate, wrong-sequence,
truncated, expired and queue-loss-affected packets are rejected. Other proprietary
or fast-packet PGNs remain available as raw frames for later offline analysis.
Proprietary Yamaha/Garmin values are never presented as decoded engine sensors.

Status words are shown raw. Word 1 flags and the low five standard bits of word 2
produce a generic visual OEM-status warning; high reserved/unsupported word-2
bits such as the captured `0xFFE0` do not create a shutdown warning. Word-1 bits
1 through 4 take precedence over the startup presentation. These indications
are provisional; retain the Yamaha instruments as the authority for alarms.
There are no threshold alarms, alarm acknowledgement/silencing, or audible output.

## Receive and trip behavior

CAN is 250 kbit/s **listen only**, GPIO15 TX / GPIO16 RX. There are no transmissions
or address claims. A dedicated high-priority receive task drains the 128-frame
controller queue independently from display refresh and USB output. This reduces
software contention; it does not establish the cause or fix the physical CAN
errors seen in earlier tests.

Diagnostics report cumulative controller errors, missed frames and overruns.
Any increase or non-running state marks CAN unhealthy until five quiet seconds
have elapsed. Unavailable/old status samples also pause trip integration.
REC/TEC zero alone never establishes CAN health. Recent errors remain visible
in the banner even when some usable measurements continue to arrive.

Trip integration requires fresh, valid RPM above zero, GPS speed and fuel flow,
healthy CAN, and adjacent samples no more than one second apart. Distance and
fuel use trapezoidal integration over the same intervals. Missing coverage is
marked incomplete and is not filled by extrapolation. Maxima use fresh running
measurements. Instant economy is withheld below 1 m/s, at zero RPM, or at fuel
flow at or below 0.3 L/h. Session best requires speed above 1 m/s, flow above
0.3 L/h, RPM within 100 and speed within 0.5 m/s of the window's initial values.
It is an observed session sample, not a calibrated cruise recommendation.

## Raw USB capture

Open native USB serial at 115200, send `i` for metadata, `r` to start raw logging,
and `s` to stop. Raw logging is off at boot and requires an attached host to save
the stream to a file. This target does not write to microSD. Disconnecting USB
never blocks CAN reception or the display.

Records use the existing scanner-diag-3 numbered/checksummed envelope and
`RAW seq=... us=... id=... dlc=... data=...` payload, including proprietary frames.
`BOOT` identifies `live-gauge-0.1.2`; `LIVE` reports diagnostics every five seconds.
The `us` field is 64-bit uptime in microseconds, avoiding the scanner's 71-minute
microsecond-counter rollover during long captures.
The raw sequence covers frames while logging is enabled; gaps and the cumulative
`rawDrops` counter expose queue loss. Failed/partial writes are counted; record
sequence/checksum checks expose damage. Queues are bounded (512 raw frames) and
USB writes have record and batch deadlines. The logger never waits in the CAN task.

```console
python ../tools/validate_scanner_log.py path-to-capture.log
```

Keep the metadata, final `LIVE` diagnostic record and raw frames when saving a
capture. Valid serial records cannot recover frames rejected or missed on CAN.
USB throughput under heavy traffic still requires boat testing.

## Tank capacity acceptance checks

After installing v0.1.2, open Settings > Tank Capacity, enter the actual capacity
in whole US gallons, and tap Save. Restart and check the saved value. Editing and
Cancel (or navigating away without Save) must retain the previously saved size.
Blank/zero inputs must remain on the editor with an error. A trip reset or source
redetection must not change capacity. Display units must not change its US-gallon
meaning. Set a fuel amount without a sender and confirm EST. A fresh network level must
override it; unplugging the sender must continue EST from the latest baseline.
Confirm reset to zero, FULL, invalid amount rejection, trip-reset independence,
and checkpoint/uncertainty behavior described in the fuel guide.

## Release validation

v0.1.2, October 15, 2026: final live build passed (74,352 bytes static RAM,
884,546 bytes application flash). Fuel inventory tests passed for initialization,
reset/empty/full, integration, network priority, fallback from the latest network
baseline, invalid data, capacity changes, persisted record validation, gaps and
clock rollover. The live-model tests and full saved boat capture replay passed.
New label widths were checked against actual font metrics. v0.1.2 was uploaded
without rebuilding; all flashed images passed hash verification, and serial
confirmed firmware v0.1.2 running with CAN initialized. No NMEA traffic was
received during that check. Hardware Save/Cancel, saved-value restart recovery
and live-sender checks remain pending.

v0.1.1, October 15, 2026: build passed (74,280 bytes static RAM, 881,042 bytes
application flash). Capacity parsing, invalid/overflow inputs, unknown capacity,
empty/full/partial levels, timestamp preservation and stale/unavailable behavior
passed portable tests. The full 5,418-frame boat capture replay also passed.
Label widths were checked against the display font metrics. v0.1.1 has not been
uploaded; keypad operation and saved-capacity recovery after restart still need
on-device validation.

Original v0.1.0 validation, October 15, 2026:

- `live-gauge`, `nmea-scanner` and `simulated-gauge`: build passed.
- Final live image: 74,264 bytes static RAM (22.7%); 878,514 bytes application
  flash (26.3% of the configured application partition).
- Portable live-model tests and the complete saved boat-capture replay: passed.
- Existing scanner parser, diagnostics, logging transport and log-validator
  regression tests: passed.
- Host C++ tests compiled with C++17, warnings enabled and treated as errors.
- v0.1.0 was subsequently uploaded and reported its firmware version over USB.
  No NMEA traffic was observed in that check; visual/live-boat checks are pending.

Host tests exercise decoding/scaling, fast-packet isolation and loss, sentinels,
source selection, freshness including clock rollover, trip integration/gaps,
zero-RPM operation, and conservative warning handling. A separate optional
replay checks the saved one-minute engine/Garmin capture (5,418 frames, 120
complete engine-dynamic messages, coolant 46-48 C and maximum 1,445.25 RPM).
Run the portable tests with `bash firmware/test/host/run.sh`; compile
`live_model_test.cpp` with a C++17 compiler and pass the saved capture filename
as its optional first argument to reproduce the replay.

Before release approval, upload and verify the actual screen/touch navigation,
five-second timing and repeated restarts, connection loss/recovery, source
redetection, saved units/theme, raw capture integrity, USB-disconnected operation,
and engine/GPS comparison against the original instruments. Those hardware tests
have not been performed on this build.

Deferred: persistent trips, dedicated Add Fuel/eighth-tank controls, SD storage, Wi-Fi/OTA,
buzzer, automatic screen sleep/wake and verified Yamaha proprietary decoding. These remain future milestones.
