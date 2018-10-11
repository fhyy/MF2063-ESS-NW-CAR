//
// ard-speed.ino: Calculates the vehicle speed using information from the IR-sensor.
//
// Authors:
//      Jacob Kimblad (jacobki@kth.se)
//
// License:
//      AFL 3.0 
//

#include "SimpleTimer.h"
// Library installed from:
// https://playground.arduino.cc/code/SimpleTimer

//Constants
int const MAX_STEPS = 5;
int const TIMEOUT_TIME = 10000;
int const WHEEL_RADIUS_MM = 55;
int const WHEEL_CIRCUMFERENCE = 2 * PI * WHEEL_RADIUS_MM;
int const STEPS_PER_ROTATION = 5;
int const DISTANCE_PER_STEP = WHEEL_CIRCUMFERENCE / STEPS_PER_ROTATION;
//Pins
int const IR_PIN = 22;
//Global variables
bool timeoutFlag = false;
int steps = 0; 
// Pin readings
int irVal = 0;      //Int to store IR-value
//Timers
SimpleTimer timer1;

void setup() {
    //Init serial communication to enable prints
    Serial.begin(115200);

    //Try printing
    Serial.println("Serial communication set up!");

    //Setup IR-input to interrupt mcu
    pinMode(IR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(IR_PIN), irInterruptHandler, RISING);

}

// put your main code here, to run repeatedly:
void loop() {
    //Start timeout countdown until next measurment update
    int timerID = timer1.setTimeout(TIMEOUT_TIME, setTimeoutFlag);
    Serial.println(timer1.getNumTimers());

    // Start time-measurments
    unsigned long startTime = millis();

    //Wait until sufficient data is captured
    while(steps < MAX_STEPS && !timeoutFlag); 

    // Stop time-measurements
    unsigned long stopTime = millis();
    unsigned long elapsedTime = stopTime - startTime;

    int speed = calculateSpeed(elapsedTime);

    //Reset flags before next loop
    timeoutFlag = false;
    steps = 0;
}

// Callback-function from timer
void setTimeoutFlag(){
   timeoutFlag = true;
}

// IR-sensor interrupt handler
void irInterruptHandler(){
    steps++;
}

// Calculate speed using elapsed time and distance traveled
int calculateSpeed(unsigned long elapsedTime){
    // distance traveled = steps * WHEEL_CIRCUMFERENCE / STEPS_PER_ROTATION
    int distance = steps * DISTANCE_PER_STEP; //Gives distance in millimeters
    float speedMMPerMS = distance/(elapsedTime); //Gives distance/time (mm per ms)
    //Uncomment to calculate meters per second
    /*unsigned float speedMPerS = speedMMPerMS * 1000; //Gives speed in meters per second*/

    return speedMMPerMS;
}

