#!/bin/bash

# Verify being run as sudo
if [[ $EUID -ne 0 ]]
then
    printf "Please run as sudo.\n" >&2
    exit 1
fi

# Get location of this script
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Source config
source $SCRIPT_DIR/config.sh

set -x

# Mount image file
mkdir -p "$MOUNT_PATH"/boot
mkdir -p "$MOUNT_PATH"/root

DEVICE=$(losetup -f)
echo "$DEVICE"
losetup "$DEVICE" -P "$IMAGE_PATH"
mount "$DEVICE"p1 "$MOUNT_PATH"/boot
mount "$DEVICE"p2 "$MOUNT_PATH"/root

# Setup TTL Serial console
#echo "$CMD_LINE_SERIAL" > "$MOUNT_PATH"/boot/cmdline.txt"
#echo "$CONFIG_FILE" > "$MOUNT_PATH"/boot/config.txt"

# Copy setup script and readme
#cp "$SCRIPT_DIR"/../boot/setup.sh "$MOUNT_PATH"/root/home/pi/
#cp "$SCRIPT_DIR"/README.md $MOUNT_PATH"/root/home/pi/

# Setup Operator directory
#mkdir -p /etc/talos/operator/config
#mkdir -p /etc/talos/operator/logs
#chown -R 1000:1000 /etc/talos

#mkdir -p "$MOUNT_PATH"/root/home/pi/talos
#cd "$MOUNT_PATH"/root/home/pi/talos
#git clone https://github.com/talos-rit/operator.git
#cd -

read -n1 -s -r -p $'Press any key to continue...\n' key

# Clean up
umount "$MOUNT_PATH"/boot
umount "$MOUNT_PATH"/root
losetup -d "$DEVICE"

set +x
exit
