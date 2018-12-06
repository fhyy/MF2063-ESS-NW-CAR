#!/bin/sh

# Deploy code onto BBB that handles distance and steering
ssh debian@192.168.0.2 "rm -r /home/debian/bbb-distance-steering"
scp -r bbb-distance-steering debian@192.168.0.2:/home/debian/
scp -r lib debian@192.168.0.2:/home/debian/bbb-distance-steering
ssh debian@192.168.0.2 "rm -r /home/debian/bbb-distance-steering/build"
ssh debian@192.168.0.2 "mkdir /home/debian/bbb-distance-steering/build"
ssh debian@192.168.0.2 "rm -r /home/debian/bbb-distance-steering/doc"
ssh debian@192.168.0.2 "cd /home/debian/bbb-distance-steering/build && cmake .. && make"

# Deploy code onto BBB that handles motor and speed
ssh debian@192.168.0.3 "rm -r /home/debian/bbb-motor-speed"
scp -r bbb-motor-speed debian@192.168.0.3:/home/debian/
scp -r lib debian@192.168.0.3:/home/debian/bbb-motor-speed
ssh debian@192.168.0.3 "rm -r /home/debian/bbb-motor-speed/build"
ssh debian@192.168.0.3 "mkdir /home/debian/bbb-motor-speed/build"
ssh debian@192.168.0.3 "rm -r /home/debian/bbb-motor-speed/doc"
ssh debian@192.168.0.3 "cd /home/debian/bbb-motor-speed/build && cmake .. && make"

# Deploy code onto the Raspberry Pi that handles the camera
ssh pi@192.168.0.4 "rm -r /home/pi/rpi-vision"
scp -r rpi-vision pi@192.168.0.4:/home/pi/
scp -r lib pi@192.168.0.4:/home/pi/rpi-vision
ssh pi@192.168.0.4 "rm -r /home/pi/rpi-vision/build"
ssh pi@192.168.0.4 "mkdir /home/pi/rpi-vision/build"
ssh pi@192.168.0.4 "rm -r /home/pi/rpi-vision/doc"
ssh pi@192.168.0.4 "cd /home/pi/rpi-vision/build && cmake .. && make"

# Deploy code onto the controller BBB
ssh debian@192.168.0.10 "rm -r  /home/debian/bbb-car-controller"
scp -r bbb-car-controller debian@192.168.0.10:/home/debian
ssh debian@192.168.0.10 "rm -r /home/debian/bbb-car-controller/build"
ssh debian@192.168.0.10 "mkdir /home/debian/bbb-car-controller/build"
ssh debian@192.168.0.10 "rm -r /home/debian/bbb-car-controller/doc"
ssh debian@192.168.0.10 "cd /home/debian/bbb-car-controller/build && cmake .. && make"

# Generate documentation for the deployed code
doxygen Doxyfile
