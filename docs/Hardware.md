# Hardware

## Selected Main Board

Waveshare ESP32-S3-Touch-LCD-5, 800x480 capacitive-touch version.

Key characteristics from Waveshare documentation:
- ESP32-S3-WROOM-1-N16R8
- 16 MB flash
- 8 MB PSRAM
- 5 inch 800x480 IPS RGB LCD
- capacitive touch
- microSD/TF slot
- Wi-Fi/BLE
- onboard CAN interface
- onboard CAN termination switch, disabled by default
- USB-C 5 V power/programming
- optional 7-36 V DC input
- two isolated 5-36 V open-drain digital outputs

The board does not include a buzzer, speaker, or audio amplifier.

## CAN Pins

Waveshare documents:

- GPIO15 = CAN TX
- GPIO16 = CAN RX

The board includes the physical CAN transceiver, so an additional MCP2515 or external CAN controller is not required for the prototype.

## Audible Warning Hardware

Add an external waterproof or panel-sealed 5-12 V active piezo buzzer controlled by isolated digital output DO0. The selected buzzer should draw less than 100 mA and be loud enough to hear at the helm during an underway test.

DO1 remains available as a spare. The buzzer is an externally powered load and must not be connected directly to an ESP32 GPIO. See [Audible Warning System](Audible-Warnings.md) and [Prototype Wiring Design](Wiring.md) for switching, protection, patterns, and validation.

## Boat Power

Prototype power source:

```text
LiFePO4 battery USB output
          │
          │ regulated 5 V USB
          ▼
Waveshare USB-C
```

The display is not initially powered from NMEA NET-S.

## NMEA 2000 Connection

The gauge connects to the existing spare NMEA 2000 drop point.

```text
Existing NMEA 2000 T
        │
        │ Micro-C drop/pigtail
        ▼

NET-H ───────── CAN-H
NET-L ───────── CAN-L
NET-C ───────── CAN reference / ground
NET-S ───────── not used for prototype power
Shield ──────── enclosure/EMC decision deferred
```

### Termination

The gauge is a drop device. The Waveshare CAN termination resistor must remain disabled because the existing NMEA 2000 backbone should already be terminated at both backbone ends.

## Prototype vs Permanent Installation

### Prototype

Use:
- Waveshare onboard CAN transceiver
- USB battery power
- short NMEA 2000 drop cable
- active piezo buzzer on DO0 after dockside electrical validation

### Permanent Marine Version

Evaluate:
- galvanically isolated CAN front end
- CAN transient/ESD protection
- improved ground/reference strategy
- sealed external connector
- conformal coating
- pressure-equalized sealed enclosure
- sealed buzzer mounting that remains audible without creating a water path

## Primary Hardware References

- Waveshare product documentation: https://docs.waveshare.com/ESP32-S3-Touch-LCD-5
- Waveshare engineering examples: https://github.com/waveshareteam/ESP32-S3-Touch-LCD-5
