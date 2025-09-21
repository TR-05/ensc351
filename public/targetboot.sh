#!/bin/sh
sudo route add default gw 192.168.7.1
echo Target Routed!
sudo mount -t nfs 192.168.7.1:/home/trevor/ENSC351/public /mnt/remote
echo Target Mounted!
