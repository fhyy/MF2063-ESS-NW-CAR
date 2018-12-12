# IP-Address

192.168.0.4

# install_vsomeip.sh

This scripts updates the repo mirrors, installs the required dependencies and then installs
vsomeip under "/home/pi/someip/vsomeip". Vsomeip can then be used in C++ programs by
including <vsomeip/vsomeip.hpp>. Note that the Raspberry Pi requires Internet access for
this script to work.

# object_detection

The folder called "object_detection" contains the object detection code, which can run on laptop and Raspberry Pi. To run color
detection on Raspberry Pi, execute python3 color_detection_cpp.py

# Software Architecture Overview
**Architecture Overview**  
![Architecture Overview](../figures/software_architecture_rpi_files.png)
