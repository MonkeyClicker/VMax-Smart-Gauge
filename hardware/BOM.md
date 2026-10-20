# Bill of materials

External prototype build. Quantities are per gauge unless noted. Procurement
status is recorded only when confirmed; selected does not mean purchased.

| ID | Qty | Component | Selection / status | Reference |
| --- | --- | --- | --- | --- |
| DISP-01 | 1 | Display/controller | Waveshare ESP32-S3-Touch-LCD-5, capacitive 800x480; existing tested board | [Hardware](../docs/Hardware.md) |
| NMEA-01 | 1 | Male panel-mount NMEA 2000 connector with pigtails | FireEarth/AliExpress M12 A-coded 5-pin, 30 cm; current panel candidate, purchase unconfirmed. Hat Labs deferred due to shipping/tariff cost | [Current specifications](datasheets/AliExpress-NMEA2000-Panel-Male.md), [Hat Labs alternate](datasheets/Hat-Labs-NMEA2000-Panel-Male.md) |
| NMEA-02 | 1 | NMEA 2000 Micro-C drop cable | Existing network drop arrangement; confirm female end mates to NMEA-01 | [Wiring](../docs/Wiring.md) |
| PWR-01 | 1 | Regulated 5 V USB supply and data-capable USB-C cable | Existing LiFePO4 battery USB source; USB-01 selected for enclosure access | [Power](../docs/Power.md) |
| USB-01 | 1 | Panel-mount USB-C female to internal USB-C male extension | STA EP-TYPE-C, **FPC-10cm** variant; **ordered**, user confirmed 2026-10-20; receipt/testing pending | [Saved selection and CAD notes](datasheets/USB-C-Panel-Extension.md) |
| LOG-01 | 1 | microSD card | Capacity/model to be selected; firmware logging acceptance still pending | [Project plan](../docs/Project-Plan.md) |
| ENC-01 | 1 set | Printed external enclosure | Draft v1: housing, rear cover, cable panel, bracket, four retainers; PETG dimensional prototype, ASA planned outdoor version | [Print files and hardware quantities](../enclosure/STL/external-v1/README.md) |
| SEAL-01 | 1 set | Gaskets, sealant, pads, sealed fasteners and vent | Final products/sizes to be selected after dry fit | [Enclosure](../docs/Enclosure.md) |
| ALARM-01 | 1 | Active piezo buzzer and protected supply wiring | Waterproof/panel-sealed 5-12 V, below 100 mA; product not yet selected | [Audible warnings](../docs/Audible-Warnings.md) |

The current AliExpress NMEA panel candidate specifies a **15.5-16 mm cutout**.
The first two-port panel uses 16 mm. The saved Hat Labs 12 mm specification
belongs to the deferred alternate and must not be used for this connector.
