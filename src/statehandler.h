/* cSpell:disable */
#pragma once
#ifndef STATUSHANDLER_H
#define STATUSHANDLER_H

#include "keyhandler.h"

typedef enum {
  none=0,                       // 0
  random_waiting_for_press,     // 1
  random_waiting_for_release,   // 2
  random_slow_down,             // 3
  random_break,                 // 4 
  random_display,               // 4
  show_statistics,              // 5 not used now
  show_statistics_Sum,          // 6
  show_statistics_Single,       // 7
  show_statistics_SumAll,       // 8
  show_statistics_SingleAll,    // 9
  wait_in_statistics,           // 10 not used now
  before_waiting_for_press,     // 11
  thened                        // 12
} state_type;
#define stateNumber (thened-none+1)


extern state_type stateTable[stateNumber][eventNumber];

void initState ();
state_type key_logic(state_type old_state);

#endif