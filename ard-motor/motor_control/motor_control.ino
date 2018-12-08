#include <Servo.h>
#include <SPI.h>

#define MOTOR_PIN 10 //digital pin 12 Mega, digital pin 10 Micro
#define ESC_MAX 2000
#define ESC_MAX_CAP 1200 //Change this, to account for load
#define ESC_MIN 1000
#define ESC_ARM 500
#define ESC_CALIBRATION_DELAY 5000

//Adjust these PID parameters as needed
float PID_P = 0.005;
float PID_I = 0.005;

Servo esc;

byte intermediateTargetSpeed = 0;
byte targetSpeed = 0;
byte lastEncoderVal = 0;
byte encoderValBeforeLast = 0;



float targetPWM = 0;
float currentPWM = 0;
float lastPWM = 0;
float errorSum = 0;

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

  int pwm = (ESC_MIN + (ESC_MAX_CAP - ESC_MIN) * dutyCycle);
  if (pwm > ESC_MAX_CAP) {
    pwm = ESC_MAX_CAP;
  }
  
  esc.writeMicroseconds(pwm);
  
  //if(lastPWM != pwm){
    /*Serial.println("---");
    Serial.print("PWM: ");
    Serial.println(pwm);
    Serial.print("Target speed: ");
    Serial.println((int)targetSpeed);
    Serial.print("intermediate target speed: ");
    Serial.println((int)intermediateTargetSpeed);
    Serial.print("Current speed: ");
    Serial.println((int)lastEncoderVal);
    Serial.print("encoderValBeforeLast: ");
    Serial.println(encoderValBeforeLast);
    Serial.print("Error sum: ");
    Serial.println(errorSum);
    Serial.print("currentDuty: ");
    Serial.println(currentPWM);
    Serial.print("targetDuty: ");
    Serial.println(targetPWM);*/
  //}
  
  lastPWM = currentPWM;
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

ISR(SPI_STC_vect)
{
  //while (!(SPSR & (1<<SPIF))){}; // Wait for the end of the transmission
  
  byte spi_in = SPDR;//SPI.transfer(0);
  if(spi_in & 0x80){
    // Assume cm/s
    lastEncoderVal = spi_in & (~0x80);
  }else{
    // Assume cm/s
    targetSpeed = spi_in;
  }
}

char setupComplete = 0;
void setupAndCallibrateESC()
{
  esc.attach(MOTOR_PIN);  // Attach servo to pin

  //esc.writeMicroseconds(ESC_MAX); // Send calibration signal to ESC (also max value)

  // Start ESC during this time
  //delay(ESC_CALIBRATION_DELAY);   // Wait for ESC to acknowledge calibration signal

  esc.writeMicroseconds(ESC_MIN);  // Send minimum signal value for calibration

  delay(ESC_CALIBRATION_DELAY); // Wait for ESC to acknowledge signal

  //esc.writeMicroseconds(ESC_ARM); // Send arm signal to ESC
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
  Serial.begin(9600);
  Serial.println("Serial started");
  startSPI();
}

float floatMax(float val1, float val2){
  if(val1 < val2){
    return val2;
  }
  return val1;
}

unsigned long time_last;
unsigned long time_new;

int iterationsFromStop = 0;

// the loop function runs over and over again forever
void loop()
{
  /* 
   * The ESC has some safety meassures implemented in this case.
   * We have to set the pwm to 0 before we can set it to anything else.
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

  if (Serial.available()){
    int input = Serial.parseInt();
    targetSpeed = input;
  }

  time_new = millis();
  float timeDiffS = ((float)(time_new-time_last))/1000;

  // Climb towards target speed, intermediateTargetSpeed used in PID, Loop more for faster climb
  for(int i = 0; i < 2; ++i){
    if(intermediateTargetSpeed < targetSpeed){
      ++intermediateTargetSpeed;
    }else if(intermediateTargetSpeed > targetSpeed){
      --intermediateTargetSpeed;
    }
  }

  // PID controlling
  float error = ((float)(intermediateTargetSpeed  - lastEncoderVal));
  errorSum += error*timeDiffS;

  // Switch between these if no PID is wanted
  //targetPWM = ((float)intermediateTargetSpeed)/127.0f;
  targetPWM = error*PID_P  + ((errorSum)*PID_I);

  int errorSumMax = (1.5f/PID_I);
  if(errorSum > errorSumMax){
    errorSum = errorSumMax;
  }else if(errorSum < 0){
    errorSum = 0;
  }

  if(currentPWM < targetPWM){
    currentPWM += 0.01f;
  }
  if(currentPWM > targetPWM){
    currentPWM -= 0.01f;
  }
  
  if(currentPWM > 1){
    currentPWM = 1.0;
  }
  if(currentPWM < (targetSpeed > 1 ? 0.0:0.0)){
    currentPWM = (targetSpeed > 1 ? 0.0:0.0);
  }
  setMotorPWM(targetPWM);

  encoderValBeforeLast = lastEncoderVal;
  time_last = time_new;

  delay(20);
}

