/* cSpell:disable */
#include "Arduino.h"
#include "statehandler.h"
#include "keyhandler.h"

state_type key_logic(state_type old_state) {
  struct Event e;
  state_type new_state=none;
  
  noInterrupts();
  bool data=buf.pull(&e);
  interrupts();
  
  if (data) { 

    // Serial.print("Pin state     : "); Serial.println(e.pinState, BIN);
    // Serial.print("Pin number    : "); Serial.println(e.pinNum);
    Serial.print("Direction    : "); Serial.println(e.direction);
    // Serial.print("Timestamp (ms): "); Serial.println(e.timestamp);


    if (old_state == random_waiting_for_press and e.direction == e_down)
      {
        new_state = random_waiting_for_release;
      } 
    else if (old_state == random_waiting_for_release and e.direction == e_up)
      {
        new_state = random_slow_down;
      } 
    else if (old_state == random_display and e.direction == e_up)
      {
        new_state = before_waiting_for_press;
      }
    else if (old_state == random_display and e.direction == e_right)
      {
        new_state = show_statistics_Single;
      }
    else if (old_state == random_display and e.direction == e_left)
      {
        new_state = show_statistics_Sum;
      }
    else if (old_state == wait_in_statistics and e.direction == e_up)
      {
        new_state = before_waiting_for_press;
      }
    else if (old_state == wait_in_statistics and e.direction == e_right)
      {
        new_state = show_statistics_Single;
      }
    else if (old_state == wait_in_statistics and e.direction == e_left)
      {
        new_state = show_statistics_Sum;
      }
    else new_state = old_state;
  }
  return new_state;
}