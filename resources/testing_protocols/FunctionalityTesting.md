## Steering node
| Action | Expected outcome |
|:---| :---: |
|Connect and start affected nodes properly| --- |
|Send correctly formated data via SPI interface to node| Data transmitted (Can be verified by adding logging and reading logs via the serial port of the steering node) |
|Send angle value 0|Front wheels turns 20 degrees to the left|
|Send angle value 50|Front wheels centeres|
|Send angle value 100|Front wheels turns 20 degrees to the right|
|Send angle value 50|Front wheels centeres|
|Send angle value 100|Front wheels turns 20 degrees to the right|
|Send angle value 0|Front wheels turns 20 degrees to the left|
|Send angle value 50|Front wheels centeres|

## Distance node
| Action | Expected outcome |
|:---| :---: |
|Connect and start affected nodes properly| --- |
|Read correctly formated data via SPI interface from node| Data received (Can be verified by adding logging and reading logs on the receiver node) |
|Hold a solid object 1 decimeter in front of the left distance sensor, but not the center or right one|The node sends distance values ~10, X, Y where X and Y are any distances sensed by the middle or right sensor. ~ means within an error margin of 2 cm |
|Hold a solid object 1 decimeter in front of the right distance sensor, but not the center or right one|The node sends distance values X, Y, ~10 where X and Y are any distances sensed by the left or middle sensor. ~ means within an error margin of 2 cm  |
|Hold a solid object 1 meter in front of the car|The node sends distance values ~100, ~100, ~100.  ~ means within an error margin of 20 cm | 
|Keep the area from 0 meter to 1 meter in front of the car clear|The node sends distances >100, >100, >100|

## Speed node
| Action | Expected outcome |
|:---| :---: |
|Connect and start affected nodes properly| --- |
|Read correctly formated data via SPI interface from node| Data received (Can be verified by adding logging and reading logs on the receiver node) |
|Keep the wheel still|The node continuously sends a speed value of 0|
|Start turning the wheel at some approximate rate|The node sends speed value of X cm/s where X is the distance in cm traveled per second if the wheel would turn at the current rate and be placed on the ground|
|Start turning the wheel at some other approximate rate|The node sends speed value of X cm/s where X is the distance in cm traveled per second if the wheel would turn at the current rate and be placed on the ground|
|Keep the wheel still|The node starts to continuously send a speed value of 0 within one second|

## Motor node
| Action | Expected outcome |
|:---| :---: |
|Connect and start affected nodes properly| --- |
|Send correctly formated data via SPI interface to node| Data transmitted (Can be verified by adding logging and reading logs via the serial port of the motor node) |
|Send target speed value 10 with the current speed value 0 | The motor starts running and runs faster and faster with time, but stops before it reaches less than half of its maximum power|
|Stop, connect to a fully functioning speed sensing node and add speed data to the data sent via SPI, and start the motor node|The speed sensor starts logging values in cm/s|
|Send a target speed value of 10 with the meassured speed values and continuously send meassured speed values and the target speed values as soon as they are updated by the speed sensor node|The motor starts running and modulates itself until the speed of 10 cm/s is read by the speed sensor and maintains this speed after at most 1 second of modulation |
|Send a target speed value of 0 with the meassured speed values and continuously send meassured speed values as before|The motor slows down to a halt in less than a second|
|Send a target speed value of 120 cm/s with the meassured speed values as before|The motor starts running and | The motor starts running and modulates itself until a steady speed of something less than 120 cm/s is read by the speed sensor, due to limitations set in the motor node, and maintains this speed after at most 1 second of modulation |
|Send a target speed value of 20 with the meassured speed values as before|The motor slows down and modulates itself until the speed of 20 cm/s is read by the speed sensor and maintains this speed after at most 1 second of modulation|
|Send a target speed value of 0 with the meassured speed values and continuously send meassured speed values as before|The motor slows down to a halt in less than a second|

