#!/bin/bash

#
# This script intends to deploy and install the software required
# to run the car onto the three BBB's and to the RPi that handles
# the camera. Note that this script does NOT install software
# on any arduinos or on the sdn controller. That has to
# be done manually.
#
# GENERATE DOCUMENTATION WITH THIS SCRIPT: ./deploy_code.sh --gen-doc
#
# This script generates two simple scripts on the sensor/actuator BBBs.
# One script for simultaneously starting vsomeip and the corresponding
# SPI program and one script for killing vsomeip if it is running in the
# background. For the controller beaglebone it generates three scripts,
# the kill-script plus one script for running the ess-shell with vsomeip
# and one script for running the auto-driver with vsomeip. For the
# camera RPi it generates one script, one that runs the Python program
# and pipes the output into vsomeip.
#
# After this script has finished, the user should be able to
# stand in any folder and type run_<something>.sh in order
# to start the program <something> on the corresponding
# node. Make sure that the user is "debian" for the BBB's
# and "pi" for the RPi.
#
# In order to run this script smoothly it is highly recommended
# to set up a passwordless login via ssh to all the nodes.
# This guide should help with that:
# https://www.thegeekstuff.com/2008/11/3-steps-to-perform-ssh-login-without-password-using-ssh-keygen-ssh-copy-id
#
# Hope this script helpts!
#
# //
# Leon


###########################################################
#                                                         #
# Deploy code onto BBB that handles distance and steering #
#                                                         #
###########################################################

# Remove the old directory and copy the new source code onto the node
ssh debian@192.168.0.2 "rm -r /home/debian/bbb-distance-steering"
scp -r bbb-distance-steering debian@192.168.0.2:/home/debian/
scp -r lib debian@192.168.0.2:/home/debian/bbb-distance-steering

