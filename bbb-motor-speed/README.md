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
