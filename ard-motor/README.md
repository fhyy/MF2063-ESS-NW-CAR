# Pin mapping (Arduino Micro)
_ESC RX S_ - Digital Pin 10  
_ESC RX __-___ - GND  
_SPI CLK_ - SCK  
_SPI Master out_ - MOSI  
_SPI Slave select_ - SS  

# Installation

## ESC setup 
_Do this if the ESC does not make this sound when switching on: **beep-beep-boooop**_  

1. Connect GND pin and PWM pin to Arduino GND and pin 10, **NOT Vcc !**  
2. Run the esc_programmer code on connected Arduino  
3. Send signal **2000** (in Serial monitor)  
4. Hold "set" button on ESC and switch it on  
5. Wait for **orange** **solid** light  
6. Release "set" button and wait for **solid** **red** light
7. Send signal **700** (in Serial monitor)  
8. Wait for **solid** **orange** light  
9. Send signal 1000  
10. Wait for **beep** and **no light**  
11. **Switch off** the ESC and you are **done**!  

Install everything as usual and make sure to switch on the ESC before the motor Arduino sends any pwm signals.

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
|TargetSpeed /</br> CurrentSpeed|  

__Variable definition__  

|Name|Range|Description|  
|:---:|:---:|:---|  
|TargetSpeed </br>CurrentSpeed|[Bit 0 --> 7]</br> (0,125) |(Bit 8 = 0):The target speed in 2cm/s (receiving x means x*2 cm/s) </br> (Bit 8 = 1): The latest encoder readings in 2cm/s|  
