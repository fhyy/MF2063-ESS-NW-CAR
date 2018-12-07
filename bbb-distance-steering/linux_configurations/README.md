This folder contains configuration files for the Beagle Bone Black with hostname bbb-distance-steering
Furthermore, make sure the sudoers file in the target beaglebone looks like the example sudoers file in this folder!

SCRIPT                  DESCRIPTION
spi_setup.sh            This script initialises all the necessary pins for the beaglebone to communicate using its SPI interface. The script is cofigured for debian9 and only needs to be executed once for each new debian installation, i.e. the settings made by it survives reboots.


FILE			PLACE IN LINUX FILESYSTEM
hostname		/etc/hostname
hosts			/etc/hosts
interfaces		/etc/network/interfaces
resolv.conf		/etc/resolv.conf (run "sudo chattr +i /etc/resolv.conf" to prevent other programs from editing this file)
