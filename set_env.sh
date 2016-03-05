#!/bin/sh
echo "Initialising environment..."
sudo iptables -F
sudo pkill dnsmasq
sudo systemctl stop NetworkManager
sudo ifconfig wlp1s0 down
sudo ifconfig wlp0s29u1u2 down
sudo ifconfig wlp0s29u1u2 192.168.1.1 netmask 255.255.255.0 up
sudo firewall-cmd --add-masquerade
echo "Done"
