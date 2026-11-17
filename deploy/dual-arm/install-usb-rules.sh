#!/bin/sh
set -eu
cd "$(dirname "$0")"
sudo install -m 0644 70-talos-arms.rules /etc/udev/rules.d/70-talos-arms.rules
sudo udevadm control --reload-rules
sudo udevadm trigger --action=add --subsystem-match=tty
sudo udevadm trigger --action=add --subsystem-match=video4linux
sudo udevadm settle
ls -l /dev/talos-*
