/* cSpell:disable */
#include "Arduino.h"
#include <ptScheduler.h>
#include "songs.h"

void genTone() {
    tone(BuzzerPin,NOTE_G2, 10);
}

void genSignal(ptScheduler &pt_song, const int songTab[][2], int songLen, int tempo, int &note) {
  int noteDuration, devider;
  int wholenote = (60000 * 4) / tempo;  
  
  if (pt_song.call()) {
    devider = songTab[note][1];
    noteDuration = wholenote / abs(devider);
    if (devider < 0) {
      noteDuration *= 1.5;
    }
    tone(BuzzerPin, songTab[note][0], noteDuration*0.9);
    note = (note +1) % songLen;
    pt_song.setInterval(noteDuration*1000);
  }
}

