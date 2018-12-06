#include <Servo.h>

Servo esc;

/*
 * How to set up the ESC:
 * 
 * Connect pins, but don't start ESC
 * Run this code
 * Send signal 2000
 * Hold "set" button on ESC and switch it on
 * Wait for orange solid light
 * Release "set" button and wait for solid red
 * Send signal 700
 * Wait for solid orange
 * Send signal 1000 'neutral'
 * Wait for beep and no light
 * Switch off ESC
 * Turn on ESC, ready to use!
 */

void setup() {
  // put your setup code here, to run once:
  esc.attach(10);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    int value = Serial.parseInt();
    Serial.print("Sending value: ");
    Serial.println(value);
    esc.writeMicroseconds(value);
  }
}
