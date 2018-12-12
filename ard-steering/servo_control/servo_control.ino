#include <Servo.h>
#include <SPI.h>

/** \def SERVO_PIN
  The pin number of which the steering servo data pin should be connected to.
*/
/** \def FULL_LEFT_PWM
 *  The PWM signal for turning the front wheels at a specified angle to the left.
 */
 /** \def CENTER_PWM
 *  The PWM signal for centering the front wheels.
 */
 /** \def FULL_RIGHT_PWM
 *  The PWM signal for turning the front wheels at a specified angle to the right.
 */
/** \def PLAY_AREA_SIZE
 *  There is a distance the servo has to turn before it has any effect when switching directions.
 *  This variable defines this distance in terms of PWM offset
 */

#define SERVO_PIN 9

#define FULL_LEFT_PWM 20
#define CENTER_PWM 60
#define FULL_RIGHT_PWM 90
#define PLAY_AREA_SIZE 40

Servo myservo;  /**< Used to send PWM signals to the steering servo*/

char lastSignal = 50; /**< Store the last signal sent to the servo to check for direction changes */

/**
 * \brief A function for setting the servo PWM from a percentage value.
 * 
 * This function is used to convert a percentage into a PWM signal in the given range 
 * from \ref FULL_LEFT_PWM "FULL_LEFT_PWM" to \ref FULL_RIGHT_PWM "FULL_RIGHT_PWM" 
 * given the offset of \ref PLAY_AREA_SIZE "PLAY_AREA_SIZE".
 */
void setSteeringPWM(float pwm)
{
  Serial.println(pwm);
  if(pwm < 0){
    pwm = 0.0f;
  }
  if(pwm > 1){
    pwm = 1.0f;
  }
	int servo_signal = 0;
	if(pwm < 0.5f){
		servo_signal = FULL_LEFT_PWM+(int)((float)((CENTER_PWM-FULL_LEFT_PWM)<<1)*pwm);
	}else{
		servo_signal = CENTER_PWM+(int)((float)((FULL_RIGHT_PWM-CENTER_PWM)<<1)*(pwm-0.5f));
	}
  char direction = 0;
  if(servo_signal > lastSignal){
    direction = 1;
  }
  // PLAY_AREA_SIZE is needed as the servo motor rotation won't affect the wheel angles
  // until the pwm signal differs by PLAY_AREA_SIZE, when the servo rotation direction has changed.
	myservo.write(servo_signal + (direction?PLAY_AREA_SIZE:0));
}

/**
 * \brief A function for setting up the SPI communication using interrupts.
 */
void startSPI()
{
  SPCR |= (1 << SPE) | (1 << SPIE); //SPI control register ,enable spi interrupt and spi
  SPSR |= (1 << SPIF); //SPI status register
  SPI.attachInterrupt();
  pinMode(MISO, OUTPUT);
}

/**
 * \brief An interrupt handler for SPI communication.
 */
ISR(SPI_STC_vect)
{ 
  digitalWrite(LED_BUILTIN, HIGH);
  byte spi_in = SPDR;
  
  if(spi_in < 0){
    spi_in = 0;
  }
  if(spi_in > 100){
    spi_in = 100;
  }

  setSteeringPWM(((float)spi_in)/100.0f);
  lastSignal = spi_in;
  digitalWrite(LED_BUILTIN, LOW);
}

/**
  * \brief Ardunino setup function. Runs once when the board is started.
  * 
  * Sets up the pin functions, attaches servo controllers, and calls \ref startSPI "startSPI()".
  */
void setup() 
{
  Serial.begin(9600);
  startSPI();
  myservo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object
  pinMode(LED_BUILTIN, OUTPUT);
}

/**
 * \brief Arduino loop function. Runs over and over, until the Arduino is turned off. Starts after the \ref setup "setup()" has finished.
 * 
 * Continuously sets the servo signal to the latest target signal received in the interrupt handler \ref ISR "ISR(SPI_STC_vect)"
 */
void loop() 
{
  //setSteeringPWM(((float)lastSignal)/100.0f);
  delay(500);
}

