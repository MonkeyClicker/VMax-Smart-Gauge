# Screwless dash enclosure v2, Rev J - 170 x 130 mm bezel

Released as [in-dash mount design v1](../../in-dash-v1/README.md). The current VMAX font is provisional and is not approved as the final style; the raised lettering is retained for this checkpoint.

Rev J adds integral VMAX lettering to the housing STL: 15 mm actual letter height, 56 mm width, raised 1 mm above the bezel face. The lettering is centered horizontally at X0 and vertically in the usable bottom flat land between the screen chamfer and accent groove (outside-front-view Y=-48.4625), with equal 3.0375 mm top/bottom margins. The custom geometric bold outlines are shared by the model and drawings in `enclosure/CAD/bezel_lettering.py`.

Rev I spaces all rear bulkhead centers at 30 mm: outside rear view X=-30, 0, +30. The removable panel is widened to 100 x 42 x 3 mm, with mounting screws moved to X=+/-45 and matching rear-cover opening widened to 84 x 26. Use the new panel and cover together; earlier narrow panels/covers are not interchangeable with this option. One-piece cover outline and six-screw pattern remain unchanged.

This revision follows the external-v1 interior architecture with its glass seat, cavity entrance and retaining supports moved back 2 mm total (another 1 mm after Rev G). The full 3 mm x 45-degree screen chamfer is followed by a 1 mm straight throat. A 170 x 130 mm screwless dash flange replaces the external pivot mounting. It is a dimensional prototype, not a validated installation.

## Parts

- `housing.stl`: 170 x 130 x 47 mm overall including raised lettering (Z=-1 to 46). Overhanging flange is 7 mm thick with a 3 mm, 45-degree outer chamfer, leaving 4 mm edge thickness. Original body footprint: 156 x 116 mm. Side pivots removed. Integral VMAX lettering is 15 mm high and raised 1 mm. Cosmetic accent groove is retained; the blue insert is not modeled.
- `rear-cover.stl`: 156 x 116 x 3 mm, v1 six-screw pattern; cable opening 84 x 26 mm, panel fasteners at X=+/-45, installed Y=-38 and -6. Use this updated cover with the new 100 mm three-port panel.
- `rear-cover-one-piece-three-port.stl`: optional single-piece 156 x 116 x 3 mm cover with USB/NMEA/SD bulkhead cutouts directly in the plate, at outside-rear-view centers (-30,-22), (0,-22), (+30,-22). Same six cover screws and housing seal. No rectangular cable opening or four panel screw holes. Install instead of `rear-cover.stl` plus separate cable panel; its rear face is Z49, 3 mm shallower than the panel configuration. Print flat, exterior face down. See drawing sheet 07 and `assembly-one-piece-cover.scad`.
- `cable-panel-three-port.stl`: 100 x 42 x 3 mm, mounting holes diameter 3.4 at X=+/-45, Y=+/-16. Viewed from outside rear: USB power left, NMEA 2000 center, SD card right. Centers at X=-30, 0, +30 mm, all Y=0. USB and SD holes are nominal diameter 20 mm with vertical left flats 1 mm inward from each circle tangent (flat at local X=-9, chord 8.72 mm). NMEA hole is diameter 14.5 mm, chosen within the requested 14-15 mm range. This panel is shown in the current assembly.
- `cable-panel-blank.stl`: 100 x 42 x 3 mm fallback with the same new mounting holes. Original external-v1 blank/two-port panels do not fit the wider opening.
- `display-retainer-print-4.stl`: unchanged v1 retainer; print four.

All STL units are millimetres. Cover, panel and retainers print flat. Raised lettering changes the housing's front-face-down setup: the text reaches the bed first and leaves a 1 mm gap beneath the surrounding bezel. The former support-free face-down instructions no longer apply; choose a slicer-reviewed orientation/support arrangement and inspect the complete layer preview before printing. No tilt bracket or dash clamps are supplied. Select/validate dash mounting brackets before installation; old flush-dash M4 clamps and carrier are not part of this revision.

## Preserved interior

Glass pocket 128.30 x 88.05, R4.30, seat Z4. Opening at the glass is 116 x 72.80, centered Y+1.475. The screen chamfer remains 3 mm x 45 degrees, giving a 122 x 78.8 mm front mouth, with a straight throat from Z3 to Z4. Cavity 144 x 104, R5, now opens at Z7. Retainer posts end at Z12; retaining fingers occupy Z12 to 15. Retainer pilots move back 2 mm total to Z7.4 through Z12; rear-cover screw pattern, rear pilots and seal track remain. Retainer shape and nominal pad gap are unchanged because glass and supports move together. Recheck board/cable clearance with the display 2 mm farther back than v1. Follow the original external-v1 guide for pads, plastic-thread screws, seals and panel assembly, using these revised depths and the 84 mm wide rear-cover opening. Cover fasteners lie inside the gasket track and require separate sealing.

Housing rear rim Z46; cover ends at Z49; separate panel ends at Z52, excluding gaskets and fittings. Flange rear face is Z7, giving 42 mm rear projection to the cover and 45 mm to the panel. The 7 mm overlap per side and 7 mm flange thickness need physical stiffness/sealing validation. Glass seat is now Z4; added flange material is cleared from the pocket and cavity. Trial cutout 157 x 117 R8.5 is proposed only; verify before cutting.

Raised text projects to Z=-1. Including that projection, assembled depth is 50 mm to the one-piece cover or 53 mm to the separate panel, before gaskets/connectors. Bezel-face datum Z0 and all glass/support positions are unchanged.

`validation.json` records closed meshes, connected parts, pairwise nominal assembly interference checks and original interior comparison. These checks exclude actual display, hardware, seals, connectors, loads and water resistance. Original v1 files are unchanged.

The three port cutouts clear the new cover opening by at least 2 mm. Actual bulkhead nuts/flanges, key-flat dimensions, seals and connector bodies must be verified against the parts; cutout clearance does not establish their envelope fit. No printer hole compensation has been added. Seal the panel with an appropriate sheet gasket and sealed fasteners. The blank panel remains available as a fallback.

Editable source is in `enclosure/CAD/screwless-v2/`; regenerate with `enclosure/CAD/generate_screwless_v2.py`, using the same numpy/manifold3d dependencies as the original generator. Revised seven-sheet drawings are in `enclosure/drawings/screwless-concept-v2/`.
