/* cSpell:disable */
#include "Arduino.h"
#include "keyhandler.h"

unsigned long stime = to_ms_since_boot(get_absolute_time());
const int delayTime = 20; // Delay for every push button may vary

void gpio_callback(uint gpio, uint32_t events) {
  static volatile int enc_result = 0;
  static volatile int enc_delta  = 0;
  // https://hifiduino.wordpress.com/2010/10/20/rotaryencoder-hw-sw-no-debounce/
  static const int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // pin states table
  static uint8_t old_AB = 0;
  int val;
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
        // Debuncing on hardware level
        old_AB <<= 2;                                                   //remember previous state
        old_AB |= ( ((gpio_get(pinB) << 1) | gpio_get(pinA)) & 0x03 );  //add current state
        enc_result = ( enc_states[( old_AB & 0x0f )] );                 // use table to find the direction

        // https://www.mikrocontroller.net/articles/Drehgeber => count number of 1 step encoder
        if( enc_result & 1 ) {                                          // check if there was a proper rotation
          enc_delta += (enc_result & 2) - 1;                            // and count the rotation
        }

        val = enc_delta;                                                // calculate for given encoder type
        switch (ENCODER_TYP) {  // 1,2 or 4 steps resolution encoder
          case  2: enc_delta = val & 1; val >>= 1; break;
          case  4: enc_delta = val & 3; val >>= 2; break;
          default: enc_delta = 0; break;
        }

        e.pinNum = 0;
        e.timestamp = millis();

        if ( val == 1 ) {
            e.direction = e_right;
            buf.add(e);
        }
        else if ( val == -1) {
          e.direction = e_left;
          buf.add(e);
        }
        else {
          ;
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