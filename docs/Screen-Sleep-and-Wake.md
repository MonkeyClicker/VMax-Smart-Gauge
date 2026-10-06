# Screen Sleep and Automatic Wake

Feature requirements confirmed: 2026-10-06. Documentation only; firmware behavior
is unchanged. Hardware operation and technical proposals still require validation.

## Feature requirements

- Turn the screen off after five minutes without engine data or touch activity.
- Automatically wake when engine data resumes, including when the engine starts again.
- Support touch wake if the display hardware permits it. After touch wake, apply
  the same five-minute inactivity rule if engine data remains absent.
- Keep receiving data, maintaining trip/fuel state, and logging while the screen is off.

Confirmed: selected-engine data keeps the screen awake even at zero RPM. Sleep
requires five minutes with neither engine data nor touch. The first touch while
asleep wakes only; the user must release and touch again to operate a control.
No unanswered user preference questions remain.

## Display standby rather than processor sleep

Implement screen sleep as **backlight off**, with the CPU, CAN receiver, touch
controller and application services operating. Keep the LCD initialized for fast
wake. Deep sleep is not part of this feature: normal CAN/touch processing must
continue, and trip recording must not gain artificial gaps from screen inactivity.

Repository evidence supports this approach: the setup notes document independent
LCD backlight control on CH422G EXIO2, while `LvglPort.cpp` polls the touch controller
through `readPoints`. The Board abstraction exposes a backlight object. This makes
touch wake a plausible software change without processor wake hardware, but it is
not validated until tested with the backlight off on the actual board. Turning off
the backlight reduces display power; no whole-gauge power-saving figure is claimed.

Use the existing board driver rather than reinitializing the expander or toggling
unrelated shared outputs. Preserve touch reset, LCD reset, SD select and USB routing.
Do not stop the LVGL timer handler if it is still responsible for polling touch.

## Exact inactivity rule

Maintain separate monotonic timestamps for last qualifying engine message and last
physical touch activity. Initialize both to boot time for an initial five-minute
visible period. Sleep only when **both** ages are at least 300,000 ms and no explicit
stay-awake reason is active. Use wrap-safe timer arithmetic, independent of the
calendar clock and Automatic trip midnight rollover.

Qualifying engine activity is a successfully decoded engine message for the selected
engine identity, such as its rapid or dynamic parameters. Invalid/malformed frames,
messages for a different engine, GPS, tank, and other network traffic do not count.
Do not count cached BoatData refreshes or repeatedly displayed stale values as new
engine reception. A decoded selected-engine message with unavailable measurement
fields still establishes communication; field freshness remains separate. Handle
source-address changes through normal engine identity discovery.

Under the confirmed data-based rule, fresh engine messages at zero RPM keep the
screen awake. Wake is triggered by received engine data, not solely a positive RPM
reading. If the engine stops but continues publishing messages, the screen stays
on.

Every genuine touch counts, including touches on empty space and Settings controls.
Track raw input activity rather than only successful button callbacks. A held finger
keeps the display awake until five minutes after its last detected contact. Ignore
synthetic UI updates and serial test commands as physical touch activity.

| State/event | Result |
| --- | --- |
| Boot, no engine messages | Screen visible; sleep after five minutes without touch |
| Engine messages continue | Screen stays visible |
| Last engine message at 10:00, touch at 10:04 | Sleep at 10:09 if no further activity |
| Screen asleep, selected-engine data resumes | Wake automatically and restart engine inactivity timing |
| Screen asleep, touch detected | Wake and restart touch inactivity timing |
| Wake by touch, no further engine data/touch | Sleep again after five minutes |
| GPS/tank traffic only | Does not prevent sleep or wake the screen |

## Wake presentation and touch handling

Restore the current page with fresh readings/status and the applicable day/night
brightness preference. Do not reset navigation, trip totals, engine-running time,
tank inventory, alarm acknowledgement, or user preferences on sleep/wake.

