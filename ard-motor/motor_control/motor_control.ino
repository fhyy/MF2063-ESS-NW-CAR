#include <Servo.h>

#define MOTOR_PIN 10 //digital pin 12 Mega, digital pin 10 Micro
#define ESC_MAX 2000
#define ESC_MAX_CAP 1500
#define ESC_MIN 1000
#define ESC_ARM 500
#define ESC_CALIBRATION_DELAY 5000

Servo esc;

/*
 * dutyCycle of 0% => standing still
 * dutyCycle of 0% < x < 100% => driving forward
 * dutyCycle of 100% => driving forward full speed
 */
void setMotorPWM(float dutyCycle)
{ 
  if(dutyCycle < 0)
    dutyCycle = 0.0f;
  if(dutyCycle > 1){
    dutyCycle = 1.0f; 
  }

  Serial.println(dutyCycle);
  Serial.println(ESC_MIN+(ESC_MAX-ESC_MIN)*dutyCycle);
  
  int pwm = (ESC_MIN+(ESC_MAX-ESC_MIN)*dutyCycle);
  if(pwm > ESC_MAX_CAP){
    pwm = ESC_MAX_CAP;
  }
  esc.writeMicroseconds(pwm);
  Serial.println(pwm);
}

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
  
  Serial.begin(9600);
}

float cycleMax = 1.0f;
float cycleMin = 0.0f;

float cycle = cycleMin;
char currentDirection = 1;

// the loop function runs over and over again forever
void loop()
{
  /*
   * TODO: Instead read PWM signal from motor controller (SPI) and send it to the motor with:
   *            setMotorPWM( 0.0f <= signal <= 1.0f )
   *            (Interrupt handling?)
   *            
   * TODO: Motor won't run until it has gone down to 0% PWM first.. don't know why..
   *       something with the gearbox
   */
  
  if(currentDirection){
    if(cycle < cycleMax){
    	cycle += 0.02f;
    }else{
    	currentDirection = 0;
    }
  }else{
	  if(cycle > cycleMin){
      cycle -= 0.02f;
    }else{
      currentDirection = 1;
    }
  }

  if(setupComplete){
    setMotorPWM(cycle);
  }
  delay(100);                       // wait for a tenth of a second
}
