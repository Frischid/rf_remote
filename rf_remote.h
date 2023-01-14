#include <avr/sleep.h>

#define HIGHEST_SEQUENCE 6

// to use with all 4 devices
#define ADVANCED_PAD

// left has external pull down
#define RIGHT_PULL_DOWN  1

#define RIGHT_OUT

#include "remote.h"

void setupInput();

// intertechno 11011011110100111011000101
// aus / an
// alle, 1, 2 ,3
// output signal ca. f=4.5kHz
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
uint8_t* outerPins[] = {&left, &right};

void setup() {
  Remote::setPin(rfPin);
  setupInput();

  Remote::blinkPin();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  for (size_t a = 0; a < sizeof(midPins) / sizeof(midPins[0]); a++)
    attachInterrupt(digitalPinToInterrupt(*midPins[a]), loop, CHANGE);
  attachInterrupt(digitalPinToInterrupt(right), loop, CHANGE);

  sleep_cpu();
}

void loop() {
  noInterrupts();


  // all of not yet implemented! - conflicting UDPI
  // put pull down to udpi pin OR configure it as output for full functionality
  for (size_t a = 0; a < sizeof(midPins) / sizeof(midPins[0]); a++) {
    uint8_t sequence = 0;
    if (!digitalRead(*midPins[a])) { // find pressed row
      digitalWrite(left, 1);
      if (digitalRead(*midPins[a])) { // pressed left
        sequence = 2 * a + 1;
      } else { // pressed right
        sequence = 2 * a ;

#ifdef RIGHT_PULL_DOWN
        if (*midPins[a] == 2) { // check if all off
          pinMode(right, INPUT_PULLUP);
          if (digitalRead(*midPins[a])) {
            sequence = HIGHEST_SEQUENCE; // all off pressed
          }
          pinMode(right, INPUT);
        }
#else
        sequence = HIGHEST_SEQUENCE;
#endif

      }
      send_sequence(sequence);
      digitalWrite(left, 0);
    }
  }

  interrupts();
  sleep_cpu(); 
  return; 
}

void setupInput() {
  // right turn off
  pinMode(left, OUTPUT);
  digitalWrite(left, 0);

#ifdef RIGHT_PULL_DOWN
  // right: external pull down
  pinMode(right, INPUT);
#elif RIGHT_OUT
  pinMode(right, OUTPUT);
  digitalWrite(right, 0);
#else
  pinMode(right, OUTPUT);
  digitalWrite(right, 0);
#endif

  for (size_t a = 0; a < sizeof(midPins) / sizeof(midPins[0]); a++) {
    pinMode(*midPins[a], INPUT_PULLUP);
  }
}

/*
  - (use TCB saves power) - but know nothing about TCD
  - reduce clock reduces power

  - low: 0.3*VDD
  - high: 0.7*VDD
  - 20(min)/35(typ)/50(max) kOhm pullup

*/
