# Pin mapping (Arduino Micro)
_ESC RX S_ - Digital Pin 10  
_ESC RX __-___ - GND  
_SPI CLK_ - SCK  
_SPI Master out_ - MOSI  
_SPI Slave select_ - SS  

# Installation
## Command line
??
## IDE
__Arduino IDE__  
Load the motor_control.ino  
Connect the arduino to any USB-port  
Select the chip type and used USB-port from the menu  
Upload the code to the Arduino

# Description
## Goal  
This arduino will read input values for target speed and the latest encoder speed data through the SPI interface and store these variables for PID-controlling to reach and maintain the target speed.

_Note: The car can only go forwards_


## SPI interface
__Stream__  

|Byte 1|  
|:---:|  
|TargetSpeed or CurrentSpeed|  

__Variable definition__  

|Name|Range|Description|  
|:---:|:---:|:---|  
|TargetSpeed/CurrentSped|[Bit 0 --> 7] (0,125) |(Bit 8 = 0):The target speed in cm/s / (Bit 8 = 1): The latest encoder readings in cm/s|  
