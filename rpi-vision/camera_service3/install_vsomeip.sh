#! /bin/sh
sudo apt-get update
sudo apt-get install libboost-system1.62-dev libboost-thread1.62-dev libboost-log1.62-dev cmake
mkdir /home/pi/someip
cd /home/pi/someip
git clone https://github.com/GENIVI/vsomeip.git
cd /home/pi/someip/vsomeip
mkdir /home/pi/someip/vsomeip/build
cd /home/pi/someip/vsomeip/build
cmake -DENABLE_SIGNAL_HANDLING=1 ..
make
