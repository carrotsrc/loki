#!/bin/sh
echo "Starting DHCP daemon..."
sudo dhcpd

echo "Starting HTTP daemon..."
sudo systemctl start lighttpd.service

echo "Starting Access Point..."
sudo hostapd hapd_loki.conf

