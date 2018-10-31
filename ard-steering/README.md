#Pin mapping (Arduino Micro)
Servo PWM signal - Digital Pin 9  
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
This arduino reads the wanted steering percentage (0% -> 100%) and sends the correct data to the servo motor.  
0% is full left, 50% is straight ahead and 100% is full right.

The servo motor turns the wheels between -20 and +20 degrees.  

__As for now__  
The arduino turns the motor loops from turning full right to forwards to full left to forwards, in 4 seconds intervals