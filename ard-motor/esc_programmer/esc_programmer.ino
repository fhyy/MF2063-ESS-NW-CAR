/**@file esc_programmer.ino */
#include <Servo.h>

/** \def ESC_PIN
	The pin number of which the ESC data pin should be connected to.
*/

#define ESC_PIN 10

Servo esc;

/**
 * How to set up the ESC:
 * 
 * Connect pins, but don't start ESC	\n
 * Run this code	\n
 * Send signal 2000	\n
 * Hold "set" button on ESC and switch it on	\n
 * Wait for orange solid light	\n
 * Release "set" button and wait for solid red	\n
 * Send signal 700	\n
 * Wait for solid orange	\n
 * Send signal 1000 'neutral'	\n
 * Wait for beep and no light	\n
 * Switch off ESC	\n
 * Turn on ESC, ready to use!
 */

/**
	This function sets up the serial communication, used with e.g. the Arduino Studio Serial monitor
*/
void setup() {
  // put your setup code here, to run once:
  esc.attach(ESC_PIN);
  Serial.begin(9600);
}

/**
	Loops for ever until the Arduino is turned off. Waits for input on the serial communication channel and writes this to the ESC
*/
void loop() {
  if(Serial.available()){
    int value = Serial.parseInt();
    Serial.print("Sending value: ");
    Serial.println(value);
    esc.writeMicroseconds(value);
  }
}
