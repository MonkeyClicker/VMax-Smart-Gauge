# Prototype Wiring Design

For the complete motor-to-Garmin-to-network-to-gauge installation, verified part numbers, Micro-C pin mapping, power circuits, and commissioning checks, see [Complete Boat Wiring Guide](Complete-Wiring.md). This page is the earlier prototype summary; use the complete guide for installation details and compatibility limits.

## Goal

Connect the Waveshare ESP32-S3-Touch-LCD-5 to the existing NMEA 2000 network as an additional drop device while preserving the existing Yamaha and Garmin installation.

## Existing Network

```text
Terminator -- backbone -- T -- T -- T -- backbone -- Terminator
                         |    |    |
                      Garmin Yamaha Gauge
```

The custom gauge uses the existing open drop connection. It is not a backbone endpoint and therefore does not add another terminator.

## Gauge Connections

For the prototype:

```text
NMEA 2000 drop                  Waveshare
-------------------------------------------
NET-H / CAN High   -----------> CAN-H
NET-L / CAN Low    -----------> CAN-L
NET-C / reference -----------> CAN reference
NET-S / network power          not used for gauge power
```

The Waveshare board is powered separately by USB from the LiFePO4 battery system.

## Waveshare CAN Interface

Current selected board:

- Waveshare ESP32-S3-Touch-LCD-5
- 5-inch 800x480 capacitive display
- onboard TJA1051 CAN transceiver
- CAN TX: GPIO15
- CAN RX: GPIO16

The onboard optional CAN termination should remain disabled when the unit is attached as a normal NMEA 2000 drop.

## Audible Buzzer Output

The board has no onboard buzzer. Add a waterproof or panel-sealed active piezo buzzer and switch it with isolated open-drain output DO0.

```text
Fused 5-12 V positive  ---- buzzer positive
Buzzer negative       ---- DO0 switched output
Supply/output return  ---- output common per Waveshare terminal diagram
```

Before energizing:

- match the active buzzer voltage rating to the selected supply
- use a dedicated low-current fuse near the supply connection
- keep buzzer current below 100 mA
- verify DO0 and output-common labels against the exact board revision and Waveshare schematic
- default DO0 to off during boot, reset, OTA, and fault recovery
- use marine-grade wire, suitable crimp terminals, strain relief, and corrosion protection
- do not use GPIO15 or GPIO16 for the buzzer because they are reserved for CAN

See [Audible Warning System](Audible-Warnings.md) for warning patterns and validation.

## Prototype Topology

```text
                 Existing NMEA 2000 backbone
                           |
                         open T
                           |
                      short drop cable
                           |
                   CAN-H / CAN-L / ref
                           |
                Waveshare ESP32-S3 gauge
                    |                 |
                  USB 5 V          DO0 buzzer
                    |
              LiFePO4 battery
```

## Development Connection

During development the USB cable may instead connect the Waveshare to a laptop for firmware flashing and serial output.

## Permanent Installation Considerations

Before final enclosure installation, review:

- waterproof cable entry or connector
- strain relief
- CAN isolation/transient protection
- serviceability of internal USB recovery connection
- avoidance of water paths along cable jackets
- enclosure pressure equalization
- sealed acoustic path or panel-mounted waterproof buzzer

## Validation Before Use

Before relying on the gauge, verify that the original Garmin and Yamaha data continue to operate normally, the backbone still has exactly two intended terminators, the custom gauge sees stable NMEA traffic, and disconnecting the custom gauge does not affect the rest of the network.

Before relying on audible warnings, test all patterns dockside, measure buzzer voltage/current, verify DO0 is off during boot and updates, confirm audibility with the engine running, and compare engine warning behavior with the Yamaha and Garmin displays.
