# Two arms on one Pi

Use two Operator processes, each with a separate controller profile, USB device,
TCP port, and log. Each PiVision instance owns exactly one Operator connection.
Commander owns both logical robots and displays the two camera feeds.

| Logical robot | Controller | Stable serial alias | Operator TCP | PiVision HTTP |
| --- | --- | --- | --- | --- |
| Bingo | native | /dev/talos-bingo | 61616 | 5051 |
| Bluey | esp-one-axis | /dev/talos-bluey | 61617 | 5052 |

Operator already accepts `scorbot_dev` in its config. The profile work also makes
the executable honor `socket_port` and `socket_address`; previously Socket bound
61616 regardless of config. Tests cover two independent loopback listeners.

Example separate config files, in the existing `key: value` format:

```yaml
# bingo.conf
scorbot_dev: /dev/talos-bingo
socket_port: 61616
socket_address: 127.0.0.1
log_loc: /home/pi/logs/bingo-operator.log
```

Bluey's file uses `/dev/talos-bluey`, port 61617, and a separate log. Native
Operator startup physically HOMEs Bingo. Do not use it until Bingo's controller
is connected and its workspace is clear.

For the eventual two-instance service setup, launch the binary with its separate
environment/config, rather than the current single-arm convenience scripts:

```sh
ERV_CONTROLLER_PROFILE=native /path/to/erv /path/to/bingo.conf
ERV_CONTROLLER_PROFILE=esp-one-axis /path/to/erv /path/to/bluey.conf
```

The current convenience launchers deliberately refuse any existing erv process;
per-device/per-port service guards are still needed before enabling two arms.

PiVision already has `--operator-host`, `--operator-port`, `--host`, and `--port`.
Run one instance per camera initially, with distinct camera sources, HTTP ports,
and Operator ports. Check any other listeners/ports used by the deployed version.
The explicit IP/port pairing must remain consistent through SSH tunnels too.

## Work remaining before dual-arm use

1. Give Commander logical robot IDs/display names independent of `socket_host`.
   Config and connection lookup currently use hostname as the unique key;
   distinct ports alone do not create two independently saved robots on one host.
   Thread the ID through slots, API routes, tracking leases, and command receipts.
2. Expose each robot's serial-device selection through a restricted Pi management
   interface, using enumerated stable devices rather than arbitrary shell text.
   Assign udev aliases using USB serial numbers or physical USB paths. Never rely
   on ttyUSB0/ttyUSB1 ordering. The current device buffer/config also imposes short
   path limits; short stable aliases avoid that issue.
3. Install separate service instances with per-device ownership guards, log files,
   controller-profile environments, and restart procedures. One serial reader per
   device; one control client per Operator. Starting or restarting Bluey must not
   restart or home Bingo.
4. Measure two-camera/inference load, memory, temperature, dropped frames, and
   simultaneous watchdog/stop behavior. Motor communication is light; two model
   instances and camera pipelines are the likely resource bottleneck. Lower
   inference rates/resolution if needed, or later share inference while retaining
   independent per-camera tracking and per-arm control state.

This is an implementation plan and a software listener test, not verification
that two physical arms or two inference pipelines have run on this Pi.
