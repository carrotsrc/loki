#!/bin/sh

echo "Bringing servers down..."
sudo pkill dhcpd
sudo ifconfig wlp0s29u1u2 down
sudo ifconfig wlp1s0 up
sudo systemctl stop lighttpd
sudo systemctl start NetworkManager
echo "Normal functionality restored"
