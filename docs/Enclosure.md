# Water-Resistant Enclosure

## Goal

Build a serviceable marine enclosure that can tolerate rain, spray, and normal helm exposure. Do not claim a formal IP rating without testing the finished assembly.

## Printable Flush-Dash Prototype

The current [in-dash mount design v1 checkpoint](../enclosure/in-dash-v1/README.md)
uses a 170 x 130 mm screwless bezel, 7 mm flange, 3 mm chamfers, four glass
retainers and rear ports on 30 mm centers. It includes a removable cable-panel
cover and a one-piece rear-cover option, seven drawing sheets, CAD/STLs and a
download package. The VMAX font is provisional; physical fit, seals and dash
retention still require validation. This checkpoint is separate from the older
flush-dash prototype below and the protective-cover CAD's smaller bezel.

The [enclosure build guide](../enclosure/README.md) contains the adjustable OpenSCAD source, five printable parts, a fit-test print, dash template, hardware list and assembly procedure. The owner selected PETG on an Ender 3, a 1/8–1/4 inch dash and 4–5 inches of rear clearance.

The bezel is 197 × 162 mm, proposed cutout 174 × 139 mm, and housing depth behind the bezel 53 mm before external hardware. **Print the fit test first and verify the provisional carrier height and connector clearance before a full print or dash cut.** Cable-gland and vent holes remain closed until actual hardware sizes are selected. The design has no tested IP rating.

## Current Mechanical Direction

### First enclosure: external test unit

The first complete enclosure will be an external, removable unit for boat test
runs, rather than an in-dash installation. Use it to collect running data and
evaluate wiring, cable routing, connection types, and port placement while the
in-dash enclosure is developed separately.

The original printed bezel was physically test-fitted by the user and reported
to fit perfectly. Preserve its glass pocket and viewing-area alignment. A second
test bezel adds a 0.8 mm, 45-degree chamfer around the front screen opening;
its physical print/acceptance is pending.

External prototype design priorities:
- serviceable rear cover for wiring changes and microSD access
- room for connector bodies, cable bends, and strain relief
- replaceable cable-entry panel so ports can evolve without replacing the housing
- removable external mounting arrangement; bracket and attachment details remain to be selected
- access to USB power/programming and recovery during testing, with protection
  against spray when in use; OTA is still outstanding
- retain the planned front seal, rear gasket, and protected cable entries for boat use

Draft 1 printable parts and assembly notes are available in
[external-v1](../enclosure/STL/external-v1/README.md). It includes an integral
chamfered front housing, six-screw rear cover and gasket track, blank replaceable
cable panel, four adjustable display fingers, and a removable U bracket. The
156 x 116 x 49 mm body (excluding pivots and panel) is a dry-fit draft; port
selection, retainer padding, seals and mounting security require validation.

Continue the existing prototype electrical plan: regulated USB battery power and
a separate NMEA 2000 drop connection. External mounting does not change the power
or CAN termination decisions. Final port types, enclosure depth, and mounting
geometry remain open until connector and mounting choices are established.
Enclosure availability does not imply SD logging firmware is complete.

### Marine enclosure construction

- PETG flush-dash prototype for the owner’s Ender 3; ASA remains an outdoor-material option
- touchscreen glass exposed through front bezel
- perimeter seal around touchscreen glass
- removable rear cover
- recessed silicone gasket groove
- stainless fasteners spaced for even compression
- sealed cable entries/connectors
- hydrophobic/ePTFE pressure equalization vent
- conformal coating as a secondary PCB protection layer

## Front Seal

Do not place an unnecessary clear cover over the capacitive touchscreen.

Preferred stack:

```text
outside
  │
front bezel
  │
sealed overlap around display glass
  │
touch glass / LCD assembly
  │
PCB
inside
```

The front display assembly should remain sealed when the rear service cover is removed.

## Rear Cover

Use a continuous silicone gasket in a defined groove rather than relying only on sealant.

Goals:
- repeatable compression
- serviceability
- no pinched gasket sections
- stainless fasteners around the perimeter

## Cable Penetrations

Minimize penetrations.

The final unit needs three rear-panel ports:
- NMEA 2000/CAN connection
- USB-C power/programming connection
- externally accessible microSD/TF connection

Current NMEA panel candidate: **FireEarth/AliExpress M12 A-coded 5-pin male
pigtail**, 30 cm, 22 AWG, seller-specified 15.5-16 mm panel cutout. Hat Labs is
deferred due to shipping/tariff cost. See [BOM](../hardware/BOM.md) and
[current connector specifications](../hardware/datasheets/AliExpress-NMEA2000-Panel-Male.md).
The existing `cable-panel-two-port.stl` has a 16 mm NMEA hole and provisional
16 mm USB-C hole. It is retained only as a two-connector fit-test part and is
superseded as the final rear-panel configuration.

The acquired QIANRENON microSD/TF bulkhead extension requires a third port. The
listing specifies a **21 mm round opening**, **6.3 mm maximum panel thickness**,
**13/16-28 UNS mounting thread**, **16 cm / 6.3 in FPC ribbon**, and a nominal
**28 mm / 1.1 in** dimension. It shows a mounting nut, O-ring and tethered cap
and claims IP67 water/dust protection. See the
[microSD component record](../hardware/datasheets/QIANRENON-MicroSD-Panel-Extension.md).
The current 3 mm printed panel satisfies the listed thickness limit, but the
existing 64 × 26 mm rear-cover opening and 80 × 42 mm two-port panel have not
been validated for three flange, cap, nut and cable envelopes. Measure the
received NMEA, USB-C and microSD parts before enlarging the opening or releasing
a three-port STL. Preserve tool access, card/cap clearance, ribbon bend radius,
panel gasket land and strain relief.

USB programming should not require an external exposed port because OTA is planned. Internal USB remains available for recovery.

For the external test enclosure, the user has ordered a panel-mount USB-C
female-to-male extension, selected **FPC-10cm** variant, for power/programming.
See [USB component notes](../hardware/datasheets/USB-C-Panel-Extension.md).
Plan its position alongside NMEA and the new microSD bulkhead extension on the
replaceable rear panel after measuring all three connectors. Mounting dimensions,
reach, power/data operation and sealing require verification. The two-port
fit-test panel remains available alongside the blank, but the final design now
requires a new three-port panel and a checked rear-cover opening.

## Pressure Equalization

Add a small hydrophobic membrane vent on the rear/bottom area to reduce pressure cycling and condensation risk while resisting liquid water ingress.

## PCB Protection

Conformal coating can provide secondary protection against humidity/condensation. Mask connectors, switches, SD contacts, and other areas that must remain electrically/mechanically accessible.

## Material

PETG is selected for this owner’s Ender 3 prototype. Verify heat distortion, clamp creep and water sealing on the printed assembly before permanent installation. ASA remains an option for a later version with a suitable printer setup; changing filament requires a new fit and sealing check.

## Testing Plan

Before permanent helm installation:
1. visual gasket inspection
2. controlled spray test with electronics removed or protected
3. paper/moisture indicator test inside enclosure
4. thermal/sun exposure test
5. repeat spray test after thermal cycling
6. inspect cable entries and pressure vent
7. only then repeat with powered electronics
