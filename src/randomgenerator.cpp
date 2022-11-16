/* cSpell:disable */
#include "Arduino.h"
#include "randomgenerator.h"

void seedrnd(unsigned long t, uint32_t &randomVal){
  
#define RANDOM_01
#ifdef RANDOM_01
  uint32_t random_bit;
  randomVal = 0;
  volatile uint32_t *rnd_reg = (uint32_t *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);

  for (int k = 0; k < 32; k++) {
    while (1) {
      random_bit = (*rnd_reg) & 1;
      if (random_bit != ((*rnd_reg) & 1)) break;
    }
    randomVal = (randomVal << 1) | random_bit;
  }
  // randomVal *= t*abs(analogRead(analog0)+analogRead(analog1));

  // secuenceN += 1;
  // Serial.print(secuenceN); 
  // Serial.print(" : "); 
  // Serial.print(randomVal); 
  // Serial.print(" : "); 
  // Serial.println((randomVal % 6)+1);

#else
uint32_t randomVal = 0x811c9dc5;
  uint8_t next_byte = 0;
  volatile uint32_t *rnd_reg = (uint32_t *)(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);

  for (int i = 0; i < 16; i++) {
    for (int k = 0; k < 8; k++) {
      next_byte = (next_byte << 1) | (*rnd_reg & 1);
    }

    randomVal ^= next_byte;
    randomVal *= 0x01000193;
  }
#endif
  randomVal += t+abs(analogRead(analog0)-analogRead(analog1));
  randomSeed(randomVal);    
}