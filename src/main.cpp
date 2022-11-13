/* cSpell:disable */
// #define DEBUG_02
// #define DEBUG_RANDOM
#ifndef ARDUINO_ARCH_RP2040
  #error works only on earlephilhower core
#endif
#include "Arduino.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/regs/rosc.h"
#include "hardware/regs/addressmap.h"
#include <TFT_eSPI.h>       // Hardware-specific library
#include "Free_Fonts.h"
#include <RingBufCPP.h>
#include <ptScheduler.h>
// #include <SPI.h>
#include <SD.h>

#include "settings.h"
#include "pic.h"
#include "counter.h"
#include "songs.h"
// #include "pitches.h"
#include "logger.h"

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
TFT_eSprite scrollText1  = TFT_eSprite(&tft);

// table with digit pointers for a reel data
const unsigned short *space = (unsigned short *)reel;
const unsigned short *qq = (unsigned short *)reel +((gHeight-1)*gWidth*1);
const unsigned short *digits[] = {
  (unsigned short *)reel+((gHeight-1)*gWidth*13), // 0
  (unsigned short *)reel+((gHeight-1)*gWidth* 3), // 1
  (unsigned short *)reel+((gHeight-1)*gWidth* 4), // 2
  (unsigned short *)reel+((gHeight-1)*gWidth* 5), // 3
  (unsigned short *)reel+((gHeight-1)*gWidth* 6), // 4
  (unsigned short *)reel+((gHeight-1)*gWidth* 7), // 5
  (unsigned short *)reel+((gHeight-1)*gWidth* 8), // 6
  (unsigned short *)reel+((gHeight-1)*gWidth*10), // 7
  (unsigned short *)reel+((gHeight-1)*gWidth*11), // 8
  (unsigned short *)reel+((gHeight-1)*gWidth*12), // 9
};
const unsigned short *ptr=(unsigned short *)reel;
const unsigned short *ptr_start = ptr + ((gHeight-1)*gWidth*  3); // 1
const unsigned short *ptr_stop  = ptr + ((gHeight-1)*gWidth*  9); // 1
const unsigned int top_pos = (gHeight-1)*gWidth*6;

ptScheduler  pt_random_waiting_for_press = ptScheduler(PT_TIME_2S);
ptScheduler  pt_random_display = ptScheduler(PT_TIME_20MS);
ptScheduler  pt_song = ptScheduler(1);

volatile byte xstate = LOW; // change build in led when a button is pressed
// volatile long secuenceN = 0;

typedef enum {pin_fall, pin_reis} pinState_t;

typedef enum {
  random_waiting_for_press,     // 0
  random_waiting_for_release,   // 1
  random_slow_down,             // 2
  random_display,               // 3
  show_statistics,              // 4
  wait_in_statistics,           // 5
  before_waiting_for_press,     // 6
  none                          // 7
} state_type;

typedef enum {
  left_and_Right,  
  sum_of_both
} BarChartType;

typedef enum {
  e_none,
  e_right,
  e_left,
  e_down,
  e_up
} coder_type;

struct Event
{
  // uint32_t pinState;
  uint pinNum;
  coder_type direction;
  unsigned long timestamp;
};

// ring buffer
#define MAX_NUM_ELEMENTS 10
RingBufCPP<struct Event, MAX_NUM_ELEMENTS> buf;

unsigned long stime = to_ms_since_boot(get_absolute_time());
const int delayTime = 20; // Delay for every push button may vary

void gpio_callback(uint gpio, uint32_t events) {
static volatile int last_delta = 0;
static volatile int last_seq = 0;
static volatile int last_remainder = 0;
static volatile int last_cycles = 0;

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

void key_clean() {
  struct Event e;
  noInterrupts();
  while (!buf.isEmpty()) {
    buf.pull(&e);
  }
  interrupts();
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
        new_state = show_statistics;
      }
    else if (old_state == wait_in_statistics and e.direction == e_up)
      {
        new_state = before_waiting_for_press;
      }
    else new_state = old_state;
  }
  return new_state;
}

