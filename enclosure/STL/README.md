# STL

External enclosure draft 1: [parts and assembly instructions](external-v1/README.md).

Generated printable enclosure models belong here.

STLs are derived artifacts; editable CAD source should remain in `../CAD/`.

First print: [front-bezel-fit-test.stl](front-bezel-fit-test.stl).
See [print settings, dimensions, and fit instructions](front-bezel-fit-test.md).

Chamfered option: [front-bezel-fit-test-chamfered.stl](front-bezel-fit-test-chamfered.stl).
Adds a 0.8 mm, 45-degree bevel around the front screen opening. The original
glass pocket, locating rim, and overall dimensions are preserved. Print in
the same orientation and settings as the original. The original bezel was
physically tested by the user and reported to fit perfectly; the chamfered
version has passed digital mesh checks but awaits a physical print.

Editable source: `../CAD/front-bezel-fit-test-chamfered.scad`.
Generator: `../CAD/generate_front_bezel_chamfered.py`.

## Flush-dash prototype

Flush-dash parts: `fit_test.stl`, `body.stl`, `carrier.stl`, `cover.stl`, and `clamp.stl` (print four clamps). Start with the fit test and follow the [build guide](../README.md).
