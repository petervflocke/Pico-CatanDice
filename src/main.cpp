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
#include "SdFat.h"
#include "settings.h"
#include "songs.h"
#include "logger.h"
#include "graphics.h"
#include "keyhandler.h"
#include "statehandler.h"
#include "randomgenerator.h"
#include "runningaverage.h"

#include "WavPwmAudio.h"
#include "WAVData.h"

 const unsigned short *wavs[]={
  wav2,
  wav3,
  wav4,
  wav5,
  wav6,
  wav7,
  wav8,
  wav9,
  wav10,
  wav11,
  wav12
 };
#define MAX_COUNT 10


TFT_eSPI tft = TFT_eSPI();  
TFT_eSprite scrollText1  = TFT_eSprite(&tft);

ptScheduler  pt_random_waiting_for_press = ptScheduler(PT_TIME_2S);
ptScheduler  pt_random_display = ptScheduler(PT_TIME_20MS);
ptScheduler  pt_random_say = ptScheduler(PT_TIME_4S);
ptScheduler  pt_delay_bird = ptScheduler(ScreenSaverAnimationDelay);
ptScheduler  pt_song = ptScheduler(1);

// volatile long secuenceN = 0;

// ring buffer
RingBufCPP<struct Event, MAX_NUM_ELEMENTS> buf;

SdFat32 sd;

boolean sdCardOK;
int noteIndex;
state_type stateTable[stateNumber][eventNumber];

RunningAverage ra;

void setup()
{
  Serial.begin(115200);
  // while (!Serial) {
  //   yield();
  // }
  
  // Prepare pins and buttons handling
  pinMode(LED_PIN, OUTPUT);
  pinMode(MutePin, OUTPUT);
  pinMode(pinBut, INPUT_PULLUP);
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);

  analogReadResolution(12);
  for (int i = 0; i < maxSample; i++) {
    ra.addValue(analogRead(BatPin));
    delay(10);
  }

  WavPwmInit(GPIO_AUDIO_OUT_LEFT);
  digitalWrite(MutePin, 0); // unmute amplifier

  tft.init();   // old tft.begin();
  tft.setRotation(3);  // landscape upside down
  tft.setSwapBytes(true);

  tft.pushImage(0, 0, catanWidth, catanHeight, catan);
  // sdCardOK = SD.begin(PIN_SD_CS, SPI);
  // sdCardOK = sd.cardBegin(SD_CONFIG);
  sdCardOK = sd.begin(SD_CONFIG);
  if (sdCardOK) {
    // delay(1000);
    // Serial.println("Start reading from SD");
    // long lFreeKB = sd.vol()->freeClusterCount();
    // lFreeKB *= sd.vol()->sectorsPerCluster()/2;
    // Serial.println(lFreeKB);
    sdCardOK = myLogger(sd);
  }
  
  myStat.currentDur = millis();
  if (sdCardOK) {
  // Serial.println("Start writing to SD");
    sdCardOK = saveResult(1, 0, 0, 0, 0, 0, 0, myStat.currentDur);
  }

  if (sdCardOK) {
    tft.setTextColor(TFT_WHITE);
    tft.drawString("SD card OK", SDX2e, SDYe, 2);
    delay(2000);
  } else {
    showSDError(tft);
    tft.setTextColor(TFT_RED);
    while (gpio_get(pinBut)) {
      tft.fillRect(SDX1e, SDYe, 45, 16, sdcardback);
      delay(100);
      tft.drawString("ERROR", SDX1e, SDYe, 2);
      delay(100);
    }    
  }
  tft.fillScreen(TFT_BLACK);
  tft.pushImage(0, 0, counterWidth, counterHeight, counter);
  for (unsigned long i=0; i<=(gHeight-1)*gWidth*1; i+=gWidth*4) {
    genTone();
    tft.pushImage(lx, ly, gWidth, gHeight, space+i);
    tft.pushImage(rx, ry, gWidth, gHeight, space+i);
    delay(20);
  }
  // attached pin irq
  gpio_set_irq_enabled_with_callback(pinBut, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true, &gpio_callback);
  gpio_set_irq_enabled(pinA, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true);
  gpio_set_irq_enabled(pinB, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true);

  initState();

  // unsigned long tt = millis();
  if (sdCardOK) {
    // Serial.println("Start reading");
    if (sdCardOK=!readResult()) {
      // Serial.println("Reading ok");  
    };
  } else myStat.inputError = false;


  // uint32_t freeKB = SD.vol()->freeClusterCount();
  // freeKB *= SD.vol()->blocksPerCluster()/2;
  // Serial.print("Free space KB: ");
  // Serial.println(freeKB);


/*   
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
*/
}


