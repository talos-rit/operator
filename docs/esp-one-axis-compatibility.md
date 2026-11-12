# Bluey ESP compatibility inspection — 2026-11-03

User-confirmed topology: Bingo uses the native controller; Bluey uses the ESP
replacement controller. Bluey's Pi is dead, so Bingo's Pi (`bingo.local`) is
temporarily attached to Bluey. Hostname does not identify the hardware backend.

ESP source inspected: `origin/one-axis-demo`, tip
`6239820faf579a834f2bdbcd7633af1981f15a9c` in talos-rit/esp-driver.

- `components/pi_link/pi_link.c` is a limited ACL compatibility bridge on UART0,
  9600 baud by default. `~` toggles manual mode; `q`/`1` drive base positive/negative;
  `4`/`r` map altitude but the demo enables only one active axis (azimuth).
- Only `HOME` and `clrbuf` have direct-line implementations. HERE/DEFP/SHIFT/MOVE
  are deliberately ignored. No LISTPV/SHOW ENCO implementation or `>` prompt.
- Manual-key silence stops motion after 100 ms. Repeated starts occur at 250 ms.
  Firmware retains its own motor, homing, and soft-limit handling.
- Debug `line [...]` output matches the supplied log, but is not measured pose.

Pi read-only inspection via the existing Bluey SSH identity:

- `/home/pi/operator2/operator`: HEAD `bb8dbe4`; tracked local change only swaps
  wrist-pitch `4`/`r` direction mapping. Also has untracked Obsidian metadata.
  No evidence of a separate ESP-specific backend in this checkout.
- At inspection, PID 933 owned TCP 61616 and ran
  `/home/pi/operator-dev-685b144-serialfix/build/bin/erv`; `/home/pi/operator-dev`
  points there. Its binary includes hardware-operation handling. Its manifest
  claims source `edb9e6a`, clean. This is not the old operator2 binary.
- Two `cat /dev/ttyUSB0` processes (804, 932) also held the serial device, alongside
  erv. They can consume replies that erv needs. No processes were stopped.

The supplied command-8 rejection is consistent with the old operator2 parser,
not the currently observed newer parser. Historical log origin/runtime switching
has not been established.

Our newer Scorbot code is not a drop-in ESP bridge: it starts with direct_mode
false, waits for `>`, gates queued commands/manual entry on that state, and polls
LISTPV. The ESP emits no such prompt or telemetry. Supporting command 8 alone
does not solve compatibility or create support for other physical axes.

Recommended implementation: explicitly select native ACL versus ESP-one-axis
backend/profile. Keep native prompt/telemetry arbitration; provide an ESP path
without unsupported polling/prompt dependencies and expose only its implemented
operations. Keep firmware watchdog/limits, verify key directions, and use one
serial reader. Full-arm visualization must remain unavailable until genuine
ESP telemetry and its own calibration are implemented.

No restart, deployment, firmware flash, or motion command was performed.
