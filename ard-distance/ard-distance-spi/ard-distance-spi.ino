#include <SPI.h>

volatile boolean send_it = false;
long cm1,cm2,cm3;
int trigPin1 = 7;
int echoPin1 = 8;

int trigPin2 = 3;
int echoPin2 = 4;

int trigPin3 = 5;
int echoPin3 = 6;


void setup (void)
{

  Serial.begin(9600);
  SPCR |= (1<<SPE)|(1<<SPIE); //SPI control register ,enable spi interrupt and spi
  SPSR |= (1<<SPIF);//SPI status register
  SPI.attachInterrupt();
  pinMode (MISO, OUTPUT);
  // set ditance sensor pins
  pinMode(trigPin1,OUTPUT);
  pinMode(echoPin1,INPUT);
  pinMode(trigPin2,OUTPUT);
  pinMode(echoPin2,INPUT);
  pinMode(trigPin3,OUTPUT);
  pinMode(echoPin3,INPUT);
}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{ 
  SPI.transfer(cm1);
  SPI.transfer(cm2);
  SPI.transfer(cm3);
}  // end of interrupt routine SPI_STC_vect


// main loop - wait for flag set in interrupt routine
void loop (void)
{


  cm1 = readDistance(trigPin1,echoPin1);  
  cm2 = readDistance(trigPin2,echoPin2);  
  cm3 = readDistance(trigPin3,echoPin3);

  Serial.println("Data display:");
  Serial.println(cm1);
  Serial.println(cm2);
  Serial.println(cm3);
  Serial.println();
  delay(100);
}  // end of loop

long readDistance(int trigPin, int echoPin){
  long duration,cm;
  //trig sensor
  digitalWrite(trigPin,LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  
  //get the distance data
  duration = pulseIn(echoPin,HIGH);
  cm = (duration/2)/39.1;
  return cm;
}