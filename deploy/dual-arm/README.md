# Two arms, one Pi

This stack runs three services per arm: Operator, USB-camera streaming, and
PiVision. Commander remains the shared desktop UI. The default is **hardware
H.264 at 1280x720/10 video FPS**, with PiVision working frames reduced independently
to 640 pixels wide, model input 192x192, and inference capped at 2 FPS per arm.

**Host prerequisite:** the tested stable Pi kernel/firmware and at least 256 MiB
CMA. See [stable-pi-upgrade.md](../../docs/stable-pi-upgrade.md). The old custom RT
kernel/64 MiB CMA combination faulted. `check-host.sh` blocks that configuration.
The tested `compose.software.yaml` fallback is 640x480/10 FPS libx264 and needs
no hardware-codec device. See [incident details](../../docs/dual-camera-incident.md).

| Robot | Profile | Serial alias | Camera alias | Operator | PiVision | RTSP path |
|---|---|---|---|---:|---:|---|
| Bluey | esp-one-axis | /dev/talos-bluey-serial | /dev/talos-bluey-camera | 61616 | 5051 | /camera |
| Bingo | native | /dev/talos-bingo-serial | /dev/talos-bingo-camera | 61617 | 5052 | /bingo |

The Pi's hostname can be `bingo.local` for both rows. Logical robot names, not
the hostname, identify arms in Commander. Do not assign both services the same
serial device.

## Pull and build

Both repositories use branch `feat/dual-arm-containers`. This directory's
`commander.ref` pins the compatible Commander commit. `prepare.sh` can clone that
branch into its private cache, or take a matching existing checkout as argument.
It does not change an existing checkout's branch or files.

On the Pi, use **64-bit Debian Bookworm and Python 3.11**, working Docker Engine
with Compose, and the existing MediaMTX service on port 8554 with publisher paths
enabled. New machines need these build prerequisites once:

```sh
sudo apt-get install -y git cmake build-essential pkg-config libcpputest-dev python3-venv
git clone -b feat/dual-arm-containers https://github.com/talos-rit/operator.git
cd operator/deploy/dual-arm
sh prepare.sh
```

`prepare.sh` builds/tests Operator, installs pinned inference dependencies into a
private venv, and builds the image. It includes the exported 192px pose model from
Commander; no model/venv/binary from the developer's laptop is required. Internet
access is needed for initial dependencies/images. If the Pi is offline, arrange
a package-download proxy or prebuilt image transfer first.

The demo Pi uses an isolated Docker runtime at `/run/talos-docker.sock` to avoid
altering the host's network/firewall setup. `stack.sh` detects it; other machines
use standard `sudo docker compose`. `install-runtime.sh` and `download_proxy.py`
describe the original offline bootstrap, not a requirement to replace an existing
Docker installation. Docker is not boot-enabled by this rollout.

## USB assignment and startup

The supplied udev rules match **this Pi's physical USB topology**. Bluey's ESP
adapter is port 1.1 and its camera is 1.2. Bingo's camera is 1.3 and native PL2303
adapter is 1.4. Review `udevadm info` and adjust the rules on a differently wired
Pi before starting. Rules do not start containers or move an arm.

```sh
sh install-usb-rules.sh
ls -l /dev/talos-*
```

Before a handoff, stop any host Operator/camera/PiVision instances that own the
same USB devices or ports. The original demo service definitions and releases
are retained. `activate-bluey.sh` implements the original demo host-to-container
handoff; on a clean machine simply start the Bluey profile after ownership checks:

```sh
mkdir -p logs/bluey logs/bingo
sh stack.sh --profile bluey up -d
# Native startup physically HOMEs Bingo. Connect its controller, power it,
# clear the workspace, then explicitly acknowledge that motion:
sh activate-bingo.sh --confirm-home
sh stack.sh ps
```

Containers have `restart: no`, and Operator uses SIGINT for shutdown. Do not
enable unattended restart for the native profile without resolving automatic
homing behavior. Each camera has its own USB device. The hardware profile additionally grants
/dev/video11 to both camera encoders; the software fallback does not. No privileged
containers are used. Use only the upgraded/validated host for the hardware profile.

## Commander

Pull the pinned Commander branch/commit on the desktop, install its normal Python
dependencies, then build the web UI (`npm --prefix web install` followed by
`npm --prefix web run build`). Start `commander-web` normally.

In Settings, create two entries:

- Robot name **bluey**, Operator host **bingo.local**, port **61616**, profile
  **Bluey · ESP one-axis demo**, camera `rtsp://bingo.local:8554/camera`, PiVision
  enabled at `http://bingo.local:5051`.
- Robot name **bingo**, same Operator host, port **61617**, profile **Bingo · native
  controller**, camera `rtsp://bingo.local:8554/bingo`, PiVision enabled at
  `http://bingo.local:5052`.

Assign these to Feed 1 and Feed 2. USB device selection is fixed in the Pi's
aliases/Compose file; changing a desktop profile does not silently rewire USB or
restart an Operator. A reported profile mismatch/unverified profile blocks motion
and leaves Stop available. Both gateways support IPv4 and IPv6 without a tunnel.

## Logs, limits, and recovery

```sh
sh stack.sh logs --tail 50 bluey-camera bingo-camera bluey-vision bingo-vision
tail -n 50 logs/bingo/operator.log
sudo journalctl -k -b -p warning
sh stack.sh stop
# Camera-only fallback, without restarting either Operator:
sh stack.sh -f compose.software.yaml up -d --no-deps bluey-camera bingo-camera
```

`rollback-bluey.sh` restores the preserved demo host services. It is specific to
the original Pi paths and does not overwrite their files. Keep a backup of desktop
local YAML before migrating robot names. There is no automatic native restart.

The original custom kernel disabled memory cgroups. The stable-kernel setup enables
that controller; verify it before relying on caps. The profiles now limit each
vision worker to 512 MiB, camera to 128 MiB, and Operator to 32 MiB, with CPU/PID
caps as well. Docker memory stats are meaningful on the updated host.

Verified: both simultaneous camera feeds, both Operator profiles, Bingo startup
HOME followed by live telemetry, and approximately 2 FPS per vision worker with
no new codec crash or throttling during the short software-encoding test. This is
not a long soak test. Autonomous following at this low inference rate has not
been physically validated: observation age can exceed the unchanged freshness
threshold and correctly pause motion. Do not raise that threshold to hide it.

Nothing in this branch flashes the ESP. Its demo profile remains base-only and
does not fabricate full-arm telemetry.

## Camera bypass option

Both webcams advertise MJPEG and YUYV, not native H.264. MediaMTX accepted an
experimental 720p native-MJPEG RTSP publisher using `-c:v copy` at about 3% of a
core. However, the desktop decoder reported packet/decode warnings, so this is
not the supported default. Keeping high-resolution RTSP publication separate
from lower-resolution/half-rate PiVision is already supported without bypass.

## Tracking both arms

Select Bluey and turn tracking on, then select Bingo and turn tracking on.
Each arm retains its own expiring control lease when selection changes.
Manual jog or HOME disarms only that robot; disable tracking separately per arm.
Bluey's current ESP profile follows horizontally only. Restarting PiVision leaves
tracking off until explicitly enabled again. The 192px pose model and one thread
per worker measured approximately 2 FPS per arm; distant-person accuracy may be
lower than the 320px model. Stale-video stops and the Operator watchdog remain
unchanged. Simultaneous physical following still needs a supervised demo test.
