/* cSpell:disable */
#pragma once
#ifndef STATUSHANDLER_H
#define STATUSHANDLER_H

typedef enum {
  random_waiting_for_press,     // 0
  random_waiting_for_release,   // 1
  random_slow_down,             // 2
  random_display,               // 3
  show_statistics,              // 4
  wait_in_statistics,           // 5
  before_waiting_for_press,     // 6
  none                          // 7
} state_type;

state_type key_logic(state_type old_state);

#endif