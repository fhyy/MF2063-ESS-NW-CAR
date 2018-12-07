This folder contains configuration files for the Beagle Bone Black with hostname bbb-car-controller

FILE			PLACE IN LINUX FILESYSTEM
hostname		/etc/hostname
hosts			/etc/hosts
interfaces		/etc/network/interfaces
resolv.conf		/etc/resolv.conf (run "sudo chattr +i /etc/resolv.conf" to prevent other programs from editing this file)
debian          /etc/sudoers.d/debian
