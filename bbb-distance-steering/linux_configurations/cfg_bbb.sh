#! /bin/sh

sudo cp hostname /etc/hostname
sudo cp hosts /etc/hosts
sudo cp interfaces /etc/network/interfaces
sudo chattr +i /etc/network/interfaces #prevent other programs from modifying
sudo cp resolv.conf /etc/resolv.conf
sudo chattr +i /etc/resolv.conf #prevent other programs from modifying

