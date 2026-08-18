# Power Design

## Current Strategy

The prototype gauge will use the regulated USB output of the LiFePO4 battery system that already powers the Garmin. The NMEA 2000 connection is used for network communication, not as the primary gauge power source.

## Prototype Arrangement

```text
LiFePO4 battery
├── Garmin 106sv
└── USB -> Waveshare ESP32-S3-Touch-LCD-5

NMEA 2000 backbone
└── drop -> Waveshare CAN interface
```

## Design Notes

- The gauge should not add another NMEA 2000 network power feed.
- The existing backbone remains responsible for NMEA 2000 power and termination.
- The prototype uses the Waveshare onboard CAN transceiver.
- During development, the gauge may be powered from a laptop USB connection for flashing and serial diagnostics.
- Because the onboard CAN interface is not galvanically isolated, electrical isolation should be revisited before finalizing a permanent marine installation.

## USB Capacity

Use a stable 5 V USB source with adequate margin for the ESP32-S3, display backlight, touch controller, CAN interface, Wi-Fi during OTA, and microSD writes. A USB source capable of at least 1 A is the target, with 2 A or more preferred for margin.

## Firmware Power Behavior

The firmware should tolerate the gauge starting before the engine, the engine starting after the gauge, temporary NMEA loss, unexpected USB removal, and restart after a brownout. Trip state should be checkpointed periodically so an unexpected power loss does not invalidate the whole trip log.

## Future Review

After the prototype is proven, evaluate a more robust permanent-installation power and CAN front end, including isolation, transient protection, switched power behavior, and recovery from abnormal power events.
