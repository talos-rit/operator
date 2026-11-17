#!/bin/sh
set -eu
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
arm=${1:-}
# One worker per Pi can use a higher cap than the shared-Pi deployment.
export TALOS_INFERENCE_FPS=${TALOS_INFERENCE_FPS:-4}
case "$arm" in
    bluey) test "$#" -eq 1 ;;
    bingo)
        if [ "$#" -ne 2 ] || [ "$2" != --confirm-home ]; then
            echo 'Native startup HOMEs Bingo. Clear the workspace, then use: sh start-arm.sh bingo --confirm-home' >&2
            exit 1
        fi
        export TALOS_CONFIRM_NATIVE_HOME=1
        ;;
    *) echo 'Usage: sh start-arm.sh bluey | sh start-arm.sh bingo --confirm-home' >&2; exit 1 ;;
esac
for kind in serial camera; do
    test -c "/dev/talos-$arm-$kind" || {
        echo "Missing /dev/talos-$arm-$kind. Check USB connections and install-usb-rules.sh." >&2
        exit 1
    }
done
mkdir -p "$here/logs/$arm"
exec sh "$here/stack.sh" --profile "$arm" up -d
