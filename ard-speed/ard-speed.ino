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
int const MAX_STEPS = 1;
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
unsigned long startTime = 0;
unsigned long stopTime = 0;
// Pin readings
int irVal = 0;      //Int to store IR-value
//Structs
struct speedStruct {
    byte integerPart;
    byte decimalPart;
};
int speedValue;

void setup() 
{
    //Init serial communication to enable prints
    Serial.begin(115200);

    //Setup spi
    startSPI();

    //Setup IR-input to generate interrupts
    pinMode(IR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(IR_PIN), irInterruptHandler, RISING);

    //Start time for first iteration
    startTime = millis();
}

// put your main code here, to run repeatedly:
void loop() 
{

    //Wait until sufficient data is captured
    while(steps < MAX_STEPS && !timeoutFlag) {
        if (millis() - startTime > TIMEOUT_TIME) {
            timeoutFlag = false;
        }
    } 

    // Stop time-measurements
    stopTime = millis();
    unsigned long elapsedTime = stopTime - startTime;
    // Start time-measurments for next loop
    startTime = millis();

    // Calculate actual speed
    calculateSpeed(elapsedTime);


    //Reset flags before next loop
    speedSent = false;
    /*decimalSent = false;*/
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
    if(!speedSent) {
        SPDR = speedValue + 1;
        speedSent = true;
        /*Serial.print("Sending: ");*/
        /*Serial.println(speedValue + 1);*/
    }
} // end of interrupt routine 

void toSpeedStruct(float speed) 
{
  speedValue = (int)(speed);
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
    float speedCMPerS = (float)(distance*100)/(float)elapsedTime; //Gives distance*1000/time (cm per s)
    speedValue = speedCMPerS;
}

