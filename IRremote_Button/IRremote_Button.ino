#include <Arduino.h>
#include <IRremote.hpp>
/* Defines of GPIO_Interrupt.ino */
const byte ledPin = 2;       // Builtin-LED pin
const byte interruptPin = 0; // BOOT/IO0 button pin
volatile byte state = LOW;

uint16_t sAddress = 0x0102;
uint8_t sCommand = 0x34;
uint8_t sRepeats = 0;

/* Defines of SendAndReceive.ino */
// select only NEC and the universal decoder for pulse width or pulse distance protocols
#define DECODE_NEC          // Includes Apple and Onkyo
#define DECODE_DISTANCE     // in case NEC is not received correctly

#include "PinDefinitionsAndMore.h"

 
void setup() {
  Serial.begin(9600);
  IrSender.begin(4);
  IrReceiver.begin(15, ENABLE_LED_FEEDBACK);
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), IRsender, CHANGE);
}

void loop() {
  Serial.println("IR receiving...");
  if (IrReceiver.decode()) {
        
        Serial.println("12322222222222222222222222222222222");
        Serial.println("12322222222222222222222222222222222");
        Serial.println("12322222222222222222222222222222222");
        IrReceiver.resume();
    }
    delay(100);
}

void IRsender() {
  Serial.println("IR sending...");
  IrSender.sendNEC(sAddress, sCommand, sRepeats);
}
