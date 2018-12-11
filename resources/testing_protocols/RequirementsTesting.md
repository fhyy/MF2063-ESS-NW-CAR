## Car start up
| Action | Expected outcome |
|:---| :---: |
|Turn on power of the car| Blinky lights on the hardware turns on. ESC makes beeping noise within 5 seconds (the pattern should be _beep-beep-boooop_ if a 2-cell battery is used). Front wheels are centered if start orientation is off-center. |

## Flag following
| Action | Expected outcome |
|:---| :---: |
|Start car| Car starts |
|Start flag app| App starts and shows a red or green screen |
|Get green flag from the app| App displays a green screen |
|Put phone with screen turned towards the car in view of the car camera| Car should move forwards until a distance of minimum 1 meter from the phone is reached, then stop |
|Move the phone sideways| Car should turn the wheels towards the direction of the phone within a couple of seconds |
|Move the phone away from the car| Car should move forwards until a distance of minimum of 1 meter from the phone is reached, then stop |
|Before car reaches the minimum distance, get red flag from the app| Car should stop within 2 seconds |

## Collision avoidance
| Action | Expected outcome |
|:---| :---: |
|Start car| Car starts |
|Start flag app| App starts and shows a red or green screen |
|Get green flag from the app| App displays a green screen |
|Put phone with screen turned towards the car in view of the car camera| Car should move forwards |
|Before the car reaches a distance of minimum 1 meter from the phone, put an obstacle directly infront of the car|The car should stop before hitting the obstacle|
|Put phone with screen turned towards the car in view of the car camera| Car should move forwards |
|Before the car reaches a distance of minimum 1 meter from the phone, put an obstacle infront of the car but off-center to the left or right |The car should stop before hitting the obstacle and turn in the other direction|

## Fault states
| Action | Expected outcome |
|:---| :---: |
|Start car| Car starts |
|Test operations as above but do one of the following things:|Car operates as normal until fault is injected|
|---|---|
|Disconnect ethernet cable for distance-steering node|Car stops within 2 seconds and stays still until boot up without faulty nodes|
|Disconnect ethernet cable for motor-speed node|Car stops within 2 seconds and stays still until boot up without faulty nodes|
|Disconnect ethernet cable for SDN-controller node|Car stops within 2 seconds and stays still until boot up without faulty nodes|
|Disconnect ethernet cable for vision node|Car stops within 2 seconds and stays still until boot up without faulty nodes|
|Disconnect ethernet cable for car controller node|Car stops within 2 seconds and stays still until boot up without faulty nodes|
|Disconnect any spi pin for speed sensing node|Car stops within 2 seconds and stays still until boot up without faulty nodes|
|Disconnect any spi pin for distance sensing node|Car stops within 2 seconds and stays still until boot up without faulty nodes|

## Analytics _(TODO)_  
| Action | Expected outcome |
|:---| :---: |
|Start car| Car starts |
|Do something|Find something out by the analytics tables|
|_TODO_| |