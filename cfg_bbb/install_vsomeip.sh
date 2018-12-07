#! /bin/sh
sudo apt-get update
sudo apt-get install cmake
sudo apt-get install libboost-system1.62-dev libboost-thread1.62-dev libboost-log1.62-dev
mkdir /home/debian/someip
cd /home/debian/someip
git clone https://github.com/GENIVI/vsomeip.git
sudo fallocate -l 700M /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
cd /home/debian/someip/vsomeip
mkdir /home/debian/someip/vsomeip/build
cd /home/debian/someip/vsomeip/build
cmake -DENABLE_SIGNAL_HANDLING=1 ..
make
sudo make install
sudo swapoff /swapfile
sudo rm /swapfile

