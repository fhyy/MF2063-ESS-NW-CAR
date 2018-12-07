#! /bin/sh

sudo usermod -aG sudo debian
sudo cp hostname /etc/hostname
sudo cp hosts /etc/hosts
sudo cp interfaces /etc/network/interfaces
sudo chattr +i /etc/network/interfaces #prevent other programs from modifying
sudo cp resolv.conf /etc/resolv.conf
sudo chattr +i /etc/resolv.conf #prevent other programs from modifying
sudo echo -e "mf2063\nmf2063" | passwd debian
