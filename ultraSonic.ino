
#include <Arduino.h>


const uint8_t pinCnlInA = 10; // control port A
const uint8_t pinCnlInB = 11; // control port B
const uint8_t pinOffLft = 12; // disable left
const uint8_t pinOffRgh = A0; // disable right
const uint8_t pinCarRev = A1; // car reverse
const uint8_t pinInTes7 = 6; // unknown in
const uint8_t pinInTes8 = 7; // unknown in
const uint8_t pinEchoRes = 2; // Echo response
const uint8_t pinBurstCm = 3; // common burst pin

volatile uint8_t index;
volatile unsigned long echoTravel = 0;
volatile unsigned long now = 0, start = 0;
const uint16_t debounceDelay = 780;//780;

void terminal() {
    if (Serial.available()) {
        String where = Serial.readStringUntil('=');
        if (where == "p7") {
            uint8_t val = Serial.readStringUntil('\n').toInt();
            digitalWrite(pinInTes7, val);
            Serial.print(F("pin 7  "));
            Serial.println(val);
        } else if (where == "p8") {
            uint8_t val = Serial.readStringUntil('\n').toInt();
            digitalWrite(pinInTes8, val);
            Serial.print(F("pin 8 "));
            Serial.println(val);
        }

    }
}


void setup() {
    Serial.begin(115200);
    Serial.println("Begin .. ..  ");
    pinMode(pinBurstCm, OUTPUT);
    pinMode(pinCnlInA, OUTPUT);
    pinMode(pinCnlInB, OUTPUT);
    pinMode(pinOffLft, OUTPUT);
    pinMode(pinOffRgh, OUTPUT);
    pinMode(pinCarRev, INPUT_PULLUP);
    pinMode(pinInTes7, INPUT);
    pinMode(pinInTes8, INPUT);
    pinMode(pinEchoRes, INPUT);

//    attachInterrupt((uint8_t) digitalPinToInterrupt(pinEchoRes), captureIsr, CHANGE);
    digitalWrite(pinEchoRes, LOW);
}



void loop() {

    terminal();
    echoTravel = 0;
    digitalWrite(pinCnlInA, LOW);
    digitalWrite(pinCnlInB, LOW);
    digitalWrite(pinOffLft, LOW);
    digitalWrite(pinOffRgh, LOW);
    for (index = 0; index < 8; ++index) {
        analogWrite(pinBurstCm, 255);
        analogWrite(pinBurstCm, 0);
        delayMicroseconds(6);
    }
    delayMicroseconds(debounceDelay);

    start = micros();
    while (start + 10000 > now) {
        uint8_t pinState = digitalRead(pinEchoRes);
        now = micros();
        uint16_t gap = now - start;
      if(pinState == LOW ){
          Serial.print(" Result: ");
          echoTravel = gap + debounceDelay;
          Serial.print(echoTravel);
          Serial.print("us ");
          Serial.print((float) map(echoTravel, 1000 , 4600, 2000, 8200) *0.01 );
          Serial.print("cm ");
          echoTravel = 0;
          break;
      }
    }
    Serial.println();
    delay(10);

}
