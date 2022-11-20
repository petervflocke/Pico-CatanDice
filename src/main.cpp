/* cSpell:disable */
// #define DEBUG_RANDOM
#ifndef ARDUINO_ARCH_RP2040
  #error works only on earlephilhower core
#endif
#include "Arduino.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/regs/rosc.h"
#include "hardware/regs/addressmap.h"
#include <TFT_eSPI.h> 
#include "Free_Fonts.h"
#include <RingBufCPP.h>
#include <ptScheduler.h>
#include <SD.h>
#include "settings.h"
#include "songs.h"
#include "logger.h"
#include "graphics.h"
#include "keyhandler.h"
#include "statehandler.h"
#include "randomgenerator.h"

TFT_eSPI tft = TFT_eSPI();  
TFT_eSprite scrollText1  = TFT_eSprite(&tft);

ptScheduler  pt_random_waiting_for_press = ptScheduler(PT_TIME_2S);
ptScheduler  pt_random_display = ptScheduler(PT_TIME_20MS);
ptScheduler  pt_song = ptScheduler(1);

// volatile long secuenceN = 0;

// ring buffer
RingBufCPP<struct Event, MAX_NUM_ELEMENTS> buf;

boolean sdCardOK;
int noteIndex;
state_type stateTable[stateNumber][eventNumber];

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  tft.init();   // old tft.begin();
  tft.setRotation(3);  // landscape upside down
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, counterWidth, counterHeight, counter);
  for (unsigned long i=0; i<=(gHeight-1)*gWidth*1; i+=gWidth*4) {
    genTone();
    tft.pushImage(lx, ly, gWidth, gHeight, space+i);
    tft.pushImage(rx, ry, gWidth, gHeight, space+i);
    delay(20);
  }
  
  // Prepare pins and buttons handling
  pinMode(LED_PIN, OUTPUT);
  pinMode(pinBut, INPUT_PULLUP);
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  gpio_set_irq_enabled_with_callback(pinBut, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true, &gpio_callback);
  gpio_set_irq_enabled(pinA, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true);
  gpio_set_irq_enabled(pinB, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true);

  myStat.currentDur = millis();
  sdCardOK = SD.begin(PIN_SD_CS, SPI);
  if (sdCardOK) {
    sdCardOK = myLogger(SD);
  }
  if (sdCardOK) {
    sdCardOK = saveResult(SD, 1, 0, 0, 0, 0, 0, 0, myStat.currentDur);
  }

  initState();
  unsigned long tt = millis();
  Serial.println("Start reading");
  
  if (readResult(SD)) {
    Serial.println("Reading ok");  
  };
  Serial.print("Stop reading after: ");
  Serial.println(millis()-tt);
  // delay(1500-tt);

  Serial.print("allDur     : "); Serial.println(myStat.allDur);
  Serial.print("currentDur : "); Serial.println(myStat.currentDur);
  Serial.print("numberGames: "); Serial.println(myStat.numberGames);
  Serial.print("numberDraws: "); Serial.println(myStat.numberDraws);
  Serial.print("inputError : "); Serial.println(myStat.inputError);
  Serial.print("lineNumberError : "); Serial.println(myStat.lineNumberError);

  for (int i=0; i<6; i++) {
    Serial.print(" L "); Serial.print(i+1); Serial.print(" : "); Serial.print(statTabLg[i]); Serial.print(" - "); Serial.println( double(statTabLg[i])/float(myStat.numberDraws)*100.0, 2 );
  }
  for (int i=0; i<6; i++) {
    Serial.print(" R "); Serial.print(i+1); Serial.print(" : "); Serial.print(statTabRg[i]); Serial.print(" - "); Serial.println( double(statTabRg[i])/float(myStat.numberDraws)*100.0, 2 );
  }
  Serial.print("i : "); Serial.println(myStat.numberDraws);
  for (int i=0; i<11; i++) {
    Serial.print(" S "); Serial.print(i+2); Serial.print(" : "); Serial.print(statTabSg[i]); Serial.print(" - "); Serial.println( double(statTabSg[i])/float(myStat.numberDraws)*100.0, 2 );
  }


}


