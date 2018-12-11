#! /bin/bash

sudo cp debian /etc/sudoers.d/debian
sudo chmod 0440 /etc/sudoers.d/debian

sudo cp hostname /etc/hostname
sudo cp hosts /etc/hosts

sudo chattr -i /etc/network/interfaces #make file modifiable
sudo cp interfaces /etc/network/interfaces
sudo chattr +i /etc/network/interfaces #prevent other programs from modifying

sudo chattr -i /etc/resolv.conf #make file modifiable
sudo cp resolv.conf /etc/resolv.conf
sudo chattr +i /etc/resolv.conf #prevent other programs from modifying
