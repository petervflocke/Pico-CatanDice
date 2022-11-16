/* cSpell:disable */
#pragma once
#ifndef KEYHANDLER_H
#define KEYHANDLER_H

#include "Arduino.h"
#include <RingBufCPP.h>

// Input / output pins
#define LED_PIN LED_BUILTIN

// Rotary switch
#define pinBut 7
#define pinA   8
#define pinB   9

typedef enum {
  e_none,
  e_right,
  e_left,
  e_down,
  e_up
} coder_type;

struct Event
{
  // uint32_t pinState;
  uint pinNum;
  coder_type direction;
  unsigned long timestamp;
};

#define MAX_NUM_ELEMENTS 10

extern RingBufCPP<struct Event, MAX_NUM_ELEMENTS> buf;

void gpio_callback(uint gpio, uint32_t events);
void key_clean();

#endif