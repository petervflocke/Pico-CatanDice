/* cSpell:disable */
#include "Arduino.h"
#ifndef SONGS_H
#define SONGS_H
#include "pitches.h"
#define BuzzerPin 6
// index in song note score

#define Song1Tempo 180
int song1[][2] PROGMEM={
  {NOTE_C5, 4},   // 0
  {NOTE_G4, 4},   // 1
  {NOTE_E4, 4},   // 2
  {NOTE_A4, 8},   // 3
  {NOTE_B4, 8},   // 4
  {NOTE_A4, 8},    // 5
  {NOTE_GS4, 8},    // 6
  {NOTE_AS4, 8},    // 7  
  {NOTE_GS4, 8},    // 8
  {NOTE_G4, 8},    // 9
  {NOTE_D4, 8},    // 10
  {NOTE_E4, 2}    // 11
};
#define Song1Len sizeof(song1)/8

#define Song2Tempo 180
int song2[][2] PROGMEM= {           // Nokia tune
  {NOTE_E5, 8},   // 0
  {NOTE_D5, 8},   // 1
  {NOTE_FS4, 4},   // 2
  {NOTE_GS4, 4},   // 3
  {NOTE_CS5, 8},   // 4
  {NOTE_B4, 8},    // 5
  {NOTE_D4, 4},    // 6
  {NOTE_E4, 4},    // 7
  {NOTE_B4, 8},    // 8
  {NOTE_A4, 8},    // 9
  {NOTE_CS4, 4},    // 10
  {NOTE_E4, 4},    // 11
  {NOTE_A4, 2}    // 12
};
#define Song2Len sizeof(song2)/8


#define Song3Tempo 180
int song3[][2] PROGMEM= {
  {NOTE_B5, 8},   // 0
  {NOTE_E6, 16},   // 1
};
#define Song3Len sizeof(song3)/8

#define Song4Tempo 180
int song4[][2] PROGMEM= {
  {NOTE_E6, 8}, 
  {NOTE_G6, 8}, 
  {NOTE_E7, 8},
  {NOTE_C7, 8},
  {NOTE_D7, 8},
  {NOTE_G7, 8},
};
#define Song4Len sizeof(song4)/8


#define Song5Tempo 180
int song5[][2] PROGMEM= {
  {NOTE_G4, 3}, 
  {NOTE_G5, 3}, 
  {NOTE_G6, 3},
};
#define Song5Len sizeof(song5)/8

#endif