# Dual-camera freeze: 2026-11-03

The previous boot's kernel log establishes this sequence:

1. FFmpeg's hardware encoder failed repeated CMA allocations (`req-size: 338
   pages, ret: -12`), followed by a DMA page-allocation failure.
2. The MMAL/bcm2835 codec path faulted while enabling a video port.
3. Codec release blocked on a mutex and the kernel reported RCU/CPU stalls.
   Network access subsequently stopped; the user power-cycled the Pi.

The Pi reserves only 64 MiB of contiguous CMA memory. Ordinary available RAM
does not guarantee an allocation from that pool. The failed allocation is
observed; the exact reason the pool could not satisfy it (exhaustion versus
fragmentation) was not determined. Starting/reconfiguring the second 720p
hardware-encoder pipeline was the trigger in this run. Containers share the host
kernel, so this driver fault could freeze the host as well.

There was no observed throttling before the failure, and normal available RAM
was about 1.2 GiB. This is not evidence that CPU, generic OOM, or thermal load
caused this fault. Cooling still improves operating headroom.

The deployment now avoids that driver: cameras supply MJPEG at 640x480; FFmpeg
uses libx264 ultrafast/zerolatency at 10 video FPS, with one encoder thread.
Only each USB camera is passed to its container, not /dev/video11. PiVision
inference is capped separately at 2 FPS. The first simultaneous run measured
about 27-28% of one CPU core per encoder. This is a reduced-resolution workload,
not a claim that two full-resolution software encoders fit this Pi.

Hardware encoding can be revisited by sizing CMA deliberately and testing a
supported kernel/firmware combination. Do not repeat the known-failing
two-encoder configuration as the demo default. No CMA/boot/kernel change was
made in this rollout.

A related upstream fault signature is documented at
https://github.com/raspberrypi/linux/issues/4920; it supports the failure-path
comparison, not proof that the exact upstream issue or fix applies here.

## Recovery and hardware retest

The Pi was upgraded through stable apt packages to kernel 6.12.109+rpt-rpi-v8
and firmware dated 2026-09-14. The explicit custom-RT kernel override was removed
in favor of the stock image, with backups retained. A two-line kernel command file
had silently ignored the new CMA argument; fixing it to one line made CmaTotal
262144 kB. Memory cgroups are now enabled too.

Two hardware H.264 streams subsequently ran with about 206 MiB CMA still free,
without the previous allocation fault or RCU stall during the short retest. Input
is MJPEG720p from both cameras, decoded and filtered to 10 FPS for the hardware
encoders. PiVision resolution/rate are separate from published stream resolution.
The software fallback remains available. This avoids claiming that a particular
upstream patch alone fixed the fault: both the kernel/firmware and CMA changed.
