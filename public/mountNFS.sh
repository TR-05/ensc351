#!/bin/bash

sudo mount -t nfs 192.168.7.1:/home/trevor/ENSC351/public /mnt/remote
echo Mounted!
cd ../../mnt/remote
ls



