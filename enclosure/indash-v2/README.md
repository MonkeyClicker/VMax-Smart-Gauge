# VMAX matched protective cover — CAD V2

Parametric OpenSCAD model for the two-piece glued cover, matched to the **provisional** in-dash bezel V1. Open `vmax_cover_assembly.scad`; set `part` to `front`, `rear`, `coupon`, or `assembly`. Press F6 and export STLs from OpenSCAD.

- Nominal bezel: **163 × 112 mm**; cover **169.5 × 118.5 mm**, excluding finger lips.
- Four inward clip nubs (two per side) align to bezel pockets at y=±23 mm, z=2.5 mm.
- Front and rear are separate prints, joined by glue flange and four locating pins.
- Rear includes clip relief slots; front includes left/right removal lips.
- Print and validate the **coupon** first. Tune `side_clearance`, clip geometry and printer tolerances before a full print.
- Dry fit and cycle clips **before gluing**. Use PETG-compatible adhesive, prevent overflow into flexible tabs.
- Front currently lacks 3D YAMAHA VMAX lettering and two-color toolpaths; that is the next cosmetic CAD revision.
- This is **not a water-tight enclosure**. Final display dimensions, dash cutout, gasket, mounting retention and thermal design are not yet validated.
- **Do not cut the dashboard based on these provisional dimensions.**

See [cover design](../../docs/Protective-Cover-Design.md) and the matching mount source in this folder.

### STL generation
```bash
openscad -o front.stl -D 'part="front"' vmax_cover_assembly.scad
openscad -o rear.stl -D 'part="rear"' vmax_cover_assembly.scad
openscad -o coupon.stl -D 'part="coupon"' vmax_cover_assembly.scad
```
