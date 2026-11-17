#!/bin/sh
set -eu
cd "$(dirname "$0")"
if [ "${1:-}" != '--confirm-home' ]; then
    echo 'Bingo native Operator startup physically HOMEs the arm.' >&2
    echo 'Connect the native serial adapter and camera to the two reserved USB ports.' >&2
    echo 'After clearing the workspace, run this script with --confirm-home.' >&2
    exit 1
fi
test -c /dev/talos-bingo-serial
test -c /dev/talos-bingo-camera
test "$(readlink -f /dev/talos-bingo-serial)" != "$(readlink -f /dev/talos-bluey-serial)"
test "$(readlink -f /dev/talos-bingo-camera)" != "$(readlink -f /dev/talos-bluey-camera)"
if sudo -n fuser /dev/talos-bingo-serial /dev/talos-bingo-camera >/dev/null 2>&1; then
    echo 'Bingo USB devices are already owned by another process; refusing to start.' >&2
    exit 1
fi
TALOS_CONFIRM_NATIVE_HOME=1 sh stack.sh --profile bingo up -d
