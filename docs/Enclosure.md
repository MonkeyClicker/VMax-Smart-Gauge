# Water-Resistant Enclosure

## Goal

Build a serviceable marine enclosure that can tolerate rain, spray, and normal helm exposure. Do not claim a formal IP rating without testing the finished assembly.

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

- ASA 3D-printed housing
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

The final unit needs:
- NMEA 2000/CAN connection
- power connection

USB programming should not require an external exposed port because OTA is planned. Internal USB remains available for recovery.

## Pressure Equalization

Add a small hydrophobic membrane vent on the rear/bottom area to reduce pressure cycling and condensation risk while resisting liquid water ingress.

## PCB Protection

Conformal coating can provide secondary protection against humidity/condensation. Mask connectors, switches, SD contacts, and other areas that must remain electrically/mechanically accessible.

## Material

ASA is preferred over PLA for the final printed prototype because of improved outdoor/UV/heat suitability.

## Testing Plan

Before permanent helm installation:
1. visual gasket inspection
2. controlled spray test with electronics removed or protected
3. paper/moisture indicator test inside enclosure
4. thermal/sun exposure test
5. repeat spray test after thermal cycling
6. inspect cable entries and pressure vent
7. only then repeat with powered electronics
