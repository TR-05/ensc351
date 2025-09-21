#!/bin/bash

if [ -f /boot/firmware/extlinux/extlinux.conf ] ; then
	if [ -f /sys/devices/soc0/family ] ; then
		make clean
		soc_family=$(cat /sys/devices/soc0/family || true)
		case "${soc_family}" in
		AM62X|J721E|J722S)
			make -j4 all_arm64
			echo "DeviceTrees: Built"
			if ! id | grep -q root; then
				echo "DeviceTrees: Install, Password required for sudo..."
				sudo make install_arm64
				echo "DeviceTrees: Installed"
			else
				make install_arm64
				echo "DeviceTrees: Installed"
			fi
			;;
		esac
	else
		make clean
		make -j4
		echo "DeviceTrees: Built"
	fi
else
	if [ -f /sys/devices/soc0/family ] ; then
		make clean
		soc_family=$(cat /sys/devices/soc0/family || true)
		case "${soc_family}" in
		AM33xx)
			make -j4 all_arm
			echo "DeviceTrees: Built"
			if ! id | grep -q root; then
				echo "DeviceTrees: Install, Password required for sudo..."
				sudo make install_arm
				echo "DeviceTrees: Installed"
			else
				make install_arm
				echo "DeviceTrees: Installed"
			fi
			;;
		esac
	else
		make clean
		make -j4
		echo "DeviceTrees: Built"
	fi
fi
