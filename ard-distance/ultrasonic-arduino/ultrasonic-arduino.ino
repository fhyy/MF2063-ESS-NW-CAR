#include <SoftwareSerial.h>

SoftwareSerial mySerial(8,7); //Rx,Tx
int trigPin = 11;
int echoPin = 12;
long duration,cm;

void setup() {
  // put your setup code here, to run once:
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  pinMode(13,OUTPUT);
  mySerial.begin(9600);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  String str;
  digitalWrite(trigPin,LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);

  pinMode(echoPin,INPUT);
  duration = pulseIn(ecnahoPin,HIGH);
  cm = (duration / 2) / 29.1;
  //str = String(cm);
  mySerial.write(cm);
  mySerial.write("cm\n");
  Serial.print(cm);
  Serial.print("cm\n");  
  delay(1000);
}