void loop()
{ 
  u_int32_t statCnt=0;

  statTab_t  statTabL = {0, 0, 0, 0, 0, 0};
  statTab_t  statTabR = {0, 0, 0, 0, 0, 0};
  statTabS_t statTabS = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  statTabSg[0] = 0;

  int scrollDelay;
  int steps;

  int rnd1; 
  int rnd2;
  uint32_t seedL;
  uint32_t seedR;
  int rndSum;
  int messageGLen;
  int messageIndex;
    
  unsigned long loop_i;
  int loop_j;
  bool loop_z;
  bool loop_w;

  char MessageStr[MessageLen];

  uint16_t resultFrameColor;

  unsigned long duration1;
  unsigned long duration2;

  duration1 = millis();

  state_type current_state = random_waiting_for_press;
  state_type former_state = none;


  while (true) {
    if (!buf.isEmpty())
      {
        // current_state = key_logic(former_state);
        current_state = key_logic(current_state);
      }  
/*     if (current_state != former_state) {
      // Serial.print("Old / New State: "); 
      // Serial.print(former_state);
      // Serial.print("->");
      // Serial.println(current_state);
      former_state = current_state;
    } */

    if (current_state == random_waiting_for_press) {
      if (pt_random_waiting_for_press.call()) {
        tone(BuzzerPin,NOTE_G2, 6);
        delay(5);
        tone(BuzzerPin,NOTE_GS2, 5);
        delay(5);
      }

      tft.pushImage(lx, ly, gWidth, gHeight, qq+(random(100)%2?1:-1)*random(4)*gWidth);
      delay(25);
      tft.pushImage(rx, ry, gWidth, gHeight, qq+(random(100)%2?1:-1)*random(4)*gWidth);
      delay(25);

      // first_run = true;
      former_state = current_state;
    } 
    else if ( (current_state == random_waiting_for_release) ) {
      // if (first_run) {
      if (current_state != former_state) {
        duration1 = (millis()-duration1);
        seedrnd(duration1, seedL);
        rnd1 = random(1, 7);
        
        #ifdef DEBUG_RANDOM
        Serial.print("1. random -> "); Serial.println(rnd1);
        // Serial.print(duration1); Serial.print(" -> ");
        // Serial.println(rnd1);
        #endif

        duration2 = millis();
        steps = slowdown[0][0];
        scrollDelay = slowdown[0][1];
        tft.fillCircle(150-ledSize/2, 0+ledSize/2, ledSize/2, TFT_RED);
        tft.drawCircle(150-ledSize/2, 0+ledSize/2, ledSize/2, TFT_WHITE);
        loop_i = 0;

        for (unsigned long i=0; i<=top_pos; i+=gWidth*steps) {
          genTone();
          tft.pushImage(lx, ly, gWidth, gHeight, ptr_start+i);
          tft.pushImage(rx, ry, gWidth, gHeight, ptr_stop -i);
          delay(scrollDelay);
        }
        // first_run = false;
        former_state = current_state;
      }
      if (loop_i <= top_pos) {
        genTone();
        tft.pushImage(lx, ly, gWidth, gHeight, ptr_start+loop_i);
        tft.pushImage(rx, ry, gWidth, gHeight, ptr_stop -loop_i);
        loop_i+=gWidth*steps;
        delay(scrollDelay);
      } else {
        loop_i = 0;
      }
    }
    else if (current_state == random_slow_down) {
      //tft.pushImage(0, 0, counterWidth, ledSize+1, counter);
      duration2 = (millis()-duration2)+random(analogRead(analog0));
      seedrnd(duration2, seedR);
      rnd2 = random(1, 7);
      #ifdef DEBUG_RANDOM
      Serial.print("2. random -> "); Serial.println(rnd2);
      // Serial.print(duration1); Serial.print(" -> ");  
      // Serial.println(rnd2);
      #endif
      steps = slowdown[0][0];
      scrollDelay = slowdown[0][1];
      tft.fillCircle(150-ledSize/2, 0+ledSize/2, ledSize/2, TFT_GREEN);
      tft.drawCircle(150-ledSize/2, 0+ledSize/2, ledSize/2, TFT_WHITE);
      for (unsigned long i=loop_i; i<=top_pos; i+=gWidth*steps) {
        genTone();
        tft.pushImage(lx, ly, gWidth, gHeight, ptr_start+i);
        tft.pushImage(rx, ry, gWidth, gHeight, ptr_stop -i);
        delay(scrollDelay);
        int slowdown_index = (i/(gWidth*(gHeight-1)))%SlowDownEl;
        steps = slowdown[slowdown_index][0];
        scrollDelay = slowdown[slowdown_index][1];
      }
      tft.pushImage(0, 0, counterWidth, ledSize+1, counter);
      // rnd1 = 6;
      // rnd2 = 6;
      bool run1 = true;
      bool run2 = true;
      steps = slowdown[SlowDownEl-1][0];
      for (unsigned long i=0; i<=top_pos; i+=gWidth*steps) {
        genTone();
        if (run1) {
            tft.pushImage(lx, ly, gWidth, gHeight, ptr_start+i);
            if (ptr_start+i >= digits[rnd1]) {
              tft.pushImage(lx, ly, gWidth, gHeight, digits[rnd1]);
              run1 = false;
            }
        }
        if (run2) {   
            tft.pushImage(rx, ry, gWidth, gHeight, ptr_stop-i);
            if (ptr_stop-i <= digits[rnd2]) {
              tft.pushImage(rx, ry, gWidth, gHeight, digits[rnd2]);  
              run2 = false;
            }
        }
        if (!(run1 or run2)) {
          break;
        }
        else {
          delay(scrollDelay);
          scrollDelay += slowdown[SlowDownEl-1][1];
        }
      }
      if (sdCardOK) {
        sdCardOK = saveResult(SD, 0, rnd1, rnd2, duration1, duration2, seedL, seedR, millis());
      }

      delay(1000);
      tft.pushImage(lx, ly, gWidth, gHeight, space);
      tft.pushImage(rx, ry, gWidth, gHeight, space);
      tft.fillRoundRect(rRX+4, rRY+4, rWi, rHi, 5, TFT_BLACK);
      tft.fillRoundRect(rRX  , rRY  , rWi, rHi, 5, TFT_WHITE);

      rndSum = rnd1+rnd2;
      if (rndSum < 10) {
        tft.pushImage(rRX+rWi/2-gWidth/2, rRY+(rHi)/2-mHi/2, gWidth, mHi, digits[rndSum]   +gWidth*dSt);
      } else {
        tft.pushImage(rRX+4,              rRY+(rHi)/2-mHi/2, gWidth, mHi, digits[rndSum/10]+gWidth*dSt);
        tft.pushImage(rRX+4+42+4,         rRY+(rHi)/2-mHi/2, gWidth, mHi, digits[rndSum%10]+gWidth*dSt);
      }
      resultFrameColor = 0;

      statTabL[rnd1-1] += 1;
      statTabR[rnd2-1] += 1;
      statTabS[rndSum-2] += 1;
      statCnt += 1;

      //#define DEBUG_RANDOM
      #ifdef DEBUG_RANDOM
      Serial.print("Count : "); Serial.println(statCnt);
      for (int i=0; i<6; i++) {
        Serial.print(" L "); Serial.print(i+1); Serial.print(" : "); Serial.print(statTabL[i]); Serial.print(" - "); Serial.println( double(statTabL[i])/float(statCnt)*100.0, 2 );
      }
      for (int i=0; i<6; i++) {
        Serial.print(" R "); Serial.print(i+1); Serial.print(" : "); Serial.print(statTabR[i]); Serial.print(" - "); Serial.println( double(statTabR[i])/float(statCnt)*100.0, 2 );
      }
      Serial.print("i : "); Serial.println(statCnt);
      for (int i=0; i<11; i++) {
        Serial.print(" S "); Serial.print(i+2); Serial.print(" : "); Serial.print(statTabS[i]); Serial.print(" - "); Serial.println( double(statTabS[i])/float(statCnt)*100.0, 2 );
      }
      #endif

      snprintf_P(MessageStr, sizeof(MessageStr), 
          /* PSTR("Press START to continue. [%d] L:%d %.0f%% | R:%d %.0f%% | S:%d %.0f%%"), */
          MessageText1,
          statCnt,
          rnd1,   float(statTabL[rnd1-1])/statCnt*100.0,
          rnd2,   float(statTabR[rnd2-1])/statCnt*100.0,
          rndSum, float(statTabS[rndSum-2])/statCnt*100.0
      );
      messageGLen = tft.textWidth(MessageStr, 2)+10;
      scrollText1.setColorDepth(8);
      scrollText1.createSprite(rBl+messageGLen, rBh);
      scrollText1.fillSprite(TFT_BLUE);
      scrollText1.setScrollRect(0, 0, rBl+messageGLen, rBh, TFT_BLUE); 
      scrollText1.setTextColor(TFT_WHITE);


      scrollText1.drawString(MessageStr, rBl, 2, 2);
      scrollText1.pushSprite(15, rRY+rHi+9, 0, 0, rBl, rBh);
      messageIndex = messageGLen;

      // Prepare songs
      if ( rndSum != 7) {
        pt_song.setSequenceRepetition(Song2Len * 1); // repeat song 1 time
      } else {
        pt_song.setSequenceRepetition(0); // repeat 4ever
      }
      pt_song.enable();
      noteIndex = 0;

      // current_state = random_display;      
      key_clean();
      key_none();
    }
    else if (current_state == random_display) {
      if (current_state != former_state) {

        former_state = current_state;
      }       
      // display
      // rndSum = 7;
      u_int32_t framecolor;
      if (pt_random_display.call()) {
        if (rndSum == 7) {
          if (resultFrameColor%2) framecolor=TFT_RED;
          else                    framecolor=TFT_WHITE;
          framecolor = default_4bit_palette[resultFrameColor];
          tft.drawCircle(rRX+rWi/2, rRY+rHi/2, gWidth/2+10, framecolor);
          tft.drawCircle(rRX+rWi/2, rRY+rHi/2, gWidth/2+11, framecolor);
          tft.drawCircle(rRX+rWi/2, rRY+rHi/2, gWidth/2+12, framecolor);
          tft.drawCircle(rRX+rWi/2, rRY+rHi/2, gWidth/2+13, framecolor);
        } else {
          framecolor = TFT_GOLD;
          // genTone();
        }
        tft.drawRoundRect(rRX-1  , rRY-1  , rWi, rHi, 5, framecolor);
        tft.drawRoundRect(rRX    , rRY    , rWi, rHi, 5, framecolor);
        tft.drawRoundRect(rRX+1  , rRY+1  , rWi, rHi, 5, framecolor);
        resultFrameColor = (resultFrameColor + 1) % 15;
        scrollText1.scroll(-1);
        messageIndex -= 1;
        if (messageIndex <=0) {
          messageIndex = messageGLen;
          scrollText1.drawString(MessageStr, rBl, 2, 2);
        }
        scrollText1.pushSprite(15, rRY+rHi+9, 0, 0, rBl, rBh);
      }
      // Play a song
      if (rndSum == 7) {
        genSignal(pt_song, song4, Song4Len, Song4Tempo, noteIndex);
      } else {
        genSignal(pt_song, song2, Song2Len, Song2Tempo, noteIndex);
      }
    }
    else if (current_state == show_statistics) {

      pt_song.disable();

      #define KEEP_
      #ifndef KEEP_

      // scrollText1.deleteSprite();
      tft.pushImage(0, 0, counterWidth, counterHeight, counter);
      tft.fillRoundRect(sRX+4, sRY+4, sWi, sHi, 5, TFT_BLACK);
      tft.fillRoundRect(sRX  , sRY  , sWi, sHi, 5, TFT_WHITE);
      tft.drawRoundRect(sRX-1, sRY-1, sWi, sHi, 5, TFT_GOLD);
      tft.drawRoundRect(sRX  , sRY  , sWi, sHi, 5, TFT_GOLD);
      tft.drawRoundRect(sRX+1, sRY+1, sWi, sHi, 5, TFT_GOLD);
      tft.drawFastVLine(sRX+2*sXY, sRY+sXY, sHi-2*sXY, TFT_BLACK);
    
      tft.setFreeFont(TT1);
      tft.setTextColor(TFT_BLUE,TFT_WHITE);  tft.setTextSize(1);
      
      u_int32_t maxProb = 0;
      for (int i=0; i<11; i++) {
        maxProb = (maxProb < statTabS[i]? statTabS[i] : maxProb);
      }
      maxProb = 100*maxProb/statCnt;
      // Serial.print("maxProb :"); Serial.println(maxProb);
      for (int i=maxProb; i > 0; i-=maxProb/5) {
        tft.drawNumber(i, sRX+sXY-(i==100?2:0), sRY+sXY+(((maxProb-i)*(sHi-3*sXY)/maxProb))-2);
        tft.drawFastHLine(sRX+2*sXY+1, sRY+sXY+((maxProb-i)*(sHi-3*sXY)/maxProb), sWi-3*sXY-1, TFT_LIGHTGREY);
      }
      tft.drawFastHLine(sRX+sXY, sRY+sHi-2*sXY, sWi-2*sXY, TFT_BLACK);
      for (int i=0; i<11; i++) {
        int barHight=100*statTabS[i]/statCnt;
        // Serial.print(i+2); Serial.print(" : "); Serial.print(barHight);
        barHight = map(barHight, 0, maxProb, 0, sHi-3*sXY);
        // Serial.print(" : "); Serial.println(barHight);
        tft.fillRect (sRX+2*sXY+sBw+i*(sBw+sBm), sRY+sHi-2*sXY-barHight, sBw, barHight, TFT_BLUE);
        tft.drawNumber(i+2, sRX+2*sXY+sBw+i*(sBw+sBm), sRY+sHi-2*sXY+sBm);
      }
      #endif
      //drawChart(statTabS, 11, 2);
      // drawChart(statTabL, 6, 1);
      // drawChart(statTabR, 6, 1);
      // drawBarChart(sum_of_both);
      // drawBarChart(left_and_Right);

      drawBarChart(tft, statTabL, statTabR, statTabS, statCnt, left_and_Right);
      // drawBarChart(tft, statTabL, statTabR, statTabS, statCnt, sum_of_both);
      delay(200);

/*
      snprintf_P(MessageStr, sizeof(MessageStr), 
          MessageText2,
          statCnt,
          rnd1,   float(statTabL[rnd1-1])/statCnt*100.0,
          rnd2,   float(statTabR[rnd2-1])/statCnt*100.0
      );
      // tft.setTextFont(2);
      // tft.setTextColor(TFT_WHITE,TFT_BLUE);
      // tft.setCursor(10,sRY+sHi+sXY);

      scrollText1.drawString(MessageStr, rBl, 2, 2);
      scrollText1.pushSprite(15, sRY+sHi+sXY, 0, 0, rBl, rBh);
*/      
      // current_state = wait_in_statistics;
      key_clean();
      key_none();
    }
    else if (current_state == show_statistics_Single) {
      if (current_state != former_state) {
        pt_song.disable();
        drawBarChart(tft, statTabL, statTabR, statTabS, statCnt, left_and_Right);
        former_state = current_state;
      } 
      delay(100);
      // current_state = wait_in_statistics;
      // key_clean();
      // key_none();
    }
    else if (current_state == show_statistics_Sum) {
      if (current_state != former_state) {
        pt_song.disable();
        drawBarChart(tft, statTabL, statTabR, statTabS, statCnt, sum_of_both);
        former_state = current_state;
      } 
      delay(100);
      // current_state = wait_in_statistics;
      // key_clean();
      // key_none();
    }
    else if (current_state == show_statistics_SingleAll) {
      if (current_state != former_state) {
      pt_song.disable();
      drawBarChart(tft, statTabLg, statTabRg, statTabSg, statCnt, left_and_Right);
        former_state = current_state;
      } 
      delay(100);
      // current_state = wait_in_statistics;
      // key_clean();
      // key_none();
    }
    else if (current_state == show_statistics_SumAll) {
      if (current_state != former_state) {
        pt_song.disable();
        drawBarChart(tft, statTabLg, statTabRg, statTabSg, statCnt, sum_of_both);
        former_state = current_state;
      } 
      delay(100);
      // current_state = wait_in_statistics;
      // key_clean();
      // key_none();
    }    
    else if (current_state == wait_in_statistics) {
      pt_song.disable();
      delay(20);
    }
    else if (current_state == before_waiting_for_press) {
      scrollText1.deleteSprite();
      tft.pushImage(0, 0, counterWidth, counterHeight, counter);
      delay(100);
      duration1 = millis()+analogRead(analog1);
      // current_state = random_waiting_for_press;
      key_clean();
      key_none();
    }
  }
}