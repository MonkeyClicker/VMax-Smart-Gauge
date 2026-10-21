# Hat Labs male NMEA 2000 panel pigtail connector

**Status update:** deferred due to shipping/tariff cost. The current panel
candidate is the [AliExpress connector](AliExpress-NMEA2000-Panel-Male.md).
Specifications below remain reference data for this alternate only.

BOM ID: **NMEA-01**, quantity **1** per enclosure.
Selected by user, who is ordering it; delivery and physical measurements are
not yet confirmed. Selection and source verification date: **2026-10-20**.

Source: [Hat Labs product page](https://shop.hatlabs.fi/products/nmea-2000-panel-pigtail-connector-male).
This is a locally saved engineering summary of published specifications, not
a manufacturer dimension drawing. No separate manufacturer part number is
published in the retrieved product description; identify it by the linked
product and the current 12 mm-hole variant.

## Published specifications

| Property | Vendor specification |
| --- | --- |
| Product | NMEA 2000 panel pigtail connector, male |
| Interface | NMEA 2000-compatible Micro-C style male panel connector |
| Termination | Prewired pigtails, no connector soldering required |
| Mounting hole | **12 mm diameter** |
| Mounting direction | Rear mounted; mounting nut is **outside** the enclosure |
| Lead length | **30 cm** (vendor also describes as 12 inches) |
| Red and black conductors | **22 AWG** |
| White and blue conductors | **24 AWG** |
| Ingress protection | Vendor lists **IP67**; the page does not specify test conditions or whether a mating connector/cap is required |

## Wiring for this project

The supplier identifies black as GND, red as 12 V, white as H, and blue as L.
For this project's USB-powered prototype:

| Lead | Supplier function | Project connection |
| --- | --- | --- |
| White | CAN H | Waveshare CAN-H |
| Blue | CAN L | Waveshare CAN-L |
| Black | Ground | CAN reference / ground per project wiring |
| Red | Network 12 V | **Leave disconnected and individually insulated**; prototype uses separate USB power |

The product text does not document a shield/drain lead or numbered contact
mapping. Do not invent either from wire colors. Verify contact-to-lead
continuity on the received connector before connecting it to the boat.
CAN termination remains disabled for this drop device.

## CAD reference and measurements still needed

Machine-readable reference: [Hat-Labs-NMEA2000-Panel-Male.json](Hat-Labs-NMEA2000-Panel-Male.json).

Use **12.0 mm as the vendor nominal mounting hole**, not as an established
printer compensation. A printed-hole clearance allowance is a separate design
choice to establish with a fit coupon or drill to final size.

Draft v1 has a 3 mm thick, 80 x 42 mm replaceable rear panel over a 64 x 26 mm
opening. This is the intended mounting part. The current STL is still blank;
the connector selection alone does not establish its exact location.

The following are not provided in the product's published specification text
and must be measured on delivery or confirmed by Hat Labs before final CAD:

- maximum allowed panel thickness and usable threaded shank length
- flange/seal diameter and required flat sealing land
- nut diameter/across-flats and tool clearance
- inside body length/diameter and wire exit direction
- outside projection, mating-cable coupling envelope and cable bend radius
- antirotation flat/key geometry, mounting thread and tightening torque
- seal hardware supplied and sealing requirements when unmated

Keep the outside nut accessible and leave room for the female drop-cable
coupling. Confirm the 3 mm panel fits the threaded mounting stack including
the seal. Check internal wires against the PCB, USB plugs and cover hardware,
and external cable clearance against the bracket throughout the intended tilt.
Do not use the Actisense PG9/15.3 mm cutout for this connector. Hat Labs notes
that its earlier variant had a different size; measure the delivered version.

IP67 is a component claim and does not establish an enclosure rating. The
assembled printed panel, seals, fasteners and mated connection need the
project's unpowered water-ingress tests.
