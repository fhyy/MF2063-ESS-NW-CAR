//
// ard-speed.ino: Calculates the vehicle speed using information from the IR-sensor.
//
// Authors:
//      Jacob Kimblad (jacobki@kth.se)
//
// License:
//      AFL 3.0 
//

#include "Time.h"
#include "TimeAlarms.h"
// Libraries isntalled from
// https://www.pjrc.com/teensy/td_libs.html

//Constants
int const MAX_STEPS = 5;
int const TIMEOUT_TIME = 1000;
int const WHEEL_RADIUS_MM = 55;
int const WHEEL_CIRCUMFERENCE = 2 * PI * WHEEL_RADIUS_MM;
int const STEPS_PER_ROTATION = 5;
float const DISTANCE_PER_STEP = (float)WHEEL_CIRCUMFERENCE / (float)STEPS_PER_ROTATION;
//Pins
int const IR_PIN = 2;
//Global variables
bool timeoutFlag = false;
int steps = 0; 
// Pin readings
int irVal = 0;      //Int to store IR-value

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

    // Start time-measurments
    unsigned long startTime = millis();

    //Wait until sufficient data is captured
    while(steps < MAX_STEPS && !timeoutFlag) {
        if (millis() - startTime > TIMEOUT_TIME) {
            setTimeoutFlag();
        }
        //Check if 1s of waiting has passed and call setTimeoutFLag
        //Checks state of timeout timer
    } 

    // Stop time-measurements
    unsigned long stopTime = millis();
    unsigned long elapsedTime = stopTime - startTime;

    // Calculate actual speed
    float speed = calculateSpeed(elapsedTime);

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
float calculateSpeed(unsigned long elapsedTime){
    int distance = steps * DISTANCE_PER_STEP; //Gives distance in millimeters
    float speedMMPerS = (float)(distance*1000)/(float)elapsedTime; //Gives distance*1000/time (mm per s)

    return speedMMPerS;
}

