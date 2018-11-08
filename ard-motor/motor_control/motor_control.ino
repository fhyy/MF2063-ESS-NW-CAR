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
  
  Serial.println(lastEncoderVal);
  Serial.println(targetSpeed);
  Serial.println(currentPWM);
  Serial.println(errorSum);
  //Serial.println(ESC_MIN + (ESC_MAX - ESC_MIN)*dutyCycle);

  int pwm = (ESC_MIN + (ESC_MAX - ESC_MIN) * dutyCycle);
  if (pwm > ESC_MAX_CAP) {
    pwm = ESC_MAX_CAP;
  }
  esc.writeMicroseconds(pwm);
  //Serial.println(pwm);
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
}

void setApproximationParameters()
{
  float pwmDiffPerCycle = ((float)(pwmBeforeLastEncoderVal-pwmLastEncoderVal))/(timeBeforeLastEncoder+0.01);
  encoderDiffPerPWMCycle = pwmDiffPerCycle*((float)lastEncoderVal-encoderValBeforeLast)/(timeBeforeLastEncoder+0.01);
}

ISR(SPI_STC_vect)
{
    while (!(SPSR & (1<<SPIF))){}; // Wait for the end of the transmission
    byte spi_in = SPDR;
    if(spi_in != 0){
      Serial.print("Spi in: ");
      Serial.println(spi_in);
    }
    if(spi_in < 0){
      readSpeed = true;
      readEncoder = false;
    }else if(readSpeed){
      
      // Assume cm/s
      targetSpeed = spi_in;
      
      readSpeed = false;
      readEncoder = true;
    }else if(readEncoder){
      encoderValBeforeLast = lastEncoderVal;

      // Assume cm/s
      lastEncoderVal = spi_in;
      
      if(timeLastEncoderUpdate == 0){
        timeLastEncoderUpdate = millis();
        delay(1);
      }
      float timeDiff = ((float)millis()-timeLastEncoderUpdate)/1000.0;
      timeBeforeLastEncoder = timeSinceLastEncoder;
      timeSinceLastEncoder = timeDiff;
      pwmBeforeLastEncoderVal = pwmLastEncoderVal;
      pwmLastEncoderVal = currentPWM;

      setApproximationParameters();
      
      readSpeed = true;
      readEncoder = false;
      timeLastEncoderUpdate = millis();
    }
} // end of interrupt routine

// simulated SPI interrupt routine
void s_ISR(byte data) //ISR(SPI_STC_vect)
{
    byte spi_in = data;
    Serial.print("Simulated Spi in: ");
    Serial.println(spi_in);
    if(spi_in == 0){
      readSpeed = true;
      readEncoder = false;
    }else if(readSpeed){
      
      // Assume cm/s
      targetSpeed = spi_in-1;
      Serial.print("targetSpeed: ");
      Serial.println(targetSpeed);
      readSpeed = false;
      readEncoder = true;
    }else if(readEncoder){
      encoderValBeforeLast = lastEncoderVal;

      // Assume cm/s
      lastEncoderVal = spi_in-1;
      Serial.print("encoderVal: ");
      Serial.println(lastEncoderVal);
      if(timeLastEncoderUpdate == 0){
        timeLastEncoderUpdate = millis();
        delay(1);
      }
      float timeDiff = ((float)millis()-timeLastEncoderUpdate)/1000.0;
      timeBeforeLastEncoder = timeSinceLastEncoder;
      timeSinceLastEncoder = timeDiff;
      pwmBeforeLastEncoderVal = pwmLastEncoderVal;
      pwmLastEncoderVal = currentPWM;

      //setApproximationParameters();
      
      readSpeed = false;
      readEncoder = false;

      timeLastEncoderUpdate = millis();
    }
} // end of interrupt routine

char setupComplete = 0;
void setupAndCallibrateESC()
{
  esc.attach(MOTOR_PIN);  // Attach servo to pin

  esc.writeMicroseconds(ESC_MAX); // Send calibration signal to ESC (also max value)

  // Start ESC during this time
  delay(ESC_CALIBRATION_DELAY);   // Wait for ESC to acknowledge calibration signal

  esc.writeMicroseconds(ESC_MIN);  // Send minimum signal value for calibration

  delay(ESC_CALIBRATION_DELAY); // Wait for ESC to acknowledge signal

  esc.writeMicroseconds(ESC_ARM); // Send arm signal to ESC
  delay(1000);
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
  startSPI();

  Serial.begin(9600);
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

  // PID controlling
  //float pwmDiffPerCycleNow = (pwmLastEncoderVal-currentPWM)/(timeSinceLastEncoder+0.01);
  byte approximateEncoderVal = lastEncoderVal; // + (timeSinceLastEncoder * encoderDiffPerPWMCycle * pwmDiffPerCycleNow);

  /*Serial.println("---------------------");
  Serial.print("encoderDiffPerPWMCycle: ");
  Serial.println(encoderDiffPerPWMCycle);
  Serial.print("pwmDiffPerCycleNow: ");
  Serial.println(pwmDiffPerCycleNow);
  Serial.print("approximateEncoderVal: ");
  Serial.println(approximateEncoderVal);
  Serial.println("=====================");*/
  
  float error = ((float)(targetSpeed - approximateEncoderVal));
  errorSum += error*timeDiffS;
  currentPWM = error*PID_P + errorSum*PID_I;
  setMotorPWM(currentPWM);

  time_last = time_new;

  // Simulation!
  /*if(millis()-timeLastEncoderUpdate > 3000 + random(1000)){
    s_ISR(0);
    s_ISR(20+random(80)+1);
    s_ISR(targetSpeed - random(40)+1);
    timeLastEncoderUpdate = millis();
  }*/
  delay(10);
}
