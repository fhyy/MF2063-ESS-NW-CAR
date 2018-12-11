#!/usr/bin/env bash

#Shell script to run automatically att startup to bootstrap all processes that needs to run such that the car can go into normal execution without user intervention.

# To get this script running automatically on boot on a new beaglebone, execute the command:
# crontab -e
# This will open a file in your favourite text-editor, copy the following line (excluding the #) into that file, save it and close it

# @reboot ~/bbb-motor-speed/autostart.sh

# Remove files that may exist from last boot and will ruin the execution
rm -rf /dev/shm/*

# Navigate into the built executables
cd ~/bbb-motor-speed/build

# Execute the necessary processes
./motor-speed-service & ./speed

