# Pin mapping (Arduino Micro)
_Servo PWM signal_ - Digital Pin 9  
_Servo GND_ - GND  (Shared ground)  
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
This arduino reads the wanted steering percentage (0% -> 100%) through the SPI interface and sends the correct data to the servo motor.  
0% is full left, 50% is straight ahead and 100% is full right.

The servo motor turns the wheels between -20 and +20 degrees.

## SPI interface
__Stream__  

|Byte 1|  
|:---:|  
|TurnPercentage|  

__Variable definition__  

|Name|Range|Description|  
|:---:|:---:|:---|  
|TurnPercentage|(0,100)|0 := full left turn, 50 := straight ahead, 100 := full right turn|  
