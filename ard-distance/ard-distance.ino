/**@file ard-distance-spi.ino */
//
// ard-distance-spi.ino: get distance data from Ultrasonic sensor and send to Beaglebone through SPI
//
// Authors:
//      Yifan Ruan(yifanr@kth.se)
//
// License:
//      AFL 3.0 
//
#include <SPI.h>

volatile boolean send_it = false;
long cm1,cm2,cm3;
int trigPin1 = 4;
int echoPin1 = 5;

int trigPin2 = 6;
int echoPin2 = 7;

int trigPin3 = 8;
int echoPin3 = 9;


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

  send_it = false;
  cm1 = readDistance(trigPin1,echoPin1);  
  delay(10);
  cm2 = readDistance(trigPin2,echoPin2);  
  delay(10);
  cm3 = readDistance(trigPin3,echoPin3);
  delay(10);
  send_it = true;

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
  if(cm > 255){
    cm = 255;
    }
  return cm;
}
