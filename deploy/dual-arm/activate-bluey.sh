#!/bin/sh
set -eu
cd "$(dirname "$0")"
test -c /dev/talos-bluey-serial
test -c /dev/talos-bluey-camera
test -c /dev/video11
# Read-only preflight: do not take over an armed tracking session.
/home/pi/tracker_venv/bin/python - <<'PY'
import json, urllib.request
with urllib.request.urlopen('http://127.0.0.1:5051/api/v1/control/status', timeout=3) as response:
    status = json.load(response)
assert status['enabled'] is False and status.get('owner') is None, 'Disable tracking before the container handoff'
PY
old_pid=$(pgrep -x erv)
test "$(readlink -f /proc/$old_pid/exe)" = /home/pi/operator-esp-demo-20261103/build/bin/erv
sudo -n systemctl stop pivision-integration.service
sudo -n systemctl stop camera-streamer.service
kill -INT "$old_pid"
for attempt in 1 2 3 4 5 6 7 8; do
    if ! kill -0 "$old_pid" 2>/dev/null; then break; fi
    sleep 1
done
if pgrep -x erv >/dev/null; then
    echo 'Old Operator did not stop. Restore host services; not starting a second serial owner.' >&2
    sudo -n systemctl start camera-streamer pivision-integration
    exit 1
fi
if sudo -n fuser /dev/talos-bluey-serial /dev/talos-bluey-camera >/dev/null 2>&1; then
    sh rollback-bluey.sh
    exit 1
fi
if ! sh stack.sh --profile bluey up -d; then
    sh rollback-bluey.sh
    exit 1
fi
if ! /home/pi/tracker_venv/bin/python - <<'PY'
import json, time, urllib.request
for _ in range(45):
    try:
        with urllib.request.urlopen('http://127.0.0.1:5051/api/v1/control/status', timeout=2) as response:
            state = json.load(response)
        with urllib.request.urlopen('http://127.0.0.1:5051/api/v1/observations', timeout=2) as response:
            observations = json.load(response)
        if state.get('operator_connected') and state.get('controller_profile') == 'esp-one-axis' and observations.get('sample'):
            print(json.dumps({'control': state, 'sample': {k: observations['sample'].get(k) for k in ('fps', 'inference_s', 'age_s')}}))
            break
    except (OSError, ValueError):
        pass
    time.sleep(1)
else:
    raise SystemExit('Container verification failed; restoring host services')
PY
then
    sh rollback-bluey.sh
    exit 1
fi
echo 'Bluey containers verified. No HOME or jog was requested.'
