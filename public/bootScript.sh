#!/bin/bash
sleep 5
route add default gw 192.168.7.1
echo Routed!
mount -t nfs 192.168.7.1:/home/trevor/ENSC351/public /mnt/remote
echo Mounted!

./home/trevor/a1



