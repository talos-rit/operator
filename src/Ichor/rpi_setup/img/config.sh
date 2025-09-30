#!/bin/bash

IMAGE_PATH="/media/brooke/Vault/Dev/RIT/Fall2024/SWEN561/rpi-rt-kernel/build/raspios_lite_arm64"
MOUNT_PATH="/mnt/raspios_rt"

CMD_LINE_SERIAL="console=serial0,115200 console=tty1 root=PARTUUID=8dad96ec-02 rootfstype=ext4 fsck.repair=yes rootwait cfg80211.ieee80211_regdom=US
net.ifnames=0 cfg80211.ieee80211_regdom=US"

CONFIG_FILE="# Known issues
# Disable USB FIQ support for Pi0, Pi2, Pi3
# https://wiki.linuxfoundation.org/realtime/documentation/known_limitations

[pi0]
dwc_otg.fiq_enable=0
dwc_otg.fiq_fsm_enable=0
auto_initramfs=1

[pi2]
dwc_otg.fiq_enable=0
dwc_otg.fiq_fsm_enable=0

[pi3]
dwc_otg.fiq_enable=0
dwc_otg.fiq_fsm_enable=0

[all]
arm_64bit=1
kernel=kernel8_rt.img
enable_uart=1
dtparam=i2c_arm=on
dtparam=spi=on"