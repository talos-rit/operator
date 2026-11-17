#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if pgrep -x erv >/dev/null; then
    echo 'An erv process is already running. Stop it before starting another.' >&2
    exit 1
fi
export ERV_CONTROLLER_PROFILE=esp-one-axis
exec "$root/build/bin/erv" "$@"
