#!/bin/sh
# User-run deployment of the already-built, serial-regression-tested jog fix.
# Native Operator startup runs HOME. This never clears an impact fault or replays a jog.
set -eu
mode=${1:---check}
case "$mode" in
    --check|--confirm-home) ;;
    *) echo 'Usage: sh deploy-bingo-jog-fix.sh --check | --confirm-home' >&2; exit 1 ;;
esac
test "$(hostname)" = bingo || { echo 'This deployment is for Bingo, not this host.' >&2; exit 1; }
base=/home/pi/talos-containers/two-pi-20261104
image=talos-arms:continuous-jog-20261104
expected=sha256:33b61baf436c3a7d339f264bc8be2f0ee00834ecbd45194afe36517420156afd
expected_binary=94779d923b098efc0153a9a7999ecb26d984475af7c2ee06fe34eacec121cf8c
docker() { sudo -n /opt/talos-docker/29.8.2/docker -H unix:///run/talos-docker.sock "$@"; }
test -f "$base/stack.sh"
test -S /run/talos-docker.sock
test -c /dev/talos-bingo-serial
test "$(docker image inspect "$image" --format '{{.Id}}')" = "$expected" || {
    echo 'Candidate image does not match the tested image; refusing deployment.' >&2; exit 1;
}
binary=$(docker run --rm --network none --entrypoint sha256sum "$image" /opt/talos/erv)
test "${binary%% *}" = "$expected_binary" || { echo 'Candidate binary checksum mismatch.' >&2; exit 1; }
old=$(docker inspect talos-arms-bingo-operator-1 --format '{{.Image}}')
echo "Current Operator image: $old"
echo "Tested candidate image: $expected"
if [ "$mode" = --check ]; then
    echo 'Preflight passed. No robot services were stopped or restarted.'
    echo 'To deploy: release all controls, clear the workspace, then rerun with --confirm-home.'
    exit 0
fi
if [ "$old" = "$expected" ]; then
    echo 'The tested Operator image is already deployed. No restart needed.'
    exit 0
fi
echo 'Deploying now. Native Operator startup will HOME Bingo. Tracking will remain OFF.'
# Stop autonomous control before touching the serial owner. Never send CON or
# a recovery jog; controller faults remain latched for explicit inspection.
python3 - <<'PY'
import json, urllib.request, urllib.error
url = 'http://127.0.0.1:5052/api/v1/control/'
state = json.load(urllib.request.urlopen(url + 'status', timeout=3))
owner = state.get('owner') or 'manual-jog-fix-deployment'
STOP_COMMAND = 4  # ICD PolarPanStop; command 2 is HOME and requires a payload.
for path, method, body in (
    ('lease', 'PUT', {'owner': owner, 'enabled': False}),
    ('commands', 'POST', {'command': STOP_COMMAND, 'payload_hex': ''}),
):
    request = urllib.request.Request(url + path, data=json.dumps(body).encode(),
        method=method, headers={'Content-Type': 'application/json'})
    try:
        with urllib.request.urlopen(request, timeout=3) as response:
            response.read()
    except urllib.error.HTTPError as error:
        detail = error.read().decode(errors='replace')
        raise SystemExit(f'{method} {path} failed ({error.code}): {detail}. Operator was not restarted.')
PY
docker tag "$old" talos-arms:before-jog-stop-20261104
printf '%s\n' "$old" > "$base/jog-fix-previous-image.txt"
cd "$base"
sh stack.sh stop bingo-operator
if sudo -n fuser /dev/talos-bingo-serial >/dev/null 2>&1; then
    echo 'Serial adapter still has an owner. Operator left stopped; no restart attempted.' >&2
    exit 1
fi
docker tag "$image" talos-arms:demo-v1
TALOS_INFERENCE_FPS=4 TALOS_CONFIRM_NATIVE_HOME=1 sh stack.sh up -d --no-deps bingo-operator
test "$(docker inspect talos-arms-bingo-operator-1 --format '{{.Image}}')" = "$expected"
echo 'Jog fix installed. HOME may still be running; wait for live joint telemetry.'
echo 'Tracking remains OFF. Refresh Commander before testing a new jog press.'
echo "Previous image retained as talos-arms:before-jog-stop-20261104 ($old)."
