The current scenario machine is for the most basic scenario, scenario1.
## Scenario description:
The initial state of the car is standing still. A run flag is put in front of the car (distance > DISTANCE_STOP). 
When the car detects the flag, it starts accelerating until reaching the constant speed we defined. The car will stop when a stop flag is detected or the distance between car & flag is smaller than DISTANCE_STOP (use 100cm in the code currently)
## Compile the code
compile the code with
```compile cmd
gcc -o nameYouWant test_sm.c car_controller_statemachine.c
```
## Test cases passed
### testcase 1
start at a low distance, acclerate until the constant speed reached. stop when the distance data is smalller then DISTANCE_STOP. Camera flag will always be RUN.
### testcase 2
start at a low distance, acclerate until the constant speed reached. stop when a STOP flag is detected. distance data is larger than DISTANCE_STOP.