## Flag following
| Action | Expected outcome | Results (2018-12-11) |
|:---| :---: | :---: |
|Start car| Car starts | Ok |
|Start flag app| App starts and shows a red or green screen | Ok. Yellow is possible as well |
|Get green flag from the app| App displays a green screen | Ok |
|Put phone with screen turned towards the car in view of the car camera| Car should move forwards until a distance of minimum 1 meter from the phone is reached, then stop | Not ok. State machine and vision not yet integrated properly |
|Move the phone sideways| Car should turn the wheels towards the direction of the phone within a couple of seconds | Not ok. State machine and vision not yet integrated properly |
|Move the phone away from the car| Car should move forwards until a distance of minimum of 1 meter from the phone is reached, then stop | Not ok. State machine and vision not yet integrated properly |
|Before car reaches the minimum distance, get red flag from the app| Car should stop within 2 seconds | Not ok. State machine and vision not yet integrated properly |

## Collision avoidance
| Action | Expected outcome | Results (2018-12-11) |
|:---| :---: | :---: |
|Start car| Car starts | Ok |
|Start flag app| App starts and shows a red or green screen | Ok. Yellow is possible as well |
|Get green flag from the app| App displays a green screen | Ok |
|Put phone with screen turned towards the car in view of the car camera| Car should move forwards | Not ok. State machine and vision not yet integrated properly |
|Before the car reaches a distance of minimum 1 meter from the phone, put an obstacle directly infront of the car|The car should stop before hitting the obstacle| Acceptable. Car stops within a reasonable time. Could only simulate |
|Put phone with screen turned towards the car in view of the car camera| Car should move forwards | Not ok. State machine and vision not yet integrated properly |
|Before the car reaches a distance of minimum 1 meter from the phone, put an obstacle infront of the car but off-center to the left or right |The car should stop before hitting the obstacle and turn in the other direction| Not ok. State machine and vision not yet integrated properly |

## Fault states
| Action | Expected outcome | Results (2018-12-11) |
|:---| :---: | :---: |
|Start car| Car starts | Ok |
|Test operations as above but do one of the following things:|Car operates as normal until fault is injected| --- |
|---|---| --- |
|Disconnect ethernet cable for distance-steering node|Car stops within 2 seconds and stays still until boot up without faulty nodes| Ok. Slightly more than 2 seconds with debug compilation. Could be improved? |
|Disconnect ethernet cable for motor-speed node|Car stops within 2 seconds and stays still until boot up without faulty nodes| Ok. Slightly more than 2 seconds with debug compilation. Could be improved? |
|Disconnect ethernet cable for SDN-controller node|Car stops within 2 seconds and stays still until boot up without faulty nodes| Ok. Slightly more than 2 seconds with debug compilation. Could be improved? |
|Disconnect ethernet cable for vision node|Car stops within 2 seconds and stays still until boot up without faulty nodes| Ok. Slightly more than 2 seconds with debug compilation. Could be improved? |
|Disconnect ethernet cable for car controller node|Car stops within 2 seconds and stays still until boot up without faulty nodes| Ok. Slightly more than 2 seconds with debug compilation. Could be improved? |
|Disconnect any spi pin for speed sensing node|Car stops within 2 seconds and stays still until boot up without faulty nodes| Not ok. Not implemented |
|Disconnect any spi pin for distance sensing node|Car stops within 2 seconds and stays still until boot up without faulty nodes| Not ok. Not implemented |
