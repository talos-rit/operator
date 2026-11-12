#!/bin/sh
set -eu
if [ "${1:-}" != '--confirm-home' ]; then
    echo 'Native Operator startup physically HOMEs the arm.' >&2
    echo 'Connect the native controller, clear the workspace, then run with --confirm-home.' >&2
    exit 1
fi
shift
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if pgrep -x erv >/dev/null; then
    echo 'An erv process is already running. Stop it before changing controller.' >&2
    exit 1
fi
export ERV_CONTROLLER_PROFILE=native
exec "$root/build/bin/erv" "$@"
