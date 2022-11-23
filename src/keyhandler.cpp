/* cSpell:disable */
#include "Arduino.h"
#include "keyhandler.h"

unsigned long stime = to_ms_since_boot(get_absolute_time());
const int delayTime = 20; // Delay for every push button may vary

void gpio_callback(uint gpio, uint32_t events) {
static volatile int last_delta = 0;
static volatile int last_seq = 0;
static volatile int last_remainder = 0;
static volatile int last_cycles = 0;
static volatile byte xstate = LOW;

  struct Event e;
  if (gpio == pinBut) {
    // debouncing press button, actually done on hardware layer
    // don't like to debounced edges
    if ((to_ms_since_boot(get_absolute_time())-stime)>delayTime) {
        stime = to_ms_since_boot(get_absolute_time()); 
        
        // Interrupt function lines for press button
        // change built in LED status, jut for fun
        xstate = !xstate;
        gpio_put(LED_PIN, xstate);
        
        // store the edge of the button signal press vs. release
        e.direction = events & GPIO_IRQ_EDGE_FALL? e_down: events & GPIO_IRQ_EDGE_RISE ? e_up: e_none;
        e.pinNum = gpio;
        e.timestamp = millis();
        buf.add(e);  // Add it to the ring buffer
    }
  } else if (gpio == pinA || gpio == pinB ) {

        #define steps_per_cycle 4
        int delta = 0;
        uint a_state = gpio_get(pinA);
        uint b_state = gpio_get(pinB);
        int current_seq = (a_state ^ b_state) | b_state << 1;
        if (current_seq != last_seq) {
          delta = (current_seq - last_seq) % 4;
          if (delta == 3) {
              delta = -1;
          }
          else if (delta == 2) {
              delta = copysign(delta, last_delta);
          }
          last_delta = delta;
          last_seq = current_seq;
          last_remainder += delta;
          last_cycles = last_remainder;
          last_remainder %= steps_per_cycle;
          // e.pinState = 0;
          // e.index = xindex++;
          e.pinNum = 0;
          e.timestamp = millis();
          if ( last_cycles == 1 ) {
              e.direction = e_right;
              buf.add(e);
          }
          else if (last_cycles == -1) {
            e.direction = e_left;
            buf.add(e);
          }
          else {
            ;          
          }
        }  
  }
}

// cleanup all key(board) presses
void key_clean() {
  struct Event e;
  noInterrupts();
  while (!buf.isEmpty()) {
    buf.pull(&e);
  }
  interrupts();
}

// add a dummy key press to enable none-key actions (logical state change)
void key_none() {
  struct Event e;
  e.direction = e_none;
  noInterrupts();
  buf.add(e);
  interrupts();
}
bool checkBreak() {
  struct Event e;
  if (!buf.isEmpty()) {
    noInterrupts();
    bool data=buf.pull(&e);
    interrupts();
    return e.direction == e_up;
  } else return false;
}