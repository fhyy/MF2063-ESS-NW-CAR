#include <Servo.h>
#include <SPI.h>

#define MOTOR_PIN 10 //digital pin 12 Mega, digital pin 10 Micro
#define ESC_MAX 2000
#define ESC_MAX_CAP 1500
#define ESC_MIN 1000
#define ESC_ARM 500
#define ESC_CALIBRATION_DELAY 5000

Servo esc;

byte targetSpeed = 0;
byte lastEncoderVal = 0;
byte encoderValBeforeLast = 0;
float timeSinceLastEncoder = 0;
float timeBeforeLastEncoder = 0;

long timeLastEncoderUpdate = 0;

float currentPWM = 0;
float pwmLastEncoderVal;
float pwmBeforeLastEncoderVal;
float errorSum = 0;

float encoderDiffPerPWMCycle = 0;



float PID_P = 0.005;
float PID_I = 0.50;

/*
   dutyCycle of 0% => standing still
   dutyCycle of 0% < x < 100% => driving forward
   dutyCycle of 100% => driving forward full speed
*/
void setMotorPWM(float dutyCycle)
{ 
  if (dutyCycle < 0)
    dutyCycle = 0.0f;
  if (dutyCycle > 1) {
    dutyCycle = 1.0f;
  }

  currentPWM = dutyCycle;
  
  //Serial.println(ESC_MIN + (ESC_MAX - ESC_MIN)*dutyCycle);

  int pwm = (ESC_MIN + (ESC_MAX - ESC_MIN) * dutyCycle);
  if (pwm > ESC_MAX_CAP) {
    pwm = ESC_MAX_CAP;
  }
  esc.writeMicroseconds(pwm);
  //Serial.println(pwm);
  //Serial.println(lastEncoderVal);
  //Serial.println(targetSpeed);
  //Serial.println(currentPWM);
  //Serial.println(errorSum);
}

bool readSpeed = 0;
bool readEncoder = 0;

// Initialize spi
void startSPI()
{
  SPCR |= (1 << SPE) | (1 << SPIE); //SPI control register ,enable spi interrupt and spi
  SPSR |= (1 << SPIF); //SPI status register
  SPI.attachInterrupt();
  pinMode(MOSI, INPUT);

  Serial.println("Spi set up!");
}

void setApproximationParameters()
{
  float pwmDiffPerCycle = ((float)(pwmBeforeLastEncoderVal-pwmLastEncoderVal))/(timeBeforeLastEncoder+0.01);
  encoderDiffPerPWMCycle = pwmDiffPerCycle*((float)lastEncoderVal-encoderValBeforeLast)/(timeBeforeLastEncoder+0.01);
}

ISR(SPI_STC_vect)
{
  //while (!(SPSR & (1<<SPIF))){}; // Wait for the end of the transmission
  
  byte spi_in = SPDR;//SPI.transfer(0);
  Serial.print((spi_in&0x80)>>7);
  Serial.print((spi_in&0x40)>>6);
  Serial.print((spi_in&0x20)>>5);
  Serial.print((spi_in&0x10)>>4);
  Serial.print(" ");
  Serial.print((spi_in&0x08)>>3);
  Serial.print((spi_in&0x04)>>2);
  Serial.print((spi_in&0x02)>>1);
  Serial.print(spi_in&0x01);
  Serial.println();
  if(spi_in & 0x80){
    // Assume cm/s
    lastEncoderVal = spi_in;
  }else{
    // Assume cm/s
    targetSpeed = spi_in;
  }
 
	Serial.print("Target: ");
  Serial.println((int)targetSpeed);
	Serial.print("Current: ");
  Serial.println((int)lastEncoderVal);
	
	// Variables used for predicting speed change
	float timeDiff = ((float)millis()-timeLastEncoderUpdate)/1000.0;
	timeBeforeLastEncoder = timeSinceLastEncoder;
	timeSinceLastEncoder = timeDiff;
	pwmBeforeLastEncoderVal = pwmLastEncoderVal;
	pwmLastEncoderVal = currentPWM;

	setApproximationParameters();
	timeLastEncoderUpdate = millis();
}

char setupComplete = 0;
void setupAndCallibrateESC()
{
  esc.attach(MOTOR_PIN);  // Attach servo to pin

  esc.writeMicroseconds(ESC_MIN);  // Send minimum signal value for calibration

  /*delay(ESC_CALIBRATION_DELAY); // Wait for ESC to acknowledge signal
  
  esc.writeMicroseconds(ESC_MAX); // Send calibration signal to ESC (also max value)

  // Start ESC during this time
  delay(ESC_CALIBRATION_DELAY);   // Wait for ESC to acknowledge calibration signal

  esc.writeMicroseconds(ESC_MIN);  // Send minimum signal value for calibration

  delay(ESC_CALIBRATION_DELAY); // Wait for ESC to acknowledge signal

  esc.writeMicroseconds(ESC_ARM); // Send arm signal to ESC
  delay(1000);
  */
  setupComplete = 1;
}

// the setup function runs once when you press reset or power the board
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

unsigned long time_last;
unsigned long time_new;

// the loop function runs over and over again forever
void loop()
{
  if(time_last == time_new){
    time_last = millis();
    return;
  }

  time_new = millis();
  float timeDiffS = ((float)(time_new-time_last))/1000;

  // Forecasting speed change
  //float pwmDiffPerCycleNow = (pwmLastEncoderVal-currentPWM)/(timeSinceLastEncoder+0.01);
  byte approximateEncoderVal = lastEncoderVal; // + (timeSinceLastEncoder * encoderDiffPerPWMCycle * pwmDiffPerCycleNow);
  
  // PID controlling
  float error = ((float)(targetSpeed - approximateEncoderVal));
  errorSum += error*timeDiffS;
  currentPWM = error*PID_P + errorSum*PID_I;
  setMotorPWM(currentPWM);

  time_last = time_new;

  delay(10);
}
