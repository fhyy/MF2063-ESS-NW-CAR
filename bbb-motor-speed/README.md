# Startup

To start the processes it is sufficient to build the executables using cmake and executing /bbb-motor-speed/autostart.sh

It might however be interesting to have the programs run automatically at startup without user intervention. This can be done using crontab. Execute 
```
crontab -e
```
when logged in as debian user and paste 
```
@reboot /path/to/bbb-motor-speed/autostart.sh
```
into the file replacing "/path/to/" accordingly.


# IP-Address

192.168.0.3


# Linux configurations

To run the entire system as smoothly as possible, it is recommended to use the following configuration files for this node.

| File          | Place in Linux filesystem
|---------------|---------------------------------------------------------------------------------------------------------
| hostname      | /etc/hostname
| hosts         | /etc/hosts
| interfaces    | /etc/network/interfaces
| resolv.conf   | /etc/resolv.conf (run "sudo chattr +i /etc/resolv.conf" to prevent other programs from editing this file)
