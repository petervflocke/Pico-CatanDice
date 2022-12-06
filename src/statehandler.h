/* cSpell:disable */
#pragma once
#ifndef STATUSHANDLER_H
#define STATUSHANDLER_H

#include "keyhandler.h"

typedef enum {
  none=0,                       // 0
  random_waiting_for_press,     
  random_waiting_for_release,   
  random_slow_down,             
  random_display,               
  show_summury,
  show_statistics,              // 5 not used now
  show_statistics_Sum,          
  show_statistics_Single,       
  show_statistics_SumAll,       
  show_statistics_SingleAll,    
  wait_in_statistics,           // 10 not used now
  before_waiting_for_press,     
  screen_saver,
  ibacklight,
  dbacklight,
  thened                        
} state_type;
#define stateNumber (thened-none+1)


extern state_type stateTable[stateNumber][eventNumber];

void initState ();
state_type key_logic(state_type old_state);

#endif