void loop()
{ 
  u_int32_t statCnt=0;

  statTab_t  statTabL = {0, 0, 0, 0, 0, 0};
  statTab_t  statTabR = {0, 0, 0, 0, 0, 0};
  statTabS_t statTabS = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  int scrollDelay;
  int steps;

  int rnd1=0; 
  int rnd2=0;
  uint32_t seedL;
  uint32_t seedR;
  int rndSum;
  int messageGLen;
  int messageIndex;
  unsigned long screenSaver = 0;
  
  unsigned int birdN;
  unsigned int BirdX;
  unsigned int BirdY;

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
  bool doBreak;

  #define MAXBACKLIGHT 16
  int backlight = MAXBACKLIGHT;

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
    }
*/
    if (current_state != former_state) {
      ra.addValue(analogRead(BatPin));
    }

    if (current_state == screen_saver) {
      if (current_state != former_state) {
        tft.pushImage(0, 0, catanWidth, catanHeight, catan);
        birdN = birdMaxN;
        former_state = current_state;
        BirdX = 15; // initilaized only for first removal, then random
        BirdY =  1;
      }
      if (pt_delay_bird.call()) {
        if (birdN >= birdMaxN) {
          restoreBack(tft, BirdX + tabBirds[birdMaxN-1].bx, BirdY + tabBirds[birdMaxN-1].by, tabBirds[birdMaxN-1].b_width, tabBirds[birdMaxN-1].b_height, catanWidth, catan);          
          birdN = 0;
          BirdX = random(15, 46);
          BirdY = random(1, 16);
        }
        else {
          if (birdN > 0) restoreBack(tft, BirdX + tabBirds[birdN-1].bx, BirdY + tabBirds[birdN-1].by, tabBirds[birdN-1].b_width, tabBirds[birdN-1].b_height, catanWidth, catan);
          tft.pushImage(BirdX + tabBirds[birdN].bx, BirdY + tabBirds[birdN].by, tabBirds[birdN].b_width, tabBirds[birdN].b_height, tabBirds[birdN].bird, TFT_WHITE);
          drawTime(tft, 65, 100, duration1);
          birdN += 1;
        }
      }
    } 
    if ( (current_state == ibacklight) || (current_state == dbacklight) ) {
      if (current_state != former_state) {
        int bl = (current_state == ibacklight)? +1:-1;
        backlight += bl;
        if ( backlight < 1  ) backlight = 1;
        if ( backlight > MAXBACKLIGHT ) backlight = MAXBACKLIGHT;
        Serial.println(map(backlight, 1, MAXBACKLIGHT, 1, 128));
        if (backlight == MAXBACKLIGHT) 
          analogWrite(TFT_BL, 255);
        else
          analogWrite(TFT_BL, map(backlight, 1, MAXBACKLIGHT, 1, 128));
        former_state = current_state;
      }
      key_clean();
      key_none();      
    }    
    if (current_state == random_waiting_for_press) {
      if (current_state != former_state) {
        tft.pushImage(0, 0, counterWidth, counterHeight, counter);
        screenSaver = millis();
        former_state = current_state;
      }
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

      if (millis() - screenSaver > screenSaverTimeout) {
        // current_state = screen_saver;
        key_clean();
        key_none();
      }
      /*
      U(BatPin) = 3.3/4095 = U2
      U(Volatege devider VBat-R1-BatPin-R2-GND)= ((R1+R2)/R2)*U2
      R1=553K, R2=991K
      */
      // Serial.println ((analogRead(BatPin)*3300)/4095)*16);
      // Serial.println (analogRead(A2)*1289);
    } 
    else if ( (current_state == random_waiting_for_release) ) {
      // if (first_run) {
      if (current_state != former_state) {
        duration1 = millis()-duration1 + analogRead(analog1);
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
      duration2 = millis()-duration2 + analogRead(analog0);
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
        doBreak = checkBreak();
        if (doBreak) {
          break;
        }        
        genTone();
        tft.pushImage(lx, ly, gWidth, gHeight, ptr_start+i);
        tft.pushImage(rx, ry, gWidth, gHeight, ptr_stop -i);
        delay(scrollDelay);
        int slowdown_index = (i/(gWidth*(gHeight-1)))%SlowDownEl;
        steps = slowdown[slowdown_index][0];
        scrollDelay = slowdown[slowdown_index][1];
      }
      tft.pushImage(0, 0, counterWidth, ledSize+1, counter);
      if (!doBreak) {
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
          doBreak = checkBreak();
          if (doBreak) {
            break;
          }    
        }
      }
      if (doBreak) {
        tft.pushImage(lx, ly, gWidth, gHeight, digits[rnd1]);
        tft.pushImage(rx, ry, gWidth, gHeight, digits[rnd2]); 
        delay(scrollDelay);
      }

      // current_state = random_display;      
      key_clean();
      key_none();
    }
    else if (current_state == random_display) {
      if (current_state != former_state) {

        if (sdCardOK) {
          sdCardOK = saveResult(0, rnd1, rnd2, duration1, duration2, seedL, seedR, millis());
        }

        delay(1000);
        key_clean();        
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

        statTabLg[rnd1-1] += 1;
        statTabRg[rnd2-1] += 1;
        statTabSg[rndSum-2] += 1;
        myStat.numberDraws += 1;        

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
          pt_song.setSequenceRepetition(Song4Len * 2); // repeat song 1 time or 0 for 4ever
        }
        pt_song.reset();
        pt_song.enable();
        noteIndex = 0;

        if (BuzzerPin == GPIO_AUDIO_OUT_LEFT)
          WavPwmInit(GPIO_AUDIO_OUT_LEFT);
        WavPwmPlayAudio(wavs[rndSum-2]);
        pt_random_say.reset();        
        pt_random_say.setSkipSequence(1);
        pt_random_say.setSequenceRepetition(3);
        
        former_state = current_state;

        // if ( 1 < rndSum && rndSum < 13 ) {
        //   WavPwmPlayAudio(wavs[rndSum-2]);
        //   while (WavPwmIsPlaying());    
        // }

      }       
      // display
      // rndSum = 7;
      u_int32_t framecolor;
      if (pt_random_display.call()) {
        if (pt_random_say.call()) {

          if (BuzzerPin == GPIO_AUDIO_OUT_LEFT)
            WavPwmInit(GPIO_AUDIO_OUT_LEFT);
          WavPwmPlayAudio(wavs[rndSum-2]);
        }
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

      // pt_song.disable();

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

      drawBarChart(tft, statTabL, statTabR, statTabS, statCnt, left_and_Right, "old");
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
        // pt_song.disable();
        drawBarChart(tft, statTabL, statTabR, statTabS, statCnt, left_and_Right, "Left & Right Dice");
        former_state = current_state;
      } 
      delay(100);
    }
    else if (current_state == show_statistics_Sum) {
      if (current_state != former_state) {
        // pt_song.disable();
        drawBarChart(tft, statTabL, statTabR, statTabS, statCnt, sum_of_both, "Both Dices");
        former_state = current_state;
      } 
      delay(100);
    }
    else if (current_state == show_statistics_SingleAll) {
      if (current_state != former_state) {
        // pt_song.disable();
        if (sdCardOK) {
          drawBarChart(tft, statTabLg, statTabRg, statTabSg, myStat.numberDraws, left_and_Right, "All lefts & rights");
        } else {
          showSDError(tft);
        }
        former_state = current_state;
      } 
      delay(100);
    }
    else if (current_state == show_statistics_SumAll) {
      if (current_state != former_state) {
        // pt_song.disable();
        if (sdCardOK) {
          drawBarChart(tft, statTabLg, statTabRg, statTabSg, myStat.numberDraws, sum_of_both, "All both dices");
        } else {
          showSDError(tft);
        }
        former_state = current_state;
      } 
      delay(100);
    }
    else if (current_state == show_summary) {
      if (current_state != former_state) {
        pt_song.disable();

/*       
        int ac2 = analogRead(BatPin);
        unsigned int batVol = ra.addValue(ac2);
        Serial.print("AC=");
        Serial.print(ac2);
        Serial.print(" Average=");
        Serial.print(batVol);
        Serial.print(" Bat =");
        Serial.println((batVol*132.692871094)/100000.00);
  */
        double batVol = ra.addValue(analogRead(BatPin));
        drawInfoText(tft, rnd1, rnd2, statCnt, sd, batVol); 
        former_state = current_state;
      } 
      delay(100);
    } 

    else if ( current_state == sbacklight) {
      double batVol = ra.addValue(analogRead(BatPin));
      if (current_state != former_state) {
        drawInfoBox(tft);
        drawBacklight(tft, backlight, false); 
        former_state = current_state;
        key_clean();
      }
      drawBatVol(tft, batVol);
      delay(100);
    } 
    else if ( current_state == ebacklight) {
      double batVol = ra.addValue(analogRead(BatPin));
      if (current_state != former_state) {
        drawBacklight(tft, backlight, true); 
        former_state = current_state;
      }
      drawBatVol(tft, batVol);
      delay(100);
    }     
    else if (current_state == wait_in_statistics) {
      pt_song.disable();
      delay(20);
    }
    else if (current_state == before_waiting_for_press) {
      scrollText1.deleteSprite();
      tft.pushImage(0, 0, counterWidth, counterHeight, counter);
      delay(100);
      duration1 = millis();
      // current_state = random_waiting_for_press;
      key_clean();
      key_none();
    }
  }
}