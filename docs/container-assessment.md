# Container assessment — 2026-11-03

Read-only inspection of the borrowed Bingo Pi, currently driving Bluey's ESP.
No Docker installation, container launch, service restart, device handoff, or
kernel/boot change was performed for this assessment.

## Observations

- Raspberry Pi 4 Model B Rev 1.5, four cores, aarch64, Debian 12 Bookworm.
- 1.8 GiB visible RAM; about 1.4 GiB available; zero swap in use.
- Root disk 59 GiB, about 50 GiB available.
- PiVision RSS about 168 MiB, FFmpeg about 148 MiB, Operator about 3.1 MiB.
- PiVision lifetime CPU about 249% (2.49 cores). Short vmstat intervals showed
  67% userspace, 5% system, 27% idle across all four cores, with no swap activity.
  Compiler processes were no longer running during the sample. This is a short
  observation, not a sustained capacity benchmark.
- Temperature 73.5 C, throttled=0x0 at inspection.
- Docker/Podman executables absent; Docker service inactive.
- cgroup v2 exposes cpuset/cpu/io/pids, but /proc/cgroups reports memory disabled.
  Container memory caps cannot be assumed effective until memory cgroups are
  enabled and verified. Do not change boot settings/reboot before the demo.
- One CP2102 serial device is attached. No second physical arm was tested.

## Recommendation

Use native ARM64 Docker Engine/Compose, not emulated x86 images or a VM.
Two small Operator containers are a reasonable target; container overhead is
expected to be modest, but was not measured because the engine is not installed.
Two full-rate copies of the current PiVision workload would likely overcommit
the CPU. Start at lower inference cadence/resolution, limit inference thread
counts, or retain inference on the laptop. One inference service consuming both
camera feeds is an optimization to investigate after separate trackers work.

Use one Operator and one PiVision service per arm, with explicit device, profile,
port, camera source, and logs. Keep the current host media services initially.
Grant each Operator only its own serial device; avoid privileged containers.
Use stable short udev aliases, not ttyUSB numbering. Explicit port mappings can
give both Operator containers the same internal port but distinct host ports.
Host networking is optional and requires distinct ports in each process.

Use SIGINT as the container stop signal because current Operator handles it;
do not assume its existing signal handler handles Docker's default SIGTERM.
Native startup physically HOMEs the arm: do not enable unattended container
restart for that profile until its home/recovery policy is deliberately resolved.
Do not expose native startup during offline benchmarks: use PTYs/fakes and give
test containers no robot devices.

## Preserve the demo

Prepare Dockerfiles and Compose configuration locally. Build ARM64 images away
from the working Pi where possible. After the demo, enable/verify required
kernel support and install the engine in a planned maintenance window. Run an
offline, no-device comparison of native versus container CPU/RAM, then a second
vision workload with caps. Only after passing latency/watchdog and thermal checks
should one arm at a time hand off from the preserved host deployment to a
container. Keep original releases, configs, and startup paths for rollback.

Commander still needs robot IDs independent of hostname for two arms on one Pi.
Containers solve process/dependency isolation, not logical robot identity or
the CPU cost of running two camera/inference pipelines.

## Sources

- https://docs.docker.com/engine/install/debian/
- https://docs.docker.com/engine/containers/run/
- https://docs.docker.com/engine/containers/resource_constraints/
- https://docs.docker.com/engine/network/drivers/host/

## Pending profile work

Local Commander adds per-robot controller settings, reported-profile checks,
and controls appropriate to native versus ESP mode. Operator publishes PROFILE
metadata and honors configured socket ports; all three candidate CTest suites
pass, including two independent loopback listeners. Candidate code is under
/tmp/talos-esp-profile-20261103; the active ESP demo deployment is unchanged.
Gateway profile forwarding must be deployed before enabling Commander's new
verification requirement. Do not restart Commander onto that requirement until
Operator and PiVision both report the profile. The new UI bundle was built in
web/dist, but the running Python backend has not been restarted onto these changes.
