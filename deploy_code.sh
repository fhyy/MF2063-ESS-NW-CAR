ssh debian@192.168.0.2 "rm -r /home/debian/bbb-distance-steering"
scp -r bbb-distance-steering debian@192.168.0.2:/home/debian/
ssh debian@192.168.0.2 "rm -r /home/debian/bbb-distance-steering/build/*"
ssh debian@192.168.0.2 "rm -r /home/debian/bbb-distance-steering/doc"
ssh debian@192.168.0.3 "rm -r /home/debian/bbb-motor-speed"
scp -r bbb-motor-speed debian@192.168.0.3:/home/debian/
ssh debian@192.168.0.3 "rm -r /home/debian/bbb-motor-speed/build/*"
ssh debian@192.168.0.3 "rm -r /home/debian/bbb-motor-speed/doc"
ssh debian@192.168.0.10 "rm -r  /home/debian/bbb-car-controller"
scp -r bbb-car-controller debian@192.168.0.10:/home/debian
ssh debian@192.168.0.10 "rm -r /home/debian/bbb-car-controller/build/*"
ssh debian@192.168.0.10 "rm -r /home/debian/bbb-car-controller/doc"
ssh pi@192.168.0.4 "rm -r /home/pi/rpi-vision"
scp -r rpi-vision pi@192.168.0.4:/home/pi/
ssh pi@192.168.0.4 "rm-r/home/pi/rpi-vision/build"
ssh pi@192.168.0.4 "rm-r/home/pi/rpi-vision/doc"
