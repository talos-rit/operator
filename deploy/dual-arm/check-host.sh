#!/bin/sh
set -eu
test "$(uname -m)" = aarch64 || { echo 'Hardware profile requires an ARM64 Pi.' >&2; exit 1; }
cma=$(awk '/^CmaTotal:/ {print $2}' /proc/meminfo)
if [ "${cma:-0}" -lt 262144 ]; then
    echo 'Hardware profile requires at least 256 MiB CMA. See docs/stable-pi-upgrade.md.' >&2
    echo 'Use compose.software.yaml for the tested 640x480 software fallback.' >&2
    exit 1
fi
case $(uname -r) in
    6.6.31-rt*) echo 'Refusing the kernel that faulted in the dual-codec test.' >&2; exit 1 ;;
esac
test -c /dev/video11 || { echo 'Pi hardware encoder /dev/video11 is missing.' >&2; exit 1; }
