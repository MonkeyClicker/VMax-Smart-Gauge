# Proprietary payload research - October 14, 2026

## Result

All five proprietary PGNs in the October 13 engine + Garmin raw capture were
investigated. No additional engine measurement has yet been validated. PGN 65344
contains a promising warming signal, but neither its units nor sensor identity
are established. The standard engine measurements already decoded from 127488
and 127489 remain the confirmed starting point.

This research used saved data only; it did not change firmware, enable CAN
transmission, or send requests to the boat network. No captures were uploaded to
external services. Public searches used PGN numbers and protocol terms.

Capture: [engine + Garmin raw log](../captures/engine-garmin-raw-1min-20261013T220900Z.log).
Supporting [byte inventory and complete proprietary messages](proprietary-payload-research-20261014.json).
The JSON preserves timestamps, raw bytes, source addresses, structural matches,
and source revision identifiers; hypotheses are not written as calibrated sensors.

## Confirmed manufacturer headers

The four engine-side PGNs (65314, 65344, 65345, 65346), all source 2, begin with
`46 9F`. Interpreting the standard 11-bit manufacturer and 3-bit industry header
identifies manufacturer 1862 (Yamaha Marine), industry 4 (marine). Byte 2 is 01
throughout, consistent with the engine instance 1 seen in standard engine PGNs;
its proprietary field identity is not independently documented.

Garmin PGN 126720, source 0, reassembles with header `E5 98`: manufacturer 229,
industry 4. Manufacturer headers identify the payload namespace, not necessarily
which physical gateway originated or forwarded it.

## Public-definition traps

The checked upstream database calls one 65314 variant Yamaha Gear Status, but
requires manufacturer 198. It calls a 65344 variant Yamaha Engine Data B, also
requiring 198. Manufacturer 198 is listed separately as Mystic Valley
Communications; our recorded header is 1862. Whether the legacy definitions are
wrong or describe another format is unresolved. Their labels and offsets cannot
be accepted merely because the PGN number matches.

The checked 65345 definition is Raymarine (1851); 65346 is Yanmar (172). Neither
matches the Yamaha payloads. PGN numbers in the proprietary range are reused by
manufacturers. The upstream engine definitions are explicitly incomplete.

## Engine-side payload results

All byte positions below are zero based. Multi-byte numeric probes use little
endian ordering. They are raw integers unless a hypothesis is explicitly stated.

| PGN | Frames | Distinct payloads | Recorded structure | Finding |
|---:|---:|---:|---|---|
| 65314 | 548 | 94 | `46 9F 01 E0 xx xx FF FF`; bytes 4-5 = 2844-3645 | Variable engine-related quantity; meaning/units unknown |
| 65344 | 120 | 8 | `46 9F 01 FF FF 7F xx 0C`; bytes 6-7 = 3232-3302 | Monotonic warming-like signal; strongest additional-sensor candidate |
| 65345 | 60 | 1 | `46 9F 01 FF FF FF 00 00` | Constant in this test; sensor/status meanings unknown |
| 65346 | 60 | 1 | `46 9F 01 E0 C0 00 00 FF` | Constant in this test; sensor/status meanings unknown |

### PGN 65314

The published gear-status issue reports a neutral indicator at byte 5, bit 1.
For our incompatible-header payload, that bit is 1 in 488 frames and 0 in 60.
There is no synchronized gear-position record to validate it. It must not be
presented as neutral/forward/reverse.

The varying 16-bit word at bytes 4-5 correlates with the nearest standard rapid
engine boost-pressure value (Pearson r about 0.875 across 548 pairs within 200 ms),
and inversely with RPM (r about -0.779). This does not establish pressure, gear,
voltage, throttle position, or any other physical meaning. Startup transients and
common timing can produce such correlations.

A proposed voltage scaling raw/256 failed: median absolute difference from
nearby standard alternator voltage was about 2.99 V (pairs within 600 ms), with
negative correlation. That particular interpretation is unsupported.

### PGN 65344

Bytes 6-7 increase from 3232 to 3302 in steps of 10. If, and only if, they were
0.1 kelvin units, the range would be 50.05-57.05 C (122.09-134.69 F).
This is an unverified decoding hypothesis, not a measured temperature claim.
Other scales and physical meanings remain possible.

The raw word correlates with independently decoded coolant temperature at about
r=0.888 (120 nearest message pairs within 600 ms). Standard coolant was 46-48 C,
so the hypothetical proprietary temperature is not simply the same numeric
reading. A different engine/head/oil sensor is possible, but none is confirmed.
A common rising time trend is a major confounder. No new oil-temperature reading
should be displayed on this evidence.

### PGNs 65345 and 65346

Both are constant for all recorded frames. All-ones bytes can resemble unavailable
or reserved values, but proprietary field boundaries are unknown, so they cannot
be labeled NA using standard field sentinels alone. Likewise zeros do not prove
absence of alarms, correct neutral state, or an actual zero measurement.

## Garmin PGN 126720

Strict sequence-ordered fast-packet reassembly yielded 117 complete messages.
Only complete messages were used for subtype identification. The initial
upstream-only search had no matches. Further research found an independent
proprietary-decoder branch that supplies six structural header matches:

