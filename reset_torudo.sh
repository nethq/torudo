#!/bin/bash

# Check if script is run as root
if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root" 1>&2
  exit 1
fi

echo "Restoring original resolv.conf..."
if [ -f /etc/resolv.conf.bak ]; then
  mv /etc/resolv.conf.bak /etc/resolv.conf
  echo "resolv.conf restored."
else
  echo "No backup of resolv.conf found."
fi

echo "Flushing iptables rules..."
iptables -F
iptables -t nat -F
iptables -t mangle -F
iptables -X
echo "iptables rules flushed."

echo "Stopping any running Tor instances..."
pkill -f 'tor -f'
systemctl stop tor
echo "Tor stopped."

echo "Reset completed."
