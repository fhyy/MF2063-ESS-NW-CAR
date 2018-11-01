//
// ard-speed.ino: Calculates the vehicle speed using information from the IR-sensor.
//
// Authors:
//      Jacob Kimblad (jacobki@kth.se)
//
// License:
//      AFL 3.0 
//

#include <SPI.h>

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
bool timeoutFlag = true;
bool integerSent = true;
bool decimalSent = false;
int steps = 0; 
float speed = 0.0;
// Pin readings
int irVal = 0;      //Int to store IR-value
//Structs
struct speedStruct {
    byte integerPart;
    byte decimalPart;
};
speedStruct speedValue;

void setup() 
{
    //Init serial communication to enable prints
    Serial.begin(115200);

    //Setup spi
    startSPI();

    //Setup IR-input to generate interrupts
    pinMode(IR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(IR_PIN), irInterruptHandler, RISING);
}

// put your main code here, to run repeatedly:
void loop() 
{
    // Start time-measurments
    unsigned long startTime = millis();

    //Wait until sufficient data is captured
    while(steps < MAX_STEPS && !timeoutFlag) {
        if (millis() - startTime > TIMEOUT_TIME) {
            setTimeoutFlag();
        }
    } 

    // Stop time-measurements
    unsigned long stopTime = millis();
    unsigned long elapsedTime = stopTime - startTime;

    // Calculate actual speed
    calculateSpeed(elapsedTime);


    //Reset flags before next loop
    integerSent = false;
    decimalSent = false;
    timeoutFlag = false;
    steps = 0;
}

// Initialize spi
void startSPI() 
{
    SPCR |= (1<<SPE)|(1<<SPIE); //SPI control register ,enable spi interrupt and spi
    SPSR |= (1<<SPIF);//SPI status register
    SPI.attachInterrupt();
    pinMode (MISO, OUTPUT);
}

// SPI interrupt routine
ISR(SPI_STC_vect)
{ 
    //To differentiate on the receiver side between going at zero speed (0 received) and no information sent (also 0 received) we add the values with one before transmitting and the subtracting the values with 1 on the receiver side
    if(!integerSent) {
        SPDR = speedValue.integerPart + 1;
        integerSent = true;
    }
    else if(!decimalSent) {
        SPDR = speedValue.decimalPart + 1;
        decimalSent = true;
    }
} // end of interrupt routine 

void toSpeedStruct(float speed) 
{
  speedValue.integerPart = (int)(speed);
  speedValue.decimalPart = 100 * (speed - speedValue.integerPart); //10000 b/c float value always have exactly 4 decimal places
}

// Callback-function from timer
void setTimeoutFlag()
{
    timeoutFlag = true;
}

// IR-sensor interrupt handler
void irInterruptHandler()
{
    steps++;
}

// Calculate speed using elapsed time and distance traveled
float calculateSpeed(unsigned long elapsedTime)
{
    int distance = steps * DISTANCE_PER_STEP; //Gives distance in millimeters
    float speedMMPerS = (float)(distance*1000)/(float)elapsedTime; //Gives distance*1000/time (mm per s)

    toSpeedStruct(speedMMPerS);
}

