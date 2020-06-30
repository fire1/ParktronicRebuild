
#include <Arduino.h>

#define PRINT_MICROS
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
byte portContainer;
int8_t portPosition;


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

/*

E S2 S1
L L L nY0 to nZ
L L H nY1 to nZ
L H L nY2 to nZ
L H H nY3 to nZ
H X X switches off
 */

void controlPorts() {
    portContainer = B0000;

    switch (portPosition) {
        default:
        case 0:
            bitSet(portContainer, 2);
        case 1:
            bitSet(portContainer, 0);
            bitSet(portContainer, 2);
            break;
        case 2:
            bitSet(portContainer, 1);
            bitSet(portContainer, 2);
            break;
        case 3:
            bitSet(portContainer, 0);
            bitSet(portContainer, 1);
            bitSet(portContainer, 2);
            break;
        case 4:
            bitSet(portContainer, 3);
            break;
        case 5:
            bitSet(portContainer, 0);
            bitSet(portContainer, 3);
            break;
        case 6:
            bitSet(portContainer, 1);
            bitSet(portContainer, 3);
            break;
        case 7:
            bitSet(portContainer, 0);
            bitSet(portContainer, 1);
            bitSet(portContainer, 3);
            break;
    }


    digitalWrite(pinCnlInA, (portContainer >> 0) & 1);
    digitalWrite(pinCnlInB, (portContainer >> 1) & 1);
    digitalWrite(pinOffLft, (portContainer >> 2) & 1);
    digitalWrite(pinOffRgh, (portContainer >> 3) & 1);
    portPosition++;
    if (portPosition > 7) portPosition = 0;
}


void setup() {
    Serial.begin(115200);
#ifdef PRINT_MICROS
    Serial.println("Begin .. ..  ");
#endif
    pinMode(pinBurstCm, OUTPUT);
    pinMode(pinCnlInA, OUTPUT);
    pinMode(pinCnlInB, OUTPUT);
    pinMode(pinOffLft, OUTPUT);
    pinMode(pinOffRgh, OUTPUT);
    pinMode(pinCarRev, INPUT_PULLUP);
    pinMode(pinInTes7, INPUT);
    pinMode(pinInTes8, INPUT);
    pinMode(pinEchoRes, INPUT);
    digitalWrite(pinEchoRes, LOW);
}


void loop() {

    terminal();
    echoTravel = 0;
    controlPorts();
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
        if (pinState == LOW) {

            echoTravel = gap + debounceDelay;
#ifdef PRINT_MICROS
            Serial.print(echoTravel);
            Serial.print("us ");
#endif
            Serial.print(portContainer, BIN);
            Serial.print(" ");
            Serial.print((float) map(echoTravel, 1000, 4600, 2000, 8200) * 0.01);

            echoTravel = 0;
            break;
        }
    }
    Serial.println();
    delay(10);

}
