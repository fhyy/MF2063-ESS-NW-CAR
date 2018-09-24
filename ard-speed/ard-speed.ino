//Global variables
int irPin = 14;     //IR-sensor connected to pin 78
int irVal = 1;      //Int to store IR-value

void setup() {
    //Init serial communication to enable prints
    Serial.begin(115200);

    //Try printing
    Serial.println("Serial communication set up!");

    //Setup in/out pins
    pinMode(irPin, INPUT);

}

void loop() {
    // put your main code here, to run repeatedly:
    irVal = digitalRead(irPin);
    Serial.println(irVal);

} 