# Build the system and create a script that launches the required programs
ssh debian@192.168.0.2 << EOF
  rm -r /home/debian/bbb-distance-steering/build # Remove build folder if one existed on the local machine
  mkdir /home/debian/bbb-distance-steering/build # and got copied onto the BBB
  rm -r /home/debian/bbb-distance-steering/doc # Remove doc folder if it got copied

  cd /home/debian/bbb-distance-steering/build
  cmake -DCMAKE_INSTALL_PREFIX=/home/debian ..
  cd /home/debian/bbb-distance-steering/build
  make
  make install

  # Create a script that launches the required programs simultaneously
  if [ ! -f /home/debian/bin/run_dist_steer.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/run_dist_steer.sh
    echo "dist-steer-service & sudo distance" >> /home/debian/bin/run_dist_steer.sh
    chmod +x /home/debian/bin/run_dist_steer.sh
  fi

  # Create a script that kills vsomeip if it is running in the background
  if [ ! -f /home/debian/bin/kill_vsomeip.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/kill_vsomeip.sh
    echo "kill $(ps aux | grep 'dist-steer-service' | awk '{print $2}')" >> /home/debian/bin/kill_vsomeip.sh
    chmod +x /home/debian/bin/kill_vsomeip.sh
  fi
EOF

###########################################################
#                                                         #
# Deploy code onto BBB that handles motor and speedometer #
#                                                         #
###########################################################

# Remove the old directory and copy the new source code onto the node
ssh debian@192.168.0.3 "rm -r /home/debian/bbb-motor-speed"
scp -r bbb-motor-speed debian@192.168.0.3:/home/debian/
scp -r lib debian@192.168.0.3:/home/debian/bbb-motor-speed

# Build the system and create a script that launches the required programs
ssh debian@192.168.0.3 << EOF
  rm -r /home/debian/bbb-motor-speed/build # Remove build folder if one existed on the local machine
  mkdir /home/debian/bbb-motor-speed/build # and got copied onto the BBB
  rm -r /home/debian/bbb-motor-speed/doc # Remove doc folder if it got copied

  cd /home/debian/bbb-motor-speed/build
  cmake -DCMAKE_INSTALL_PREFIX=/home/debian ..
  cd /home/debian/bbb-motor-speed/build
  make
  make install

  # Create a script that launches the required programs simultaneously
  if [ ! -f /home/debian/bin/run_motor_speed.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/run_motor_speed.sh
    echo "motor-speed-service & sudo speed" >> /home/debian/bin/run_motor_speed.sh
    chmod +x /home/debian/bin/run_motor_speed.sh
  fi

  # Create a script that kills vsomeip if it is running in the background
  if [ ! -f /home/debian/bin/kill_vsomeip.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/kill_vsomeip.sh
    echo "kill $(ps aux | grep 'motor-speed-service' | awk '{print $2}')" >> /home/debian/bin/kill_vsomeip.sh
    chmod +x /home/debian/bin/kill_vsomeip.sh
  fi
EOF

###########################################################
#                                                         #
# Deploy code onto the RPi that handles the camera        #
#                                                         #
###########################################################

# Remove the old directory and copy the new source code onto the node
ssh pi@192.168.0.4 "rm -r /home/pi/rpi-vision"
scp -r rpi-vision pi@192.168.0.4:/home/pi/
scp -r lib pi@192.168.0.4:/home/pi/rpi-vision

# Build the system and create a script that launches the required programs
ssh pi@192.168.0.4 << EOF
  rm -r /home/pi/rpi-vision/build # Remove build folder if one existed on the local machine
  mkdir /home/pi/rpi-vision/build # and got copied onto the BBB
  rm -r /home/pi/rpi-vision/doc # Remove doc folder if it got copied

  cd /home/pi/rpi-vision/build
  cmake -DCMAKE_INSTALL_PREFIX=/home/pi ..
  cd /home/pi/rpi-vision/build
  make
  make install

  # Create a script that launches the required programs simultaneously
  if [ ! -f /home/pi/bin/run_camera.sh ]; then
    echo "#!/bin/bash" >> /home/pi/bin/run_camera.sh
    echo "python3 /home/pi/rpi-vision/src/color_detection_cpp.py | camera_service" >> /home/pi/bin/run_camera.sh
    chmod +x /home/pi/bin/run_camera.sh
  fi
EOF

###########################################################
#                                                         #
# Deploy code onto BBB that is the main car controller    #
#                                                         #
###########################################################

# Remove the old directory and copy the new source code onto the node
ssh debian@192.168.0.10 "rm -r  /home/debian/bbb-car-controller"
scp -r bbb-car-controller debian@192.168.0.10:/home/debian
scp -r lib debian@192.168.0.10:/home/debian/bbb-car-controller

# Build the system and create a script that launches the required programs
ssh debian@192.168.0.10 << EOF
  rm -r /home/debian/bbb-car-controller/build # Remove build folder if one existed on the local machine
  mkdir /home/debian/bbb-car-controller/build # and got copied onto the BBB
  rm -r /home/debian/bbb-car-controller/doc # Remove doc folder if it got copied

  cd /home/debian/bbb-car-controller/build
  cmake -DCMAKE_INSTALL_PREFIX=/home/debian ..
  cd /home/debian/bbb-car-controller/build
  make
  make install

  # Create a script that launches the shell and vsomeip simultaneously
  if [ ! -f /home/debian/bin/run_ess_shell.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/run_ess_shell.sh
    echo "car-ctrl-client & ess_shell" >> /home/debian/bin/run_ess_shell.sh
    chmod +x /home/pi/bin/run_ess_shell.sh
  fi
  # Create a script that launches the auto-driver and vsomeip simultaneously
  if [ ! -f /home/debian/bin/run_ess_auto.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/run_ess_auto.sh
    echo "car-ctrl-client & main" >> /home/debian/bin/run_ess_auto.sh
    chmod +x /home/debian/bin/run_ess_auto.sh
  fi

  # Create a script that kills vsomeip if it is running in the background
  if [ ! -f /home/debian/bin/kill_vsomeip.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/kill_vsomeip.sh
    echo "kill $(ps aux | grep 'car-ctrl-client' | awk '{print $2}')" >> /home/debian/bin/kill_vsomeip.sh
    chmod +x /home/debian/bin/kill_vsomeip.sh
  fi
EOF

# Generate documentation for the deployed code.
# Note that the documentation will be generated on the
# same device that is running this script, not on any
# of the car nodes.
if [ "$1" = "--gen-doc" ]; then
  doxygen Doxyfile
fi
