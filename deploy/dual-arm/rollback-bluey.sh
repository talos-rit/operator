#!/bin/sh
set -eu
cd "$(dirname "$0")"
sh stack.sh stop bluey-vision bluey-camera bluey-operator || true
sudo -n systemctl start camera-streamer.service
if ! sudo -n fuser /dev/talos-bluey-serial >/dev/null 2>&1; then
    ERV_CONTROLLER_PROFILE=esp-one-axis nohup /home/pi/operator-esp-demo-20261103/build/bin/erv \
        "$PWD/rollback-bluey.conf" > "$PWD/logs/bluey/host-rollback-console.log" 2>&1 < /dev/null &
fi
sudo -n systemctl start pivision-integration.service
echo 'Host Bluey services restored; original deployments were preserved.'
