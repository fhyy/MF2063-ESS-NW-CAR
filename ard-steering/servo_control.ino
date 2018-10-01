#include <Servo.h>

Servo myservo;

void setup() {
  myservo.attach(8);  // attaches the servo on pin 9 to the servo object
  pinMode(LED_BUILTIN, OUTPUT);
}

int pos = 0;

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  for (pos = 40; pos <= 140; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
  digitalWrite(LED_BUILTIN, LOW);
  for (pos = 140; pos >= 40; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
}

