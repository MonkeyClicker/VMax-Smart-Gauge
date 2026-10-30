# Rev J - 170 x 130 mm screwless enclosure

This final working revision is the [in-dash mount design v1 checkpoint](../../in-dash-v1/README.md). The current VMAX lettering font remains provisional, with its replacement left for a later design revision.

Rev J adds integral raised VMAX lettering: actual letter height 15 mm, width 56 mm, projection 1 mm from the bezel face. It is centered in the usable bottom flat land, at outside-front-view X0, Y=-48.4625, with equal 3.0375 mm margins to the screen chamfer and accent groove. Outlines are shared with the STL generator, so the drawing and model agree. Text reaches Z=-1, making the housing alone 47 mm deep, and assembled depth to one-piece cover 50 mm (53 mm to the removable panel). Bezel-face and internal depth datums remain unchanged. Raised lettering requires revisiting housing print orientation/supports; see the STL guide.

Rev I changes both rear-cover options to 30 mm port center spacing (outside rear view X=-30, 0, +30, row Y=-22). The removable panel becomes 100 x 42 x 3 mm, mounting holes move to X=+/-45, Y=+/-16 locally, and rear-cover opening becomes 84 x 26. Use the widened panel and matching cover together; old narrow panels/covers do not match. One-piece cover outline and six mounting holes are unchanged. All other Rev H features remain.

Rev H moves the glass seat and support stack back another 1 mm: glass seat Z4, cavity entrance Z7, retainer post tops Z12, retaining fingers Z12-15. Total shift is 2 mm from v1. The 3 mm x 45-degree screen chamfer, 116 x 72.8 opening and 122 x 78.8 front mouth remain, with a new 1 mm straight throat between the chamfer and glass seat. Covers and exterior dimensions remain unchanged.

Rev G increases the screen chamfer to a full 3 mm x 45 degrees. The opening at the glass stays 116 x 72.8 mm; the front mouth is 122 x 78.8 mm. As selected by the user, the glass seat moves from Z2 to Z3, cavity entrance from Z5 to Z6 and retaining supports/fingers back 1 mm (posts end Z11; fingers Z11-14). Rear rim, covers, overall envelope and bezel remain unchanged. Recheck actual display and cable clearance after this 1 mm shift.

Rev F enlarged the touchscreen opening from 110 x 66.8 to 116 x 72.8 mm: 3 mm on each of the four edges. Rev G retains that opening and Y+1.475 offset, with the screen chamfer and support depth changes described above.

Rev E adds a one-piece rear-cover option on sheet 07. This 156 x 116 x 3 mm cover integrates the same three bulkhead cutouts, keeps the six housing screws and rear gasket, and removes the removable-panel opening/joint and four panel screws. Installed rear face is Z49 instead of Z52. Both configurations remain available. Use `rear-cover-one-piece-three-port.stl` instead of the panel-based assembly.

Rev D added three rear ports on 23 mm centers, outside rear view left-to-right USB power / NMEA 2000 / SD card. USB and SD holes have 20 mm nominal diameter with a vertical left flat 1 mm inward from the circle tangent; NMEA is 14.5 mm diameter. The removable-panel rear-cover opening is widened from 64 to 70 mm to clear the outer ports. Panel outline and mounting holes are retained. Actual bulkhead nuts/flanges and bodies remain unverified.

The 7 mm flange, 3 mm outer chamfer and 170 x 130 bezel remain unchanged. Seven A3 sheets cover front/rear, all four sides, internal housing, cover/cable panel, four retainer fingers, installation section and one-piece-cover option. Older PDFs and ZIPs are historical snapshots; use the Rev J PDF/package with the current STLs and SVGs.

The interior follows external-v1 `housing.stl` and its generator. The old large PCB carrier, eight-screw cover, blank cable pilots and M4 clamp anchors are removed. The v1 glass pocket, offset window, retainer posts/fingers, rear-cover outline and six-screw pattern are preserved. The replaceable panel now has three ports and the cover has a wider opening. The external side pivots/tilt bracket are omitted.

New housing STLs and validation are in `enclosure/STL/screwless-v2/`; editable model sources are in `enclosure/CAD/screwless-v2/`. The face is exactly 170 x 130 mm, with a 7 mm flange, 7 mm overlap per side, clipped corners, 3 mm outer bevel and cosmetic accent groove moved clear of that bevel. Glass seat is Z4 and supports move back 2 mm total. Raised VMAX lettering is part of the housing STL.

Dimensions are millimetres; print the A3 PDF at 100%. All plan/elevation views are 1:1 except the enlarged retainer at 4:1. Section view is schematic. B = enclosure-v1 baseline, P = new bezel detail, H = hardware-dependent. Vector SVG files accompany the PDF.

Dash mounting brackets remain to be designed and validated; this revision does not reuse the old clamp arrangement. Physical fit, flange stiffness, seal compression, connectors and cable clearances need checking before fabrication/installation. The proposed cutout is not a cutting template.
