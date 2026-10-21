# Feature Request: Startup Boot Screen

Requested: October 15, 2026.
Status: Implemented in the `live-gauge` v0.1.0 target; on-device acceptance checks pending.
See [live firmware guide](Live-Firmware-v0.1.md).
Tracking: [GitHub issue #29](https://github.com/MonkeyClicker/VMax-Smart-Gauge/issues/29).

## Requested behavior

Display a cool-looking boot screen for five seconds on startup, including the
running firmware version, before automatically opening the Engine page.

## Visual direction

- Match the approved V MAX navy/cyan visual theme on the 800x480 landscape display.
- Feature V MAX SMART GAUGE branding and a polished turbine or segmented-ring animation.
- Show a clearly readable version label such as `Firmware v1.0.0`.
- Populate the version from the running firmware's build metadata; the example
  version is illustrative, not an assigned release number.
- Use subtle motion and a clean composition without suggesting that animation
  proves network reception or that initialization has a measured percentage complete.

## Acceptance criteria

1. Normal power-on and restart display the boot screen for five seconds measured
   from its first visible frame, then enter the Engine page without touch.
2. The version displayed matches the running firmware's diagnostic version.
3. Initialization and CAN reception continue during the presentation. Implement
   the interval without a blocking five-second delay; retain received data for
   the first Engine-page update and preserve freshness/unavailable states.
4. Startup faults or critical warnings take precedence over the presentation;
   the splash must not conceal a failure indefinitely or delay a critical warning.
5. Visual verification on the Waveshare confirms readable text, smooth animation,
   no clipping/flicker, and reliable transitions through repeated restarts.

The first implementation should use the existing verified display stack. This
request does not require Wi-Fi, an internet connection, or extra hardware.
