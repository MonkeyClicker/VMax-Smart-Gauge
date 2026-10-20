# Front bezel fit-test v1

Print **front-bezel-fit-test.stl**, in millimeters at **100% scale**.

Designed for the bare Waveshare **ESP32-S3-Touch-LCD-5 capacitive 800x480**
assembly selected in `docs/Hardware.md`. This is a shallow fit gauge, without
fasteners, PCB supports, gasket groove, rear housing, or a waterproof seal.

## Dimensions and sources

Official dimension drawing (touch version):
https://docs.waveshare.com/assets/images/ESP32-S3-Touch-LCD-5-touch-size-2-f55a9abf7b42455adbc880b71b4b92f9.webp

Local reference: `../drawings/waveshare-touch-dimensions.webp`.
Retrieved 2026-10-20 from https://docs.waveshare.com/ESP32-S3-Touch-LCD-5.
Use that drawing rather than the conflicting 112.4 x 75.1 mm summary table
on the documentation page, which does not match the 5-inch touch drawing.

Vendor dimensions, mm:
- Glass: 127.70 +/-0.1 wide x 87.45 +/-0.1 high; four R4 corners.
- Viewing area: 109.00 +/-0.2 x 65.80; left and top offsets 9.35.
- Thus viewing-area center is 1.475 mm above glass center.

Design choices, mm:
- Glass pocket: 128.30 x 88.05, R4.30 (0.30 clearance per side nominal).
- Window: 110.00 x 66.80, square corners, 1.475 above glass center.
- Outside: 140.30 x 100.05, R7.30.
- Front lip: 2.00 thick; locating rim extends another 2.00 behind it.
- Total thickness: 4.00. No allowance for seal material in this first test.

The shallow rim locates the front glass only. It does not capture the full
assembly thickness or prove PCB, connector, or mounting-hole clearance.

## Print and fit

1. Keep the supplied orientation: broad flat front face on the bed (Z=0),
   recessed glass pocket facing up. No supports required.
2. Suggested starting settings: 0.20 mm layers, 3 walls, 15-20% infill.
   PLA is acceptable for a short indoor dimension check; use the planned ASA
   and its tuned printer profile when assessing final-material fit/shrinkage.
3. Remove any brim and first-layer bulge from the pocket before trying it.
4. Rest the print front face down on a clean soft surface. Gently place the
   display glass face down into the pocket from above. Do not force or clamp
   the glass. Support the board independently; this is not a retaining mount.
5. The wider glass border belongs at the bottom. The opening center is shifted
   toward the top; keep that orientation when checking the lit display.
6. Check that glass enters freely, sits evenly, and the entire lit image and
   edge touch targets remain accessible. Record clearance on all four sides,
   rocking/corner interference, and any PCB/contact interference.

Do not glue, seal, drill, or install this test piece permanently. Use the
observed fit to adjust the editable source before making a sealed enclosure.

## Editable source and validation

`../CAD/front-bezel-fit-test.scad` is the parametric CAD source.
`../CAD/generate_front_bezel.py` generates the matching default STL using only
Python's standard library. If parameters change, regenerate with OpenSCAD,
or update the generator constants to match before running it.

The generated mesh is checked for nonzero triangle areas, two faces per edge,
opposite edge winding, positive volume, and correct planar face normals.
This checks the digital mesh; physical fit still needs the first print.
