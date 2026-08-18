# Water-Resistant Enclosure

## Goal

Build a serviceable marine enclosure that can tolerate rain, spray, and normal helm exposure. Do not claim a formal IP rating without testing the finished assembly.

## Current Mechanical Direction

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
