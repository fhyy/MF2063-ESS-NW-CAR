/**@file ard-speed.ino */

//
// ard-speed.ino: Calculates the vehicle speed using information from the IR-sensor.
//
// Authors:
//      Jacob Kimblad (jacobki@kth.se)
//
//

#include <SPI.h>

//Constants
int const MAX_STEPS = 1;
int const TIMEOUT_TIME = 1000;
int const WHEEL_RADIUS_MM = 55;
int const WHEEL_CIRCUMFERENCE = 2 * PI * WHEEL_RADIUS_MM;
int const STEPS_PER_ROTATION = 10;
float const DISTANCE_PER_STEP = (float)WHEEL_CIRCUMFERENCE / (float)STEPS_PER_ROTATION;

//Pins
int const IR_PIN = 2;

//Global variables
bool timeoutFlag = true;
int steps = 0; 
unsigned long startTime = 0;
unsigned long stopTime = 0;
unsigned long elapsedTime = 0;

// Pin readings
int irVal = 0;      //Int to store IR-value
int speedValue;

/// This is the setup, this is run once before we enter the loop function which never returns. Any code that should run once on boot before we assume normal behaviour should be put in here.
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

/// This is the main loop that is continously itterated while the arduino is powered on. Any code that should be run contiously should be put in here. 
void loop() 
{

    //Wait until sufficient data is captured
    while(steps < MAX_STEPS) {
        if (millis() - startTime > TIMEOUT_TIME) {
            speedValue = 0;
        }
    } 

    // Stop time-measurements
    stopTime = millis();
    elapsedTime = stopTime - startTime;
    // Start time-measurments for next loop
    startTime = millis();

    // Calculate actual speed
    calculateSpeed();

    timeoutFlag = false;
    steps = 0;
}

/// Initialises the SPI such that the arduino is configured as a slave serving the BBB master
void startSPI() 
{
    SPCR |= (1<<SPE)|(1<<SPIE); //SPI control register ,enable spi interrupt and spi
    SPSR |= (1<<SPIF);//SPI status register
    SPI.attachInterrupt();
    pinMode (MISO, OUTPUT);
}

/// SPI interrupt routine, interrupts are generated by the master enabling the arduinos Slave Select (SS) port by setting it to low. This means we can enter this interrupt routine and send the desired data
ISR(SPI_STC_vect)
{ 
    SPDR = speedValue;
    //Serial.print("Sending: ");
    //Serial.println(speedValue);

} 

/// The IR-sensor interrupt handler
void irInterruptHandler()
{
    steps++;
    //Serial.println("Step");
}

/// Calculates speed using elapsed time and distance traveled
float calculateSpeed()
{
    int distance = steps * DISTANCE_PER_STEP; //Gives distance in millimeters
    float speedCMPerS = (float)(distance*100)/(float)elapsedTime; //Gives distance*1000/time (cm per s)
    speedValue = speedCMPerS;
}

