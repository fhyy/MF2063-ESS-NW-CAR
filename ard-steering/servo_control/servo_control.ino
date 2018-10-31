#include <Servo.h>

#define FULL_LEFT_PWM 20
#define CENTER_PWM 60
#define FULL_RIGHT_PWM 90
#define PLAY_AREA_SIZE 40

Servo myservo;

int lastSignal = 0;

void setSteeringPWM(float pwm){
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
 
  Serial.print(servo_signal + (direction?PLAY_AREA_SIZE:0));
  Serial.print("\n");
}

void setup() {
  Serial.begin(9600);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pinMode(LED_BUILTIN, OUTPUT);
}

float pwm = 0.0f;
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  while (pwm < 1.0f) { // goes from 0 degrees to 180 degrees
    setSteeringPWM(pwm);              // tell servo to go to position in variable 'pos'
	  pwm += 0.5f;
    delay(1000);                       // waits 15ms for the servo to reach the position
  }
  digitalWrite(LED_BUILTIN, LOW);
  while (pwm > 0.0f) { // goes from 180 degrees to 0 degrees
    setSteeringPWM(pwm);              // tell servo to go to position in variable 'pos'
	  pwm -= 0.5f;
    delay(1000);                       // waits 15ms for the servo to reach the position
  }
}

