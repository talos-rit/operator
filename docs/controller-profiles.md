# Controller profiles

The same `erv` executable supports two explicitly selected controllers:

```sh
# Bingo: native ACL controller (default)
ERV_CONTROLLER_PROFILE=native ./build/bin/erv

# Bluey: current ESP one-axis-demo firmware, even on the borrowed Bingo Pi
ERV_CONTROLLER_PROFILE=esp-one-axis ./build/bin/erv
# Or use the launcher, which also refuses a second erv process:
sh tools/start-esp-one-axis.sh
```

An unknown profile fails before opening the serial device. No hostname-based
selection occurs. Native startup HOME, prompt arbitration, telemetry, and motion
handling retain their existing behavior.

## Current ESP firmware

The ESP profile targets esp-driver `one-axis-demo` at `6239820`. It does not need
new firmware, a `>` prompt, or serial replies. It does not send LISTPV, CON, SPEED,
DEFP, HERE, SHIFT, or MOVE, nor publish synthetic joint telemetry.

- Launch sends a blank line and `clrbuf`, without an automatic HOME.
- Continuous base jog uses `~` to enter manual mode, `q`/`1` for direction, and
  `~` followed by `clrbuf` to stop. Key repetition is at most 50 ms, inside the
  firmware's 100 ms key-silence timeout. Operator's 500 ms command-refresh
  watchdog is unchanged.
- `TrackingJog` (hardware operation 6) supports horizontal/base correction only.
- JointJogStop is accepted. Shoulder/elbow jog, altitude tracking, relative
  position moves, servo-enable, and variable speed are rejected and stop an
  existing jog. Firmware retains its configured fixed motor speed and limits.
- Discrete polar position moves are rejected. Select **Continuous** in Commander
  for manual base jog. Configure tracking for horizontal motion only.
- Explicit HOME sends only `HOME`; it may move the arm. Firmware provides no
  completion notification. Current firmware cannot abort homing through clrbuf;
  the UI's stop button must not be described as a homing E-stop.

Only Operator should read the serial device. Stop diagnostic `cat` readers before
a supervised runtime change. Do not flash the ESP or overwrite the old Operator
trees to select this profile. Until ESP telemetry is implemented and calibrated,
the web arm visualization has no measured pose to render.

## Verification

`EspProfile.SilentSerialSupportsJogStopWatchdogAndHomeWithoutNativeAcl` uses a
PTY with no controller responses. It exercises binary API parsing, serial start,
reverse/restart, stop, watchdog, hardware-operation tracking, unsupported-axis
rejection, and explicit HOME. This does not validate physical motor directions.

## Demo deployment, 2026-11-03

Built on the borrowed Pi and passed all three CTest suites. The candidate is
running in ESP mode from `/home/pi/operator-esp-demo-20261103`, independently of
the preserved `/home/pi/operator-dev` and `/home/pi/operator2/operator` trees.
At activation, exactly one erv process owned `/dev/ttyUSB0` and TCP 61616; the
two diagnostic cat readers were stopped. Startup logged only blank-line/stop
traffic, and the ESP replied to clrbuf. No HOME or physical motion test was sent.
The profile is selected for this process, not installed as a boot service.

The borrowed Pi is reachable from this laptop over IPv6, while its PiVision
gateway listens on IPv4 only. The demo uses a local SSH forward from
`127.0.0.1:15051` to the Pi's `127.0.0.1:5051`, and Commander's saved PiVision URL
is `http://127.0.0.1:15051`. Keep the SSH tunnel running for gateway control.
The robot config disables shoulder, elbow, and vertical tracking for this
base-only firmware. On 2026-11-03, Commander status confirmed Operator connected,
video available, and null gateway error/robot fault. Tracking was left disabled.