| Subtype | Complete messages | Payload bytes | Research-branch structural description |
|---:|---:|---:|---|
| 1502 / 0x05DE | 1 | 14 | No exact variant match; shares a transport family with published display-mode variants |
| 1633 / 0x0661 | 17 | 42 | GPSMAP data-sync record B; per-sensor field semantics still unspecified |
| 2272 / 0x08E0 | 36 | 14 | Display identity/version handshake; short record |
| 2279 / 0x08E7 | 32 | 17 | Device-presence / master-advertise beacon |
| 2557 / 0x09FD | 4 | 14 | Configuration-table descriptor beacon |
| 6471 / 0x1947 | 11 | 39 | Node-status report, labeled GHC in the research branch |
| 6472 / 0x1948 | 16 | 24 | Self-status beacon, labeled GHC in the research branch |

These are incomplete third-party research definitions, not Garmin documentation.
Exact manufacturer/subtype/wrapper discriminator matching supports a structural
classification, not every explanation of the body. Names containing GHC do not
prove a GHC/autopilot is installed; these captures came from the user's Garmin
network. The 2272 short form is 14 bytes; the research definition's 24-byte long
form contains optional trailing fields that are absent here.

Recorded checks:

- Subtype 1633 has one identical 42-byte payload across its 17 complete messages.
  Several all-ones/signed-maximum patterns are present, but field-level semantics
  are not known. No additional live engine measurement was identified.
- Subtype 2272's proposed version word contains 2024 and 4048. The latter makes
  a simple calendar-year interpretation unsafe. Keep these as raw codes.
- Subtype 2279 contains a constant body. Its protocol word differs from the
  example constant described by the research branch; that description is not a
  validated literal invariant for our device.
- Subtype 2557 has descriptor count 1 and type 6 under the research layout.
- Subtypes 6471/6472 contain the same candidate node ID, 480248199. The varying
  self-status word remains an unidentified status/counter, not an engine fault
  code or sensor value.
- Subtype 1502 was `E598DE05050500000C004ED0A63E`. It does not satisfy the exact
  published day/night/color-mode discriminator values, so no mode is assigned.

## What additional engine data is actually established?

None from these proprietary fields yet. In particular, oil temperature, coolant
pressure, fuel pressure, load and torque have not been recovered. Their standard
127489 fields were encoded unavailable in this capture. That does not prove the
engine has no such sensors or never transmits them elsewhere.

The already decoded standard messages supply RPM, trim, the standard boost
pressure field, oil pressure, coolant temperature, alternator voltage, fuel flow,
and engine hours. Their physical accuracy and pressure interpretation should be
compared with existing instruments before product use.

## Most useful next evidence

1. Resolve or substantially reduce CAN errors first; raw logging cannot recover
   malformed/rejected frames. Serial-record integrity passed for this capture,
   but some fast-packet streams were still incomplete.
2. Capture a labeled cold-to-warm session with Garmin/Yamaha instrument readings
   noted at matching times, especially engine temperature and any oil/head
   temperature that the existing instruments actually display.
3. Record ignition transitions and steady idle separately. Repeat on another
   session so temperature-like trends can be distinguished from uptime/counters.
4. Compare safely available, explicitly documented gear/trim states; do not infer
   gear from engine speed or a database label with the wrong manufacturer.
5. Confirm each candidate's scale, offset, sentinel values, and sensor identity
   before adding it to the gauge. Retain raw bytes and complete-message checks.

## Sources and reproducibility

Research date: October 14, 2026. Primary project sources only were used for field
claims; general forum results were not treated as decoder specifications.

- [Upstream CANboat manufacturer codes](https://github.com/canboat/canboat/blob/36ff977f1faf6257de53887ff4298f4fe312780b/database/lookups/MANUFACTURER_CODE.yaml)
- [Upstream Yamaha gear definition](https://github.com/canboat/canboat/blob/36ff977f1faf6257de53887ff4298f4fe312780b/database/pgns/065314-yamahaGearStatus.yaml)
- [Original Yamaha gear-status report](https://github.com/canboat/canboat/issues/603)
- [Upstream Engine Data B placeholder](https://github.com/canboat/canboat/blob/36ff977f1faf6257de53887ff4298f4fe312780b/database/pgns/065344-yamahaEngineDataB.yaml)
- [Upstream PGN database](https://github.com/canboat/canboat/blob/36ff977f1faf6257de53887ff4298f4fe312780b/docs/canboat.json)
- [Independent Garmin research branch](https://github.com/pete-jorgensen/canboat/tree/713e3e0e1d5474763207a5116d9fbdcd7376a1f2)
- [Independent branch PGN definitions](https://github.com/pete-jorgensen/canboat/blob/713e3e0e1d5474763207a5116d9fbdcd7376a1f2/docs/canboat.json)

Capture SHA-256: `cf63f59c8b42b03dd3694e159e75d1638ff7617cec7e444b324c5c04f675f782`.
Supporting JSON stores the raw Yamaha samples and 117 complete Garmin messages.
Counts and manufacturer headers were cross-checked against the raw capture.
