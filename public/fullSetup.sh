#!/bin/bash

# Check if script is run with sudo
if [ "$(id -u)" != "0" ]; then
    echo "This script must be run with sudo."
    exit 1
fi

echo "alias ll='ls -la'" >> ~/.bashrc
echo "alias public='cd /mnt/remote/'" >> ~/.bashrc

# Step 1: Enable Networking
cat << 'EOF' > internetToTarget.sh

#!/bin/sh
sudo route add default gw 192.168.7.1
echo nameserver 8.8.8.8 | sudo tee -a /etc/resolv.conf

EOF

chmod +x internetToTarget.sh

echo "./internetToTarget.sh" >> ~/.bashrc
source ~/.bashrc

# Step 2: Updates
sudo apt update
sudo apt upgrade
sudo apt install nfs-common
sudo apt-get install libasan6
sudo apt-get install build-essential

# Step 3: Enable NFS
sudo mkdir -p /mnt/remote
sudo chown trevor /mnt/remote

cat << 'EOF' > mountNFS.sh

#!/bash/sh
sudo mount -t nfs 192.168.7.1:home/trevor/351/public /mnt/remote

EOF

chmod +x mountNFS.sh

echo "./mountNFS.sh" >> ~/.bashrc
source ~/.bashrc
