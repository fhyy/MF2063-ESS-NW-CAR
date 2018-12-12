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

# Pin mapping
![Pin mapping](https://raw.githubusercontent.com/fhyy/MF2063-ESS-NW-CAR/master/resources/diagrams/motor-speed-node.svg?sanitize=true)

# IP-Address

192.168.0.3


# Linux configurations

To run the entire system as smoothly as possible, it is recommended to use the following configuration files for this node.
Note that the bottom two files reside in "/cfg_bbb" (where / is the toplevel of this repo). In "/cfg_bbb", some
scripts can be found that help with the configurations. By running the script /deploy_code.sh, all files needed
for a smooth, scripted setup should be copied neatly onto the respective BegleBone nodes.

| File          | Place in Linux filesystem
|---------------|---------------------------------------------------------------------------------------------------------
| hostname      | /etc/hostname
| hosts         | /etc/hosts
| interfaces    | /etc/network/interfaces
| resolv.conf   | /etc/resolv.conf (run "sudo chattr +i /etc/resolv.conf" to prevent other programs from editing this file)
| debian        | /etc/sudoers.d/debian

# Building the motor/speed service

To build this program on the target node stand in the same folder as this README and type:
```bash
mkdir build
cd build
cmake ..
make
```

This will build all executables needed to publish sensor data and take actuator requests through vsomeip.

# Starting the motor/speed service
To start the program, stand in the build folder and type:
```bash
sudo ./motor-speed-service & sudo ./motor_speed
```

This will run the vsomeip service and the SPI master program, respectively.
Additionally the following flags can be passed to "motor-speed-service":
| Flag           | Argument        |Description
|----------------|-----------------|-----------------------------------------------------------------------------------------------
| --skip-go      | [EMPTY]         | Speedometer events will always be published, even if client has not given any GO-signal
| --sleep        | A positive int  | Time spent sleeping (in millisec) between publishing speedometer events

**Example**
To run the software with 100 milliseconds of sleep between speed events, and
make it ignore the availability of the go-signal, stand in the build folder and type:
```bash
sudo ./motor-speed-service --skip-go --sleep 100 & sudo ./motor_speed
```

# Software Architecture Overview
**Architecture Overview**  
![Architecture Overview](../figures/software_architecture_bbb_files.png)
