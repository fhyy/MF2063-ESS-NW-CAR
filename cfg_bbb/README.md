# Overview

The following files wile get copied onto the BeagleBones upon running the script "deploy_code.sh",
which should reside in the top level of this repo. They can be used to configure the BeagleBones
into a state in which the system can run smoothly.

# cfg_bbb.sh

This script can be run on any of the BeagleBones after "deploy_code.sh" has been run.
It does some basic configurations by copying configuration files to their respective
location in the Linux filesystem. Note that a rebbot is required before the changes
take effect.

# debian

This file should reside in "/etc/sudoers.d/debian" and it allows "sudo" to see the user
"debian's" local bin directory as well as allow the user "debian" to run "sudo" without
entering a password.

# install_vsomeip.sh

This scripts updates the repo mirrors, installs the required dependencies and then installs
vsomeip under "/home/debian/someip/vsomeip". Vsomeip can then be used in C++ programs by
including <vsomeip/vsomeip.hpp>. Note that the BeagleBone requires Internet access for
this script to work.

# resolv.conf

Depending on how BeagleBone connects to the Internet, it might sometimes require knowledge
of where it can resolve Internet addresses (URLs). This file configures the BeagleBone
to use Google's public DNS server, which should be enough for most cases. The file should
reside in /etc/resolv.conf.

# spi_setup.sh

To use SPI on the BeagleBones the hardware needs to be configured correctly. This script
is based on the instructions given [here]
(http://www.nagavenkat.adurthi.com/arduini-beaglebone/479/). The script compiles a
firmware file and places it under /lib/firmware.It also registers gpio60 and gpio48 to be
used as slave select pins. The script works for debian9 and the system should be rebooted
after running