## Distance-Steering controller
| Action | Expected outcome |
|:---| :---: |
|Connect and start affected nodes and VSOME/IP clients properly (assume functioning distance node) | --- |
| Read SPI messages and messages on the VSOME/IP protocol for distance data, sent by the node | A stream of distances correlating to the closets obstacle in front of the car will be read |
| Put something in front of the car at some approximate distance | A stream of distances correlating to the closets obstacle in front of the car will be read on the VSOME/IP channel for distance data |
| Repeat last step to verify it works for a set of approximate distances | --- |
| Send a value of 0 over the VSOME/IP protocol for steering | The SPI channel for steering will read 0x00 |
| Send a value of 100 over the VSOME/IP protocol for steering | The SPI channel for steering will read 0x64 (100 in decimal) |
| Send a value of 50 over the VSOME/IP protocol for steering | The SPI channel for steering will read 0x32 (50 in decimal) |

## Motor-Speed controller
| Action | Expected outcome |
|:---| :---: |
|Connect and start affected nodes and VSOME/IP clients properly| --- |
| Read SPI messages and messages on the VSOME/IP protocol for speed data, sent by the node | A continuous stream of 0x80 will be read (due to the first bit acting as a data type id)|
| Prepare a byte 0x10 to send over the other SPI channel to the node | The first SPI channel will read a continuous stream of 0x90 (due to the first bit acting as a data type id). </br> The VSOME/IP message for speed data will read the value 16|
| Send a value of 0x20 over the VSOME/IP protocol for target speed | The first SPI channel will read 0x20 followed by a continuous stream of 0x90 (as before) |  
| Prepare a byte 0x00 to send over the other SPI channel to the node | The first SPI channel will read a continuous stream of 0x80 (due to the first bit acting as a data type id). </br> The VSOME/IP message for speed data will read the value 0 |
| Send a value of less than 0x16 (30 in decimal) on the VSOME/IP protocol for distance | The first SPI channel will read a 0x00 within less than a second |
| Send a value of more than 0x64 (100 in decimal) on the VSOME/IP protocol for distance | The first SPI channel will read a 0x20 within less than a second |
| Send a value of 0x64 (100 decimal) on the VSOME/IP protocol for minimum distance | No change |
| Send a value of less than 0x64 (100 in decimal) and more than 0x16 on the VSOME/IP protocol for distance | The first SPI channel will read a 0x00 within less than a second |
| Send a value of 0x30 over the VSOME/IP protocol for target speed | No change |  
| Send a value of more than 0x64 (100 in decimal) on the VSOME/IP protocol for distance | The first SPI channel will read a 0x30 within less than a second |
| Send a value of 0x00 over the VSOME/IP protocol for target speed| The first SPI channel will read 0x00 followed by a continuous stream of 0x80 (as before) |  

## Vision controller  
| Action | Expected outcome |
|:---| :---: |
|Connect and start affected nodes properly| --- |
|Hold something with a red colour in front of the camera (Android app, or mobile webpage can be found in the project git repositories)|The vision controller should send the message "1*X* " within 1 second, where 1 means red, and X is any direction towards the red thing|
|Hold something with a green colour in front of the camera but a bit to the left|The vision controller sends the message "21" within 1 second, where 2 means green, and 1 means left|
|Hold something with a green colour in front of the camera but a bit to the right|The vision controller sends the message "22" within 1 second, where 2 means green, and the second 2 means right|
|Hold something with a green colour in front of the camera but center in view|The vision controller sends the message "23" within 1 second, where 2 means green, and the 3 means center|
|Hold something with a red colour in front of the camera but a bit to the left|The vision controller sends the message "11" within 1 second, where 1 means red, and the second 1 means left|
|Hold something with a red colour in front of the camera but a bit to the right|The vision controller sends the message "12" within 1 second, where 1 means red, and the 2 means right|
|Hold something with a red colour in front of the camera but center in view|The vision controller sends the message "13" within 1 second, where 1 means red, and the 3 means center|
|Hold something with a yellow colour in front of the camera but a bit to the left|The vision controller sends the message "31" within 1 second, where 1 means red, and the 1 means left|
|Hold something with a yellow colour in front of the camera but a bit to the right|The vision controller sends the message "32" within 1 second, where 1 means red, and the 2 means right|
|Hold something with a yellow colour in front of the camera but center in view|The vision controller sends the message "33" within 1 second, where 1 means red, and the second 3 means center|
|Keep very green, very red, and very yellow coloured object away from the camera field of view|The vision controller sends "00", which represents no control flag object in view|

## Car controller  
| Action | Expected outcome |
|:---| :---: |
|Connect and start affected nodes properly| --- |
| _TODO_ ||