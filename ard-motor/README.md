#Pin mapping (Arduino Micro)
ESC __S__ signal - Digital Pin 10  
ESC __-__ signal - GND 
_Don't connect ESC __+___
SPI - TBD

#Installation
##Command line
??
##IDE
__Arduino IDE__  
Load the motor_control.ino  
Connect the arduino to any USB-port  
Upload the code to the Arduino

#Description
__Goal__  
This arduino will read input values for motor PWM duty cycle and modify the registers to send out this PWM to the Motor PWM signal pin.

__As for now__  
This arduino sets up timer 1 for Fast PWM mode with variable duty cycle.  
The duty cycle is constantly changing from 0% to 100% and back to 0% in 20 seconds.  

_**Note:** this code does not work with the motor itself at the moment..._