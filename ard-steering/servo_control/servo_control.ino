#include <Servo.h>
#include <SPI.h>

#define FULL_LEFT_PWM 20
#define CENTER_PWM 60
#define FULL_RIGHT_PWM 90
#define PLAY_AREA_SIZE 40

Servo myservo;

char lastSignal = 50;

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
  lastSignal = servo_signal;
  // PLAY_AREA_SIZE is needed as the servo motor rotation won't affect the wheel angles
  // until the pwm signal differs by PLAY_AREA_SIZE, when the servo rotation direction has changed.
	myservo.write(servo_signal + (direction?PLAY_AREA_SIZE:0));
}

// Initialize spi
void startSPI()
{
  SPCR |= (1 << SPE) | (1 << SPIE); //SPI control register ,enable spi interrupt and spi
  SPSR |= (1 << SPIF); //SPI status register
  SPI.attachInterrupt();
  pinMode(MISO, OUTPUT);
}

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

  while (!SPIF){}; // Wait for the end of the transmission
  digitalWrite(LED_BUILTIN, LOW);

//  Serial.print("SPI in: ");
//  Serial.println(spi_in);
}

void setup() 
{
  Serial.begin(9600);
  startSPI();
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() 
{
  //setSteeringPWM(((float)lastSignal)/100.0f);
  delay(1000);
}

