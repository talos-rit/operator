#!/bin/bash

# Verify being run as sudo
if [[ $EUID -ne 0 ]]
then
    printf "Please run as sudo.\n" >&2
    exit 1
fi

set -x
# apt update
# apt -y upgrade

HOSTAPD_PACKAGES="hostapd dnsmasqsudo dhcpcd netfilter-persistent iptables-persistent"
OPERATOR_PACKAGES="git cpputest"

# Networking prereqs
apt install -y "$HOSTAPD_PACKAGES" "$OPERATOR_PACKAGES"
systemctl unmask hostapd
systemctl enable hostapd
touch /tmp/reboot_flag
# Setup configurations
echo "interface wlan0
    static ip_address=192.168.4.1/24
    nohook wpa_supplicant" > /etc/dhcpcd.conf

echo "# Enable IPv4 routing
net.ipv4.ip_forward=1" > /etc/sysctl.d/routed-ap.conf

echo "interface=wlan0 # Listening interface
dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h
                # Pool of IP addresses served via DHCP
domain=wlan     # Local wireless DNS domain
address=/gw.wlan/192.168.4.1
                # Alias for this router" > /etc/dnsmasq.conf

echo "country_code=US
interface=wlan0
hw_mode=a
channel=48
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=WirelessNetworkPassword
wpa_pairwise=TKIP
rsn_pairwise=CCMP

ssid=Talos
wpa_key_mgmt=Operator
" > /etc/hostapd/hostapd.conf

raspi-config nonint do_wifi_country US
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
netfilter-persistent save
rm /tmp/reboot_flag
reboot
