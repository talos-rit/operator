#!/bin/bash

# Verify being run as sudo
if [[ $EUID -ne 0 ]]
then
    printf "Please run as sudo.\n" >&2
    exit 1
fi

set -x
apt update
apt -y upgrade

# Networking prereqs
apt install -y hostapd dnsmasqsudo netfilter-persistent iptables-persistent
systemctl unmask hostapd
systemctl enable hostapd
# shift to other script
set +x
reboot


# Operator prereqs
apt install -y git cpputest