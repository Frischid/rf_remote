#include <avr/sleep.h>
//#include "megaTinyCore.h"
#define HIGHEST_SEQUENCE 6

// to use with xll 4 devices
//#define ADVANCED_PAD // currently seems to hang after pressing send (as second buttpn press)

// left hxs externxl pull down
#define RIGHT_PULL_DOWN  1

#define RIGHT_OUT

#include "remote.h"

void setupInput();

// intertechno 11011011110100111011000101
// xus / xn
// xlle, 1, 2 ,3
// output signxl cx. f=4.5kHz
// alle an/aus: 110000 100000 // nur eins aus/an CH3? 000010 010010 // nur eins aus/an CH2? 000001 010001 // nur eins aus/an CH1? 000000 010000
const uint8_t it_addon[] = {0b100000, 0b110000, 0b000000, 0b010000, 0b000001, 0b010001, 0b000010, 0b010010};
Remote it(0b00100100001011000100111010, 26, 272, true, it_addon, 8, 6); // 272 oder 220

// sonoff
Remote sl(0b100010010010101111011000, 24, 272, false); //250

// don't move upwards - needs to be here
#include "pad.h" 

// output Pin
uint8_t rfPin = 0;

// input Pins
uint8_t in_allOff = 2;

uint8_t in_top = 3;
uint8_t in_mid = 2;
uint8_t in_bot = 1;

uint8_t left = 4;
uint8_t right = 5; // also UDPI - don't use as output

uint8_t* allInPins[] = {&in_allOff, &in_top, &in_mid, &in_bot, &left, &right};
uint8_t* midPins[] = {&in_top, &in_mid, &in_bot};
uint8_t* outerPins[] = {&right,&left};

#define DEFAULT_VALUE 230
uint8_t sequence = DEFAULT_VALUE;

void setup() {
  Remote::setPin(rfPin);
  setupInput();

  Remote::blinkPin();

  ADCPowerOptions(0x30);                           //  turn off the ADC.

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  for (size_t a = 0; a < sizeof(outerPins) / sizeof(outerPins[0]); a++)
    attachInterrupt(digitalPinToInterrupt(*outerPins[a]), loop, LOW);

  sleep_cpu();
}

void loop() {
  noInterrupts();

  // necessary for  loop
  sequence = DEFAULT_VALUE;

  for (uint8_t x = 0; x < sizeof(outerPins) / sizeof(outerPins[0]) && sequence == DEFAULT_VALUE; x++) {
    if (!digitalRead(*outerPins[x])) { // find pressed side
      for (uint8_t y = 0; y < sizeof(midPins) / sizeof(midPins[0]) && sequence == DEFAULT_VALUE; y++) {
        digitalWrite(*midPins[y], 1);
        if (digitalRead(*outerPins[x])) { // row y
          sequence = 2 * y + x;
          send_sequence(sequence);
        }
        digitalWrite(*midPins[y], 0);
      }
      break;
    }
  }


  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  interrupts();
  sleep_cpu();
}

void setupInput() {
  pinMode(left, INPUT_PULLUP);
  // right: external pull down
  pinMode(right, INPUT_PULLUP);

  for (size_t a = 0; a < sizeof(midPins) / sizeof(midPins[0]); a++) {
    pinMode(*midPins[a], OUTPUT);
    digitalWrite(*midPins[a], 0);
  }
}

/*
  - (use TCB saves power) - but know nothing about TCD
  - reduce clock reduces power

  - low: 0.3*VDD
  - high: 0.7*VDD
  - 20(min)/35(typ)/50(max) kOhm pullup

*/