u_int32_t statTabL[6];
u_int32_t statTabR[6];
u_int32_t statTabS[11];
/*  statistics[ 0] =>  2
    statistics[ 1] =>  3
    statistics[ 2] =>  4
    ...
    statistics[10] => 12
*/
u_int32_t statCnt=0;


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


void drawBarChart(BarChartType statTab) {
  #define sRX   7 /* stat box upper x*/
  #define sRY  20 /* stat box uper y*/
  #define sWi 143 /* stat box width */
  #define sHi  80 /* stat box high */
  #define sXY  10 /* margin of 0,0 from boarders*/
  #define sBw   5 /* bar width */
  #define sBm   5 /* gap between bars */
  #define sBwlr 6 /* bar width */
  #define sBmlr 11 /* gap between bars */
  
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
  if (statTab  == left_and_Right) {
    for (int i=0; i<6; i++) {
      maxProb = (maxProb < statTabL[i]? statTabL[i] : maxProb);
      maxProb = (maxProb < statTabR[i]? statTabR[i] : maxProb);
    }
  } else if (statTab == sum_of_both) {
    for (int i=0; i<11; i++) {
      maxProb = (maxProb < statTabS[i]? statTabS[i] : maxProb);
    }
  }
  maxProb = 100*maxProb/statCnt;
  // Serial.print("maxProb :"); Serial.println(maxProb);
  for (int i=maxProb; i > 0; i-=maxProb/5) {
    tft.drawNumber(i, sRX+sXY-(i==100?2:0), sRY+sXY+(((maxProb-i)*(sHi-3*sXY)/maxProb))-2);
    tft.drawFastHLine(sRX+2*sXY+1, sRY+sXY+((maxProb-i)*(sHi-3*sXY)/maxProb), sWi-3*sXY-1, TFT_LIGHTGREY);
  }
  tft.drawFastHLine(sRX+sXY, sRY+sHi-2*sXY, sWi-2*sXY, TFT_BLACK);
  if (statTab  == left_and_Right) {
    for (int i=0; i<6; i++) {
      int barHight=100*statTabL[i]/statCnt;
      barHight = map(barHight, 0, maxProb, 0, sHi-3*sXY);
      tft.fillRect (sRX+2*sXY+sBwlr+i*(sBwlr+sBmlr), sRY+sHi-2*sXY-barHight, sBwlr, barHight, TFT_GREEN);
      
      barHight=100*statTabR[i]/statCnt;
      barHight = map(barHight, 0, maxProb, 0, sHi-3*sXY);
      tft.fillRect (sRX+2*sXY+sBwlr+i*(sBwlr+sBmlr)+sBwlr+1, sRY+sHi-2*sXY-barHight, sBwlr, barHight, TFT_RED);

      tft.drawNumber(i+1, sRX+2*sXY+sBwlr+i*(sBwlr+sBmlr)+sBwlr-3, sRY+sHi-2*sXY+sBm);
      tft.drawFastVLine(sRX+2*sXY+sBwlr+i*(sBwlr+sBmlr)+sBwlr*2+1, sRY+sHi-2*sXY+1, sBm, TFT_LIGHTGREY);
    }    
  } else if (statTab == sum_of_both) {
    for (int i=0; i<11; i++) {
      int barHight=100*statTabS[i]/statCnt;
      // Serial.print(i+2); Serial.print(" : "); Serial.print(barHight);
      barHight = map(barHight, 0, maxProb, 0, sHi-3*sXY);
      // Serial.print(" : "); Serial.println(barHight);
      tft.fillRect (sRX+2*sXY+sBw+i*(sBw+sBm), sRY+sHi-2*sXY-barHight, sBw, barHight, TFT_BLUE);
      tft.drawNumber(i+2, sRX+2*sXY+sBw+i*(sBw+sBm), sRY+sHi-2*sXY+sBm);
    }
  }
}