Confirmed first-touch behavior: consume the entire wake gesture, including release,
so it cannot reset a trip, add fuel, acknowledge an alarm, or start a service. A new
touch after release operates normally. Suppress input before forwarding it to LVGL,
including pending press/release state when sleep starts. If engine data wakes the
screen while a wake touch is held, finish consuming that gesture consistently.

Cancel temporary two-tap confirmations when entering sleep, including an armed
Trip Reset, so wake cannot complete an old confirmation. Preserve unsaved form
entries where feasible without treating sleep as Apply or Cancel.

Proposed wake responsiveness target: visible within 500 ms of qualifying data or
touch detection. Measure on-device; no reset/reboot is required to wake.

## Service and warning behavior

Proposed exceptions: keep the screen visible during an active firmware update,
active log transfer, or an active Warning/Critical alarm. Wake when a new Warning
or Critical alarm requires presentation; screen sleep cannot silence audible alarms.
Touch feedback and advisories do not independently wake the screen.

Do not treat simply opening Settings, an idle download page, or old alarm history as
activity. Explicit stay-awake reasons must have bounded service lifetimes. When the
last reason clears, grant a new five-minute grace period; do not immediately blank
the screen because its old activity timestamps expired during the operation.

## Settings and diagnostics

Five minutes is the fixed requested timeout. No configurable timeout or new toggle
is required for the initial feature. Add explanatory text in Settings/System:
“Screen turns off after 5 minutes without engine data or touch. Engine data or a
touch wakes it.” This describes display standby, not shutting down the gauge.

Diagnostics shows screen state, engine/touch activity ages, last wake reason,
active stay-awake reason, and backlight/touch errors. Screen state and activity
timestamps are runtime state, not persisted preferences; every boot starts awake.

If backlight-off fails, retain an awake/error state and retry in a bounded manner.
If wake fails, continue CAN/logging and expose a diagnostic error. If touch wake
cannot be validated, retain automatic engine-data wake and report the limitation
before release; do not advertise touch wake as working without a device test.

## Implementation boundaries

Add `ScreenPowerService` with Awake/Asleep states and activity inputs from selected
engine reception and the raw touch path. Keep hardware actions in the board adapter
and serialize LVGL work under the existing lock. Do not add blocking waits in CAN
or touch callbacks. Serialize timeout, engine reception and touch events so activity
arriving at the timeout boundary wins and avoids off/on flicker.

TripService and FuelInventoryService continue independently of screen state.
Engine-running time still uses fresh running status; a touch or wake event never
adds running time. Use a separate explicit simulator engine-activity signal for
bench testing; synthesized UI redraws must not represent real network reception.

## Acceptance and validation

| Scenario | Required result |
| --- | --- |
| Boot without engine data/touch | Backlight off at five minutes within scheduling tolerance |
| Continuous selected-engine messages, including zero RPM | Stay awake under the data-based rule |
| Other engine or navigation/tank traffic | Does not delay sleep or cause wake |
| Touch at 4:59, including empty space | Timeout extends five minutes from touch |
| Engine data returns while dark | Automatic wake with current page/totals retained |
| Touch while dark | Wake only; entire first gesture consumed; next touch operates controls |
| Touch wake, then no data/touch | Dark again after five minutes |
| Data/touch arrives at timeout boundary | Remains/wakes visibly without repeated transitions |
| Sleep on Trip/Fuel Configuration | No reset/refueling; pending confirmation cannot activate on wake |
| Screen dark during normal recording | CAN, trip/fuel integration, logging and alarms remain operational |
| OTA/transfer/alarm exception clears | New five-minute grace period, then normal timeout |
| Day/night change, calendar jump, timer wrap, repeated wake | Correct brightness; monotonic timing; no memory/resource drift |

Use deterministic state-machine tests for timeout boundaries, timer wrap and event
ordering. Test real backlight toggling and touch detection while dark on the device,
then engine stop/start on the boat. Verify sustained data capture and log durability
across repeated display sleep cycles. Hardware touch-wake feasibility and warning/
service exceptions require validation before release.
