# Optional glued split housing

Use these two STLs **instead of** the one-piece `housing.stl`:

- `front-bezel-with-pins.stl`: 170 x 130 mm bezel, 7 mm flange, four integral pins projecting another 4 mm. Print front face on the bed, pins upward.
- `rear-housing-glue-option.stl`: 156 x 116 x 39 mm rear box, with matching blind sockets. Print the new front joint face on the bed, existing rear cover rim upward. Its export is shifted 7 mm forward in Z for printing; assembled position is Z7 relative to the bezel face.

The split lies exactly at the transition from the bezel flange to the outer box, Z7. The glass seat, screen opening, supports, rear seal and rear-cover interfaces retain their current assembled locations. Both existing rear-cover options fit. VMAX remains a flat surface marking; there is no raised text in these parts.

## Alignment

Four pins are diameter 2.6 mm, with 4 mm projection and a 0.3 mm tapered tip. Sockets are diameter 3 mm, 4.5 mm deep, with a short diameter 3.6 mm entry lead. Nominal radial clearance is 0.2 mm and end clearance is 0.5 mm. This is a locating/slip fit, not a press fit; actual fit depends on printer compensation.

Pin centers in model XY are (-75,-25), (-75,+25), (+75,-20), (+75,+30) mm. The asymmetric pattern prevents a reversed assembly. The matching faces appear mirrored when viewed face-to-face; use drawing sheet 08.

## Glue assembly

1. Inspect the slicer preview, then print one of each part at 100% in millimetres. Clean the mating faces and dry-fit the pins before using adhesive. The rims should meet evenly without forcing the pins or distorting the enclosure.
2. Prepare surfaces and use adhesive compatible with the actual printed material. Bond the continuous flat housing rim to the bezel. Pins locate the parts while the joint cures; they do not establish joint strength by themselves.
3. Keep excess adhesive out of the glass seat, electronics cavity and fastener bores. Avoid pooling adhesive at the bottom of the blind sockets. Clamp gently without bending the bezel and follow the adhesive's cure instructions.
4. Verify the new seam is continuously sealed. Test the glued enclosure without electronics before exposure to water. The CAD uses a nominal zero-gap joint; a real adhesive bondline can add slightly to assembled depth.

The generated meshes are closed, consistently wound and single connected parts; the split preserves the unsplit geometry, and pins/sockets pass nominal interference checks. Real fit, bond strength, printed-wall strength and sealing remain physically unverified. The narrowest sidewall at each socket entry is nominally 1.2 mm; do not force the pins.
