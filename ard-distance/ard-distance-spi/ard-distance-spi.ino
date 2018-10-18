#include <SPI.h>

volatile char c;
volatile char buf[4];
volatile boolean send_it = false;

int trigPin = 5;
int echoPin = 6;
long duration,cm;

void setup (void)
{

  Serial.begin(9600);

  SPCR |= (1<<SPE)|(1<<SPIE); //SPI control register ,enable spi interrupt and spi
  SPSR |= (1<<SPIF);//SPI status register
  SPI.attachInterrupt();
  pinMode (MISO, OUTPUT);
  // set ditance sensor pins
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{ 
  if(send_it){
    SPDR = cm;
    while(!SPIF){};
  }
}  // end of interrupt routine SPI_STC_vect

// main loop - wait for flag set in interrupt routine
void loop (void)
{
 //trig sensor
  digitalWrite(trigPin,LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  
  //get the distance data
  duration = pulseIn(echoPin,HIGH);
  send_it = false;
  cm = (duration/2)/39.1;
  if(cm>400){
    cm = 0;  
  }
  send_it = true;
  Serial.println(cm);
  delay(100);
}  // end of loop