void drawChart(u_int32_t* statTabX, int maxIndex, int deltaIndex) {
  #define sRX   7 /* stat box upper x*/
  #define sRY  20 /* stat box uper y*/
  #define sWi 143 /* stat box width */
  #define sHi  80 /* stat box high */
  #define sXY  10 /* margin of 0,0 from boarders*/
  #define sBw   5 /* bar width */
  #define sBm   5 /* gap between bars */
  
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
  for (int i=0; i<maxIndex; i++) {
    maxProb = (maxProb < statTabX[i]? statTabX[i] : maxProb);
  }
  maxProb = 100*maxProb/statCnt;
  // Serial.print("maxProb :"); Serial.println(maxProb);
  for (int i=maxProb; i > 0; i-=maxProb/5) {
    tft.drawNumber(i, sRX+sXY-(i==100?2:0), sRY+sXY+(((maxProb-i)*(sHi-3*sXY)/maxProb))-2);
    tft.drawFastHLine(sRX+2*sXY+1, sRY+sXY+((maxProb-i)*(sHi-3*sXY)/maxProb), sWi-3*sXY-1, TFT_LIGHTGREY);
  }
  tft.drawFastHLine(sRX+sXY, sRY+sHi-2*sXY, sWi-2*sXY, TFT_BLACK);
  for (int i=0; i<maxIndex; i++) {
    int barHight=100*statTabX[i]/statCnt;
    // Serial.print(i+2); Serial.print(" : "); Serial.print(barHight);
    barHight = map(barHight, 0, maxProb, 0, sHi-3*sXY);
    // Serial.print(" : "); Serial.println(barHight);
    tft.fillRect (sRX+2*sXY+sBw+i*(sBw+sBm), sRY+sHi-2*sXY-barHight, sBw, barHight, TFT_BLUE);
    tft.drawNumber(i+deltaIndex, sRX+2*sXY+sBw+i*(sBw+sBm), sRY+sHi-2*sXY+sBm);
  }  
}

void genTone();
void genSignal(ptScheduler &pt_song, int songTab[][2], int songLen, int tempo, int &note);

void genTone() {
    tone(BuzzerPin,NOTE_G2, 10);
}

void genSignal(ptScheduler &pt_song, int songTab[][2], int songLen, int tempo, int &note) {
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

boolean sdCardOK;
int noteIndex;

void setup()
{
  Serial.begin(115200);
  // while (!Serial);
  delay(2000);

  tft.init();   // old tft.begin();
  tft.setRotation(3);  // landscape upside down
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, counterWidth, counterHeight, counter);
  #ifndef DEBUG_02
  for (unsigned long i=0; i<=(gHeight-1)*gWidth*1; i+=gWidth*3) {
    tft.pushImage(lx, ly, gWidth, gHeight, space+i);
    tft.pushImage(rx, ry, gWidth, gHeight, space+i);
    delay(20);
  }
  #endif

  // Prepare pins and buttons handling
  pinMode(LED_PIN, OUTPUT);
  pinMode(pinBut, INPUT_PULLUP);
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  gpio_set_irq_enabled_with_callback(pinBut, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true, &gpio_callback);
  gpio_set_irq_enabled(pinA, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true);
  gpio_set_irq_enabled(pinB, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE , true);


  sdCardOK = SD.begin(PIN_SD_CS, SPI);
  if (sdCardOK) {
    sdCardOK = myLogger(SD);
  }
  if (sdCardOK) {
    sdCardOK = saveResult(SD, 1, 0, 0, 0, 0, 0, 0, millis());
  }

  /* bool headerOK = false;
  char lineBuf[CSVMessageLen];
  sdCardOK = SD.begin(PIN_SD_CS, SPI);
  if (sdCardOK) {
    myFile = SD.open(fileName, FILE_READ);
    if (myFile) {
      if (readLine(myFile, lineBuf, CSVMessageLen)) {
        Serial.print("File OK: "); Serial.println(fileName);
        Serial.println(lineBuf);
      }
      myFile.close();
      Serial.println("Comapre header: "); Serial.println(fileName);
      if (strcmp(lineBuf, CSVHeader) != 0) {
        headerOK = false;
        Serial.println("Header BAD:");
        Serial.print("file  :");stringdump(lineBuf);
        Serial.println();
        Serial.print("header:");stringdump((char *)CSVHeader);
        Serial.println();
      } else headerOK = true;
    }
  }
  if (sdCardOK && (!headerOK)) {
    // if the file didn't open, try to create one
    Serial.print("Creating a proper file: "); Serial.println(fileName);
    // try to remove
    SD.remove(fileName);
    myFile = SD.open(fileName, FILE_WRITE);
    if (myFile) {
      myFile.write(CSVHeader);
      myFile.write("\n");
      myFile.close();
    }
    else {
      Serial.print("New file cannot be created"); Serial.println(fileName);  
      sdCardOK = false;
    }
  } */


  // File myFile;
  // if (sdCardOK) {
  //   snprintf_P(lineBuf, CSVMessageLen, 
  //         CSVMessage,
  //         1, 0, 0, 0, 0, 1234, 5678, 123456789, millis()
  //   );
  //   Serial.print("Data:"); Serial.println(lineBuf);
  //   // Serial.print("Str :"); stringdump(lineBuf);
  //   myFile = SD.open(fileName, FILE_WRITE);
  //   if (myFile) {
  //     myFile.write(lineBuf, strlen(lineBuf));
  //     myFile.close();
  //   } 

  // while (true) {
  //   Serial.print("Analog0 : "); Serial.print(analogRead(A0));
  //   Serial.print("   Analog1 : "); Serial.println(analogRead(A1));
  //   delay(250);
  // }

}

