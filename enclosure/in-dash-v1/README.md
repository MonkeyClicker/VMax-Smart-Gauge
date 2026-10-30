# In-dash mount design - version 1

Version 1 checkpoint, 2026-10-30. This records the current 170 x 130 mm screwless dash design, including both rear-cover options. **The current VMAX font is provisional and is not approved as the final lettering style.** Its geometry is retained in this checkpoint; a future revision can replace the font.

## Files

- [STL-only download](vmax-in-dash-mount-v1-stls.zip): six STL files, print notes, validation report and verified SHA-256 manifest. Print four display retainers; choose either the one-piece rear cover or the cover plus removable panel.
- [Complete version 1 package](vmax-in-dash-mount-v1.zip): CAD, binary STLs, validation report, seven vector drawing sheets and supporting generators.
- [Dimensioned drawing set](../drawings/screwless-concept-v2/vmax-screwless-enclosure-drawings-rev-j.pdf): seven A3 sheets, drawing revision J.
- [STLs and assembly/print notes](../STL/screwless-v2/README.md).
- [Editable OpenSCAD parts](../CAD/screwless-v2/).
- [Removable-panel assembly](../CAD/screwless-v2/assembly.scad) and [one-piece-cover assembly](../CAD/screwless-v2/assembly-one-piece-cover.scad), for review rather than printing as an assembly.
- [Screwless dash styling concept](../../media/graphics/vmax-dash-mounted-concept-v2.png): visual reference, not a dimensional rendering of the final model.

The implementation folders retain their working names `screwless-v2` and `screwless-concept-v2`; this checkpoint's design-release version is **1**. Drawing revision J identifies the final working revision included here. Earlier drawing/package iterations are not part of this release.

## Version 1 dimensions

| Feature | Version 1 |
| --- | --- |
| Bezel outline | 170 x 130 mm, clipped corners |
| Overhanging flange | 7 mm thick, 7 mm overlap per side |
| Outer chamfer | 3 mm x 45 degrees |
| Opening at glass | 116 x 72.8 mm; Y offset +1.475 mm |
| Screen chamfer | 3 mm x 45 degrees; front mouth 122 x 78.8 mm |
| Glass pocket | 128.30 x 88.05 mm, R4.30; seat Z4 |
| Internal cavity | 144 x 104 mm, R5; opens at Z7 |
| Display retention | Four fingers on posts ending Z12; fingers Z12-15 |
| Rear housing rim | Z46 |
| Rear covers | 156 x 116 x 3 mm; six screws; rear face Z49 |
| Removable port panel | 100 x 42 x 3 mm; rear face Z52; cover opening 84 x 26 mm |
| Rear port spacing | 30 mm centers; USB power left, NMEA center, SD right, viewed from outside rear |
| Port cutouts | USB/SD nominal diameter 20 mm, 1 mm left flats; NMEA diameter 14.5 mm |
| VMAX lettering | 15 mm high x 56 mm wide, raised 1 mm; centered in bottom flat land; font provisional |

Z0 is the bezel face; positive Z runs toward the rear. Lettering projects to Z-1. Including lettering, assembled depth is 50 mm to the one-piece cover, or 53 mm to the removable panel, excluding gaskets and connectors.

## Validation and remaining work

The generator checks each exported mesh for a single connected part, closed edges, consistent winding and non-degenerate triangles. Both nominal assemblies pass interference and port-clearance checks. It also checks the screen opening/chamfer, glass seat and retained rear-core geometry against the external-v1 baseline. The final drawings were rendered and visually inspected.

This is a design checkpoint, not a physically validated installation. Dash mounting brackets, real connector nuts/flanges, display/cable clearance, seals, retainer pads and mechanical/water testing remain open. NMEA cutout diameter 14.5 mm follows the requested 14-15 mm range; verify the received part before printing the final cover. Raised lettering changes the housing print setup: inspect slicer supports/orientation rather than using the old support-free face-down instructions.

The separate [protective-cover CAD V2](../indash-v2/README.md) targets a provisional 163 x 112 mm bezel. It is not a matched cover for this 170 x 130 mm checkpoint; reconcile those dimensions before combining the designs.

## Regeneration

Use Python 3.12 with `numpy`, `manifold3d==3.5.4` and `reportlab`. The CAD generator also reads the existing external-v1 generator and housing STL as its reference. These are included in the download package.

```powershell
python enclosure/CAD/generate_screwless_v2.py
python enclosure/CAD/draw_screwless_v2.py
```

The model generator uses `bezel_lettering.py` for shared polygon lettering. The drawing generator reads the drawing helpers at the start of `draw_screwless_concept.py`. Preserve those supporting files when moving the sources. Use Poppler to render the PDF after editing and inspect every changed sheet.
