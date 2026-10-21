# USB-C panel extension — ordered

BOM ID: **USB-01**, quantity **1**. User confirmed ordered on 2026-10-20.
Receipt and physical testing are pending.

## Selected item

User-provided screenshot shows STA branding, a title beginning EP-TYPE-C Panel
Female Socket, and selected variant **FPC-10cm**, Line Connector, 10P.
The photo shows a threaded USB-C female panel receptacle connected to an
internal USB-C male plug by flexible printed ribbon. Nominal length is **10 cm**
from the variant label; the generic title says 30 cm, so verify on delivery.

Source evidence: user screenshot
`codex-clipboard-d12e277c-1f74-4b69-b939-41d4f4b501a5.png` and subsequent order
confirmation in this project chat. The screenshot is not stored in the repo.
A verified USB product URL is not yet recorded; the ambient browser URL points
to the separate NMEA connector and must not be used for this USB component.

## Intended use and CAD reference

### Provisional dimensional reference supplied by user

On 2026-10-20 the user supplied references including the
[Atlavox ribbon bulkhead extension](https://atlavox.com/products/usb-c-waterproof-bulkhead-extension-ribbon-cable)
and [AliExpress item 1005007792961281](https://www.aliexpress.com/item/1005007792961281.html).
The latter is a user-supplied candidate USB listing URL; direct retrieval failed,
so correspondence to the purchased STA variant is not independently verified.

Atlavox explicitly lists a **5/8 inch (15.8 mm)** mounting hole for its CAZN
extension, with USB-C female/male ends, 100/200 mm ribbon options, USB 3.1 and
IP67. It describes an O-ring, mounting nut and gasketed threaded cap. These
are specifications of the Atlavox product, not proof of the purchased STA
part's dimensions or included accessories.

Use **15.8 mm only as a provisional layout reference**, pending confirmation
on the ordered connector. Exactly 5/8 inch is 15.875 mm. The user's collected
notes also call the housing M16; a 15.8 mm hole cannot be assumed to clear a
16 mm mounting thread. Confirm whether M16 describes the mounting thread or
another thread before selecting the final hole diameter and print allowance.

Other collected claims (PA66, contact plating, 1.5-3 A, 24/30 V, insulation,
500 cycles and temperature range) span multiple listings and are not verified
for USB-01. The connector's voltage rating does not authorize applying that
voltage to the Waveshare USB input; retain regulated 5 V USB power.

Provide external USB power/programming access with the internal male plug
connected to the Waveshare. Existing regulated 5 V battery USB power remains
the plan. User assessed the item as meeting requirements; bench confirmation
of power and data is pending.

Mount in the replaceable rear cable panel alongside the NMEA connector.
The panel is currently 3 mm thick and its STL remains blank. Measure before
finalizing the hole and connector location:

- mounting hole, antirotation flats, usable thread length and panel thickness limit
- flange/seal diameter, nut size and tool clearance
- inward/outward projections including the plugged-in cable
- actual ribbon reach, plug orientation and routing without tension or sharp folds
- supplied seals/cap and protection with an ordinary USB cable connected

Mounting dimensions, rated current, USB data specification and ingress rating
are unverified. The screenshot's AI overview is not specification evidence for
IP67/IP68, current capacity, temperature or durability. Do not derive a hole
size from the photo or the NMEA connector's dimensions.

On receipt, bench-check 5 V power, enumeration and firmware upload through the
extension using the intended cables. Check reach and validate seals before
exposed boat use.
