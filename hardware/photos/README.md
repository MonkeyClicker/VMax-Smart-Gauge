# Waveshare board reference photos

User-provided photos of the ESP32-S3-Touch-LCD-5 Rev 1.1 board.

- [Rear board and terminal labels](20260821_164633.jpg)
- [Front display](20260821_164657.jpg)

## Scanner connections

With the rear board oriented as in the first photo (USB at the top, long green
terminal strip on the left), connect a standard-color NMEA 2000 pigtail as follows.
Confirm the actual cable labels before relying on colors.

| Cable wire | Board terminal |
|---|---|
| White / NET-H | CAN_H |
| Blue / NET-L | CAN_L |
| Black / NET-C | GND immediately above SDA (below VOUT) |
| Red / NET-S | Disconnected and separately insulated; scanner uses USB power |
| Bare shield/drain | Insulated at scanner end for this prototype |

CAN_L is immediately above CAN_H. Use the printed labels, not a guessed connector
orientation. Do not use DI_COM for the network ground.

The green connector has screw clamps. With USB disconnected and network power
turned off, loosen the relevant screw, insert the stripped conductor into the
corresponding wire-entry opening, and tighten. Leave no exposed copper outside
the clamp; tug gently to verify retention. Provide cable strain relief.

The two-position termination switch is below the terminal strip in the photo,
marked CAN / RS485 and OFF / ON. Set the CAN switch to OFF for a normal NMEA
2000 drop. The separate battery switch is not the CAN termination switch.

Connect the Micro-C drop to the spare backbone T and restore network and USB
power. The scanner should distinguish running software from received traffic.
These photos document the board before wiring; they do not verify network reception.

See [prototype wiring](../../docs/Wiring.md) for the overall network topology.
