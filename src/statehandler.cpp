/* cSpell:disable */
#include "Arduino.h"
#include "statehandler.h"
#include "keyhandler.h"


inline state_type& operator++(state_type& state, int) {
    const int st_ = static_cast<int>(state)+1;
    state = static_cast<state_type>((st_) % stateNumber);
    return state;
}

void initState () {
  for (state_type i=none; i<thened; i++) {
    // zero => none the complete mapping state table none => keep old state
    stateTable[i][e_none]=none;
    stateTable[i][e_right]=none;
    stateTable[i][e_left]=none;
    stateTable[i][e_down]=none;
    stateTable[i][e_up]=none;
  }
  // initiate button and none-button actions for existing machine states
  // current state + button direction => new state
  stateTable[random_waiting_for_press]  [e_down]  =  random_waiting_for_release;
  stateTable[random_waiting_for_release][e_up]    =  random_slow_down;
  stateTable[random_slow_down]          [none]    =  random_display;
  stateTable[random_display]            [e_up]    =  before_waiting_for_press;
  stateTable[random_display]            [e_right] =  show_statistics_Single;
  stateTable[random_display]            [e_left]  =  show_statistics_Sum;
  stateTable[show_statistics]           [none]    =  wait_in_statistics;
  stateTable[show_statistics_Single]    [e_up]    =  before_waiting_for_press;
  stateTable[show_statistics_Sum]       [e_up]    =  before_waiting_for_press;
  stateTable[show_statistics_Single]    [e_right] =  show_statistics_SingleAll;
  stateTable[show_statistics_Single]    [e_left]  =  show_statistics_Sum;
  stateTable[show_statistics_Sum]       [e_left]  =  show_statistics_SumAll;
  stateTable[show_statistics_Sum]       [e_right] =  show_statistics_Single;
  
  stateTable[show_statistics_SingleAll] [e_up]    =  before_waiting_for_press;
  stateTable[show_statistics_SingleAll] [e_right] =  show_statistics_SumAll;
  stateTable[show_statistics_SingleAll] [e_left]  =  show_statistics_Single;

  stateTable[show_statistics_SumAll]    [e_up]    =  before_waiting_for_press;
  stateTable[show_statistics_SumAll]    [e_right] =  show_statistics_Sum;
  stateTable[show_statistics_SumAll]    [e_left]  =  show_statistics_SingleAll;



/*   stateTable[wait_in_statistics]        [e_up]    =  before_waiting_for_press;
  stateTable[wait_in_statistics]        [e_right] =  show_statistics_Single;
  stateTable[wait_in_statistics]        [e_left]  =  show_statistics_Sum; */

  stateTable[before_waiting_for_press]  [none]    =  random_waiting_for_press;
}
  

state_type key_logic(state_type old_state) {
  struct Event e;
  state_type new_state=none;
  
  noInterrupts();
  bool data=buf.pull(&e);
  interrupts();
  
  if (data) { 

    // Serial.print("Pin state     : "); Serial.println(e.pinState, BIN);
    // Serial.print("Pin number    : "); Serial.println(e.pinNum);
    // Serial.print("Direction    : "); Serial.println(e.direction);
    // Serial.print("Timestamp (ms): "); Serial.println(e.timestamp);

    new_state = stateTable[old_state][e.direction];
    // if (new_state == none) new_state = old_state; // none => no action defined, keep state

  }
  return (new_state == none)? old_state:new_state;
}

/* // not more maintained hardcoded state logic / instead of data based logic 
state_type key_logic(state_type old_state) {
  struct Event e;
  state_type new_state=none;
  
  noInterrupts();
  bool data=buf.pull(&e);
  interrupts();
  
  if (data) { 

    // Serial.print("Pin state     : "); Serial.println(e.pinState, BIN);
    // Serial.print("Pin number    : "); Serial.println(e.pinNum);
    // Serial.print("Direction    : "); Serial.println(e.direction);
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
 */ 