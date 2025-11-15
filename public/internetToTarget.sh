#!/bin/bash
sudo route add default gw 192.168.7.1
sudo timedatectl set-ntp true
echo Routed!
