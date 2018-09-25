

#define MOTOR_PIN 12 //digital pin 9

void setMotorPWM(float dutyCycle){
  OCR1B = (int)(((float)OCR1A) * dutyCycle);
}

void setupMotorPWM(){
  //Set PWM pin
  pinMode(MOTOR_PIN, OUTPUT);
  
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  //Set up Fast PWM mode 15 on Timer 1
  TCCR1A |= (1 << WGM10); //1
  TCCR1A |= (1 << WGM11);  //1
  TCCR1B |= (1 << WGM12);  //1
  TCCR1B |= (1 << WGM13);  //1
  
  //Set compare output mode
  //TCCR1A &= ~(1 << COM1B0);
  TCCR1A |= (1 << COM1B1);
  
  //Set OCR1A to 319 := TOP or Period 50Hz
  OCR1A = 319;
  
  //Set OCR1B := Duty cycle
  OCR1B = 100; //~30%
  
  //Set clock source with no prescaling (1)
  TCCR1B |= (1 << CS10);  //1
  //TCCR1B &= ~(1 << CS11);  //0
  //TCCR1B &= ~(1 << CS12); //0
  
  //Disable PRTIM0 in PRR0
  //PRR0 &= ~(1 << PRTIM1); //0
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);
  setupMotorPWM();
}

float cycle = 0.0;
// the loop function runs over and over again forever
void loop() {
  cycle+=0.01;
  if(cycle >= 2.0)
    cycle = 0.0;
  if(cycle >= 1.0)
    setMotorPWM(2.0 - cycle);
  else
    setMotorPWM(cycle);
  delay(100);                       // wait for a second
}
