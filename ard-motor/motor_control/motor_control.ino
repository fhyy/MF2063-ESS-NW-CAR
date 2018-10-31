#include <Servo.h>

#define MOTOR_PIN 10 //digital pin 12 Mega, digital pin 10 Micro
#define ESC_MAX 2000
#define ESC_MIN 1000
#define ESC_ARM 500
#define ESC_CALIBRATION_DELAY 5000

Servo esc;

/*
 * dutyCycle of 50% => standing still
 * dutyCycle of <50% => reversing
 * dutyCycle of >50% => driving forward
 */
void setMotorPWM(float dutyCycle)
{
  Serial.println(dutyCycle);
  Serial.println(ESC_MIN+(ESC_MAX-ESC_MIN)*dutyCycle);
  esc.writeMicroseconds(ESC_MIN+(ESC_MAX-ESC_MIN)*dutyCycle);
}

void setupAndCallibrateESC()
{
    esc.attach(MOTOR_PIN);  // Attach servo to pin
    
    esc.writeMicroseconds(ESC_MAX); // Send calibration signal to ESC (also max value)
    
    // Start ESC during this time
    delay(ESC_CALIBRATION_DELAY);   // Wait for ESC to acknowledge calibration signal
    
    esc.writeMicroseconds(ESC_MIN);  // Send minimum signal value for calibration
    
    delay(ESC_CALIBRATION_DELAY); // Wait for ESC to acknowledge signal
    
    esc.writeMicroseconds(ESC_ARM); // Send arm signal to ESC
}

// the setup function runs once when you press reset or power the board
void setup() 
{
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(LED_BUILTIN, HIGH); // LED is on when setting up motor
  setupAndCallibrateESC();
  digitalWrite(LED_BUILTIN, LOW); // LED is off when setup is complete
  
  Serial.begin(9600);
}

float cycleMax = 0.3;
float cycleMin = -.14;

float cycle = cycleMin;
char currentDirection = 0;
char directionChanged = 0;

// the loop function runs over and over again forever
void loop()
{
  /*
   * TODO: Instead read PWM signal from motor controller (SPI) and send it to the motor with:
   *            setMotorPWM( 0.0f <= signal <= 1.0f )
   *            (Interrupt handling?)
   */
  if(currentDirection == 0){
    cycle+=0.02;
    if(cycle >= cycleMax){
      currentDirection = 1;
      directionChanged = 1;
    }
  }
  else if(currentDirection == 1){
    cycle-=0.01;
    if(cycle <= cycleMin){
      currentDirection = 0;
      directionChanged = 1;
    }
  }
  if(directionChanged == 1 && (cycle <= 0.00001 && cycle >= -0.00001)){
    directionChanged = 0;
    delay(2000);
  }
  setMotorPWM(0.5+cycle/2);
  delay(100);                       // wait for a tenth of a second
}