void loop()
{ 
  int scrollDelay;
  bool first_run = true;
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
  state_type former_state = current_state;

  while (true) {
    if (!buf.isEmpty())
      {
        current_state = key_logic(former_state);
      }  
    if (current_state != former_state) {
      // Serial.print("Old / New State: "); 
      // Serial.print(former_state);
      // Serial.print("->");
      // Serial.println(current_state);
      former_state = current_state;
    }

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

      first_run = true;
    } 
    else if ( (current_state == random_waiting_for_release) ) {
      if (first_run) {
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
        #ifndef DEBUG_02
        tft.fillCircle(150-ledSize/2, 0+ledSize/2, ledSize/2, TFT_RED);
        tft.drawCircle(150-ledSize/2, 0+ledSize/2, ledSize/2, TFT_WHITE);
        loop_i = 0;

        for (unsigned long i=0; i<=top_pos; i+=gWidth*steps) {
          genTone();
          tft.pushImage(lx, ly, gWidth, gHeight, ptr_start+i);
          tft.pushImage(rx, ry, gWidth, gHeight, ptr_stop -i);
          delay(scrollDelay);
        }
        #endif
        first_run = false;
      }
      #ifndef DEBUG_02

      if (loop_i <= top_pos) {
        genTone();
        tft.pushImage(lx, ly, gWidth, gHeight, ptr_start+loop_i);
        tft.pushImage(rx, ry, gWidth, gHeight, ptr_stop -loop_i);
        loop_i+=gWidth*steps;
        delay(scrollDelay);
      } else {
        loop_i = 0;
      }

      #endif
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
      #ifndef DEBUG_02
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
      #endif

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

      current_state = random_display;      
      key_clean();      
    }
    else if (current_state == random_display) {
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
      drawBarChart(left_and_Right);
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
      current_state = wait_in_statistics;
      key_clean();
    }
    else if (current_state == wait_in_statistics) {
        delay(50);
        // scrollText1.scroll(-1);
        // scrollText1.pushSprite(15, sRY+sHi+sXY, 0, 0, rBl, rBh);
    }
    else if (current_state == before_waiting_for_press) {
      scrollText1.deleteSprite();
      tft.pushImage(0, 0, counterWidth, counterHeight, counter);
      current_state = random_waiting_for_press;
      delay(100);
      duration1 = millis()+analogRead(analog1);
      key_clean();
    }
  }
}