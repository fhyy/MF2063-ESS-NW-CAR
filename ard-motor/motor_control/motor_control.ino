#include <Servo.h>
#include <SPI.h>

/** \def MOTOR_PIN
	The pin number of which the ESC data pin should be connected to.
*/
/** \def ESC_MAX
	Maximum PWM for ESC.
*/
/** \def ESC_MIN
	Minimum PWM for ESC for a neutral state.
*/
/** \def ESC_MAX_CAP
	A hard coded maximum PWM for the ESC.
	Adjust this to account for different loads.
*/
/** \def ESC_CALIBRATION_DELAY
	Time in ms before any PWM is sent to the motor on boot.
*/


#define MOTOR_PIN 10
#define ESC_MAX 2000
#define ESC_MAX_CAP 1300
#define ESC_MIN 1000
#define ESC_CALIBRATION_DELAY 5000

float PID_P = 0.005;	/**< Used for the proportional control in the PID controller. Adjust this as necessary */
float PID_I = 0.005;	/**< Used for the integral term in the PID controller. Adjust this as necessary */

Servo esc;	/** < Used to to send PWM signals to the ESC */

byte intermediateTargetSpeed = 0; /**< Used to smoothen speed changes */
byte targetSpeed = 0; /**< Set through SPI to define the target speed */
byte lastEncoderVal = 0; /**< Set through SPI for encoder feedback, used by PID controller */

float lastPWM = 0; /**< Used to check the state of motor and to handle the ESC safety measures described in \ref loop "loop()"*/
float errorSum = 0; /**< Used with the integral term in the PID controller */

/** 
	\brief A function for setting the motor PWM from a percentage value.

	This function is used to convert a percentage into a PWM signal in the valid range.
	
	dutyCycle == 0.0 => standing still \n
	0.0 < dutyCycle < 1.0 => driving forward \n	
	dutyCycle == 1.0 => driving forward full speed \n
*/
void setMotorPWM(float dutyCycle)
{ 
  if (dutyCycle < 0){
    dutyCycle = 0.0f;
  }else if (dutyCycle > 1) {
    dutyCycle = 1.0f;
  }

  int pwm = (ESC_MIN + (ESC_MAX_CAP - ESC_MIN) * dutyCycle);
  if (pwm > ESC_MAX_CAP) {
    pwm = ESC_MAX_CAP;
  }
  
  esc.writeMicroseconds(pwm);
  
  lastPWM = dutyCycle;
}

/**
	\brief A function for setting up the SPI communication using interrupts.
*/
void startSPI()
{
  SPCR |= (1 << SPE) | (1 << SPIE); //SPI control register ,enable spi interrupt and spi
  SPSR |= (1 << SPIF); //SPI status register
  SPI.attachInterrupt();
  pinMode(MOSI, INPUT);

  Serial.println("Spi set up!");
}

/**
	\brief An interrupt handler for SPI communication.
*/
ISR(SPI_STC_vect)
{  
  //Serial.println("SPI interrupt");
  byte spi_in = SPDR;//SPI.transfer(0);
  if(spi_in & 0x80){
    // Assume cm/s
    lastEncoderVal = spi_in & (~0x80);
  }else{
    // Assume cm/s
    targetSpeed = spi_in;
  }
}

/**
	\brief A function for setting up communication with the ESC.
*/
void setupAndCallibrateESC()
{
  esc.attach(MOTOR_PIN);  // Attach servo to pin

  esc.writeMicroseconds(ESC_MIN);  // Send minimum signal value for calibration

  // Start ESC before this step
  // Wait for ESC to acknowledge signal
  delay(ESC_CALIBRATION_DELAY);
}

/**
	\brief Ardunino setup function. Runs once when the board is started.
*/
void setup()
{
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH); // LED is on when setting up motor
  setupAndCallibrateESC();
  digitalWrite(LED_BUILTIN, LOW); // LED is off when setup is complete

  setMotorPWM(0.01f);
  delay(100);
  setMotorPWM(0.0f);
  delay(100);
  Serial.begin(9600);
  Serial.println("Serial started");
  startSPI();
}

unsigned long time_last;	/**< Used by the program to calculate time difference */

int iterationsFromStop = 0;	/**< Used for handling ESC safety measures and avoiding noisy data from SPI */

/**
	\brief Arduino loop function. Runs over and over, until the Arduino is turned off. Starts after the \ref setup "setup()" has finished.
	
	This loop can be configured in the source code to do one of two things:
	1. Calculate a PID signal used for setting the target PWM signal, so that the car maintains a set speed.
	2. Translate the given target speed into a target PWM signal, in case the PID parameters are not set up properly.
	
	The PWM signal sent to the motor is a signal that is incremented or decremented each step to try and become equal to the set target PWM. This makes the motor run more smoothly and reduces the wear-out time.
	
	The ESC has a specific case where if the PWM is set to something higher than the neutral value (somewhere between 1000 and 1100) and the motor can't turn due to some load and friction, the ESC will set the motor PWM to neutral by itself to avoid tear. To be able to start the motor again, you have to send the ESC a neutral PWM for a few milliseconds, and then continue with your higher values. This is taken care of in the loop as well.
*/
void loop()
{
  /*
	The ESC has some safety measures implemented.
	In the given case, we have to set the pwm to 0 before we can set it to anything else.
   */
  if(targetSpeed > 0 &&
    lastEncoderVal == 0 &&
    lastPWM != 0){
        if(iterationsFromStop <= 10){
          ++iterationsFromStop;
        }else{
          setMotorPWM(0);
          Serial.println("Stopping to reenable the motor!");
          delay(500);
          iterationsFromStop = 0;
        }
  }else{
    iterationsFromStop = 0;
  }
  
  if(time_last == 0){
    time_last = millis();
    return;
  }

  /*
	  Enable this block of code to read target speed from the serial interface (e.g. serial monitor in Arduino studio)
  */
  /* TO ENABLE, SET A / BEFORE /*, REMOVE TO DISABLE
  if (Serial.available()){
    int input = Serial.parseInt();
    targetSpeed = input;
  }
  //*/

  unsigned long time_new = millis();
  float timeDiffS = ((float)(time_new-time_last))/1000;

  for(int i = 0; i < 2; ++i){
    if(intermediateTargetSpeed < targetSpeed){
      ++intermediateTargetSpeed;
    }else if(intermediateTargetSpeed > targetSpeed){
      --intermediateTargetSpeed;
    }
  }

  /*
	PID controlling
   */
  float error = ((float)(intermediateTargetSpeed  - lastEncoderVal));
  errorSum += error*timeDiffS;

  int errorSumMax = (1.5f/PID_I);
  if(errorSum > errorSumMax){
    errorSum = errorSumMax;
  }else if(errorSum < 0){
    errorSum = 0;
  }

  /* 
    Switch between these lines if PID is wanted or not
   */
  float targetPWM = ((float)intermediateTargetSpeed)/100.0f; // NO PID
  //targetPWM = error*PID_P  + ((errorSum)*PID_I);     // PID

  setMotorPWM(targetPWM);
  
  time_last = time_new;

  delay(20);
}

