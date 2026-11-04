# Stable Pi kernel/firmware prerequisite

The dual hardware-encoder profile was tested on Raspberry Pi 4, Debian Bookworm
ARM64, **6.12.109+rpt-rpi-v8**, Raspberry Pi firmware dated **2026-09-14**, and
**256 MiB CMA**. The previous custom 6.6.31-rt30 kernel and 64 MiB CMA configuration
faulted; see [dual-camera-incident.md](dual-camera-incident.md).

This is host maintenance, not an ESP flash or a container image update. Stop all
arm services first. Back up `/boot`, `/etc/default/raspi-firmware`, and initramfs
settings. Preserve the existing kernel and its modules for recovery.

Use the configured stable Raspberry Pi apt repository, not `rpi-update`/nightly:

```sh
sudo apt-get update
sudo apt-get install --only-upgrade raspi-firmware linux-image-rpi-v8
sudo dpkg --audit
```

On the original Pi, initramfs generation failed because MODULES=dep could not
resolve the root device. The tool's documented MODULES=most workaround was applied
in `/etc/initramfs-tools/conf.d/talos-modules`, then `sudo dpkg --configure -a`
completed. Do not reboot until package configuration succeeds and the matching
`kernel8.img` and `initramfs8` have been generated and checked.

The original `/boot/firmware/config.txt` pinned `kernel=kernel8_rt.img`, bypassing
installed stock kernels. It was changed under `[all]` to `kernel=kernel8.img`
with `auto_initramfs=1`. Preserve the RT image; do not delete it.

`/boot/firmware/cmdline.txt` must be **one line**. Preserve its existing root,
console, and filesystem parameters and append:

```text
cma=256M cgroup_enable=memory cgroup_memory=1
```

The original file had an ignored second line; adding CMA there did nothing.
Do not accidentally activate unrelated old ignored parameters while repairing it.
After a supervised reboot, verify actual runtime state:

```sh
uname -r
vcgencmd version
grep Cma /proc/meminfo
cat /sys/fs/cgroup/cgroup.controllers
sudo journalctl -k -b -p warning
```

Require CmaTotal >= 262144 kB before the hardware profile. Memory cgroups should
also be present before relying on container memory caps. Containers do not isolate
the host from a kernel driver fault. Start/test the camera encoders one at a time
before starting native Operator, whose startup physically HOMEs Bingo.
