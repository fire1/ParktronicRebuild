
#include <Arduino.h>
#include <SendOnlySoftwareSerial.h>

#ifndef SendOnlySoftwareSerial_h

#include "../libraries/SendOnlySoftwareSerial/SendOnlySoftwareSerial.h"

#endif



#define debugInfo
const uint8_t pinDataTx = A2;
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
const uint16_t debounceDelay = 980;//780;
byte portAddress;
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
    portAddress = B0000;

    switch (portPosition) {
        default:
        case 0:
            bitSet(portAddress, 2);
        case 1:
            bitSet(portAddress, 0);
            bitSet(portAddress, 2);
            break;
        case 2:
            bitSet(portAddress, 1);
            bitSet(portAddress, 2);
            break;
        case 3:
            bitSet(portAddress, 0);
            bitSet(portAddress, 1);
            bitSet(portAddress, 2);
            break;
        case 4:
            bitSet(portAddress, 3);
            break;
        case 5:
            bitSet(portAddress, 0);
            bitSet(portAddress, 3);
            break;
        case 6:
            bitSet(portAddress, 1);
            bitSet(portAddress, 3);
            break;
        case 7:
            bitSet(portAddress, 0);
            bitSet(portAddress, 1);
            bitSet(portAddress, 3);
            break;
    }


    digitalWrite(pinCnlInA, (portAddress >> 0) & 1);
    digitalWrite(pinCnlInB, (portAddress >> 1) & 1);
    digitalWrite(pinOffLft, (portAddress >> 2) & 1);
    digitalWrite(pinOffRgh, (portAddress >> 3) & 1);
    portPosition++;
    if (portPosition > 7) portPosition = 0;
}

//
// Create UART send data
SendOnlySoftwareSerial Data(pinDataTx);

void setup() {
#ifdef debugInfo
    Serial.begin(115200);
    Serial.println("Begin .. ..  ");
#endif
    Data.begin(9600);
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
#ifdef debugInfo
    terminal();
#endif
    echoTravel = 0;
    controlPorts();
    for (index = 0; index < 8; ++index) {
        analogWrite(pinBurstCm, 255);
        analogWrite(pinBurstCm, 0);
        delayMicroseconds(6);
    }

    delayMicroseconds(debounceDelay);
    start = micros();
    while (start + 16500 > now) {
        uint8_t pinState = digitalRead(pinEchoRes);
        now = micros();
        uint16_t gap = now - start;
        if (pinState == LOW && gap > 20) {

            echoTravel = gap + debounceDelay;
            float distance = (float) map(echoTravel, 1000, 4600, 2000, 8200) * 0.01;
#ifdef debugInfo
            Serial.print(" Port: ");
            Serial.print(portAddress, BIN);
            Serial.print(" ");
            Serial.print(echoTravel);
            Serial.print("us ");;
            Serial.print(distance);
            Serial.print("cm ");
            Serial.println();
#endif
            Data.print(portAddress, BIN);
            Data.print(" ");
            Data.print(distance);
            Data.print(" ");

            echoTravel = 0;
            break;
        }
    }
    delay(15);

}
