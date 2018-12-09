#!/bin/bash

#
# This script intends to deploy and install the software required
# to run the car onto the three BBB's and to the RPi that handles
# the camera. Note that this script does NOT install software
# on any arduinos or on the SDN controller. That has to
# be done manually.
#
# GENERATE DOCUMENTATION WITH THIS SCRIPT: ./deploy_code.sh --gen-doc
#
# This script generates simple scripts called run_<something>.sh which
# runs the respective vsomeip programs and the node-specific program
# simultaneously.
#
# After this script has finished, the user should be able to
# stand in any folder on the <something> node and type run_<something>.sh
# in order to start the program <something> on the corresponding
# node. Make sure that the user is "debian" for the BBB's
# and "pi" for the RPi.
#
# If the vsomeip configurations  (vsomeip.json) are not loaded properly,
# try calling run<something>.sh while standing in the build folder of
# that node (or in the folder above the build folder). For example:
# if run_ess_shell.sh does not seem to load the correct vsomeip.json
# configs, try standing in ~/bbb-car-controller/build or ~/bbb-car-controller
# and call the script from there by simply typing run_ess_shell.sh
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
scp cfg_bbb/* debian@192.168.0.2:/home/debian/bbb-distance-steering/linux_configurations

# Build the system and create a script that launches the required programs
ssh debian@192.168.0.2 << EOF
  rm -r /home/debian/bbb-distance-steering/build # Remove build folder if it got copied
  rm -r /home/debian/bbb-distance-steering/doc # Remove doc folder if it got copied

  mkdir /home/debian/bbb-distance-steering/build
  cd /home/debian/bbb-distance-steering/build
  cmake ..
  make

  ln -sf /home/debian/bbb-distance-steering/build/dist-steer-service /home/debian/bin
  ln -sf /home/debian/bbb-distance-steering/build/distance /home/debian/bin
  ln -sf /home/debian/bbb-distance-steering/build/vsomeip.json /home/debian/bin

  # Create a script that launches and kills the required programs simultaneously
  if [ ! -f /home/debian/bin/run_dist_steer.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/run_dist_steer.sh
    echo "set VSOMEIP_CONFIGURATION="/home/debian/bbb-distance-steering/vsomeip.json"" >> /home/debian/bin/run_dist_steer.sh
    echo "sudo dist-steer-service & sudo distance" >> /home/debian/bin/run_dist_steer.sh
    chmod +x /home/debian/bin/run_dist_steer.sh
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
scp cfg_bbb/* debian@192.168.0.3:/home/debian/bbb-motor-speed/linux_configurations

# Build the system and create a script that launches the required programs
ssh debian@192.168.0.3 << EOF
  rm -r /home/debian/bbb-motor-speed/build # Remove build folder if it got copied
  rm -r /home/debian/bbb-motor-speed/doc # Remove doc folder if it got copied


  mkdir /home/debian/bbb-motor-speed/build
  cd /home/debian/bbb-motor-speed/build
  cmake ..
  make

  ln -sf /home/debian/bbb-motor-speed/build/motor-speed-service /home/debian/bin
  ln -sf /home/debian/bbb-motor-speed/build/speed /home/debian/bin
  ln -sf /home/debian/bbb-motor-speed/build/vsomeip.json /home/debian/bin

  # Create a script that launches the required programs simultaneously
  if [ ! -f /home/debian/bin/run_motor_speed.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/run_motor_speed.sh
    echo "set VSOMEIP_CONFIGURATION="/home/debian/bbb-motor-speed/vsomeip.json"" >> /home/debian/bin/run_motor_speed.sh
    echo "sudo motor-speed-service & sudo speed" >> /home/debian/bin/run_motor_speed.sh
    chmod +x /home/debian/bin/run_motor_speed.sh
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
  rm -r /home/pi/rpi-vision/build # Remove build folder if it got copied
  rm -r /home/pi/rpi-vision/doc # Remove doc folder if it got copied

  mkdir /home/pi/rpi-vision/build
  cd /home/pi/rpi-vision/build
  cmake ..
  make

  ln -sf /home/pi/rpi-vision/build/camera-service /home/pi/bin
  ln -sf /home/pi/rpi-vision/src/color_detection_cpp.py /home/pi/bin
  ln -sf /home/pi/rpi-vision/build/vsomeip.json /home/pi/bin

  # Create a script that launches the required programs simultaneously
  if [ ! -f /home/pi/bin/run_camera.sh ]; then
    echo "#!/bin/bash" >> /home/pi/bin/run_camera.sh
    echo "set VSOMEIP_CONFIGURATION="/home/pi/rpi-vision/vsomeip.json"" >> /home/pi/bin/run_camera.sh
    echo "python3 /home/pi/bin/color_detection_cpp.py | camera-service" >> /home/pi/bin/run_camera.sh
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
scp cfg_bbb/* debian@192.168.0.10:/home/debian/bbb-car-controller/linux_configurations

# Build the system and create a script that launches the required programs
ssh debian@192.168.0.10 << "EOF"
  rm -r /home/debian/bbb-car-controller/build # Remove build folder if it got copied
  rm -r /home/debian/bbb-car-controller/doc # Remove doc folder if it got copied


  mkdir /home/debian/bbb-car-controller/build
  cd /home/debian/bbb-car-controller/build
  cmake ..
  make

  ln -sf /home/debian/bbb-car-controller/build/car-ctrl-client /home/debian/bin
  ln -sf /home/debian/bbb-car-controller/build/ess_shell /home/debian/bin
  ln -sf /home/debian/bbb-car-controller/build/main /home/debian/bin
  ln -sf /home/debian/bbb-car-controller/build/vsomeip.json /home/debian/bin

  # Create a script that launches the shell and vsomeip simultaneously
  if [ ! -f /home/debian/bin/run_ess_shell.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/run_ess_shell.sh
    echo "set VSOMEIP_CONFIGURATION="/home/debian/bbb-car-controller/vsomeip.json"" >> /home/debian/bin/run_ess_shell.sh
    echo "car-ctrl-client & ess_shell" >> /home/debian/bin/run_ess_shell.sh
    chmod +x /home/debian/bin/run_ess_shell.sh
  fi
  # Create a script that launches the auto-driver and vsomeip simultaneously
  if [ ! -f /home/debian/bin/run_ess_auto.sh ]; then
    echo "#!/bin/bash" >> /home/debian/bin/run_ess_auto.sh
    echo "set VSOMEIP_CONFIGURATION="/home/debian/bbb-car-controller/vsomeip.json"" >> /home/debian/bin/run_ess_auto.sh
    echo "car-ctrl-client & main" >> /home/debian/bin/run_ess_auto.sh
    chmod +x /home/debian/bin/run_ess_auto.sh
  fi
EOF

# Generate documentation for the deployed code.
# Note that the documentation will be generated on the
# same device that is running this script, not on any
# of the car nodes.
if [ "$1" = "--gen-doc" ]; then
  doxygen Doxyfile
fi
