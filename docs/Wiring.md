# Prototype Wiring Design

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
                           |
                         USB 5 V
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

## Validation Before Use

Before relying on the gauge, verify that the original Garmin and Yamaha data continue to operate normally, the backbone still has exactly two intended terminators, the custom gauge sees stable NMEA traffic, and disconnecting the custom gauge does not affect the rest of the network.
