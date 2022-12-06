/* cSpell:disable */
#include "Arduino.h"
#include <string.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "graphics.h"
#include "statehandler.h"
#include "logger.h"

// table with pointers of digits from reel data
// const unsigned short *space = (unsigned short *)reel;
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

  #define sRX   7 /* stat box upper x*/
  #define sRY  10 /* stat box uper y*/
  #define sWi 143 /* stat box width */
  #define sHi  90 /* stat box high */
  #define sXi  10 /* place for bottom margin */
  #define sXY  10 /* margin of 0,0 from boarders*/
  #define sBw   5 /* bar width */
  #define sBm   5 /* gap between bars */
  #define sBwlr 6 /* bar width */
  #define sBmlr 11 /* gap between bars */


#define BirdD 200

birdStruc tabBirds[] = {
  {29,  1, bird00_width, bird00_height, bird00},
  {46,  0, bird01_width, bird01_height, bird01},
  {59,  3, bird02_width, bird02_height, bird02},
  {74, 10, bird03_width, bird03_height, bird03},
  {84, 23, bird04_width, bird04_height, bird04},
  {69, 39, bird05_width, bird05_height, bird05},
  {32, 50, bird06_width, bird06_height, bird06},
  { 2, 33, bird07_width, bird07_height, bird07},
  { 0, 16, bird08_width, bird08_height, bird08},
  {13,  6, bird09_width, bird09_height, bird09},
};

void drawInfoBox(TFT_eSPI &tft) {
  tft.pushImage(0, 0, counterWidth, counterHeight, counter);
  tft.fillRoundRect(sRX+4, sRY+4, sWi, sHi, 5, TFT_BLACK);
  tft.fillRoundRect(sRX  , sRY  , sWi, sHi, 5, TFT_WHITE);
  tft.drawRoundRect(sRX-1, sRY-1, sWi, sHi, 5, TFT_GOLD);
  tft.drawRoundRect(sRX  , sRY  , sWi, sHi, 5, TFT_GOLD);
  tft.drawRoundRect(sRX+1, sRY+1, sWi, sHi, 5, TFT_GOLD);
}

void drawBarChart(TFT_eSPI &tft, u_int32_t TabL[], u_int32_t TabR[], u_int32_t TabS[], u_int32_t drawCnt, BarChartType statTab, char *titel) {

  drawInfoBox(tft);
  tft.drawFastVLine(sRX+2*sXY, sRY+sXY, sHi-2*sXY-sXi, TFT_BLACK);
  tft.setTextColor(TFT_BLUE,TFT_WHITE);
  tft.setTextFont(2);
  tft.drawCentreString(titel, 80, sRY+sHi-2*sXY+sBm, 1);
  tft.setFreeFont(TT1);
  tft.setTextSize(1);

  u_int32_t maxProb = 0;  
  if (statTab  == left_and_Right) {
    for (int i=0; i<6; i++) {
      maxProb = (maxProb < TabL[i]? TabL[i] : maxProb);
      maxProb = (maxProb < TabR[i]? TabR[i] : maxProb);
    }
  } else if (statTab == sum_of_both) {
    for (int i=0; i<11; i++) {
      maxProb = (maxProb < TabS[i]? TabS[i] : maxProb);
    }
  }
  maxProb = 100*maxProb/drawCnt;
  // Serial.print("maxProb :"); Serial.println(maxProb);
  for (int i=maxProb; i > 0; i-=maxProb/5) {
    tft.drawNumber(i, sRX+sXY-(i==100?2:0), sRY+sXY+(((maxProb-i)*(sHi-3*sXY-sXi)/maxProb))-2);
    tft.drawFastHLine(sRX+2*sXY+1, sRY+sXY+((maxProb-i)*(sHi-3*sXY-sXi)/maxProb), sWi-3*sXY-1, TFT_LIGHTGREY);
  }
  tft.drawFastHLine(sRX+sXY, sRY+sHi-2*sXY-sXi, sWi-2*sXY, TFT_BLACK);
  if (statTab  == left_and_Right) {
    for (int i=0; i<6; i++) {
      int barHight=100*TabL[i]/drawCnt;
      barHight = map(barHight, 0, maxProb, 0, sHi-3*sXY-sXi);
      tft.fillRect (sRX+2*sXY+sBwlr+i*(sBwlr+sBmlr), sRY+sHi-2*sXY-barHight-sXi, sBwlr, barHight, TFT_GREEN);
      
      barHight=100*TabR[i]/drawCnt;
      barHight = map(barHight, 0, maxProb, 0, sHi-3*sXY-sXi);
      tft.fillRect (sRX+2*sXY+sBwlr+i*(sBwlr+sBmlr)+sBwlr+1, sRY+sHi-2*sXY-barHight-sXi, sBwlr, barHight, TFT_RED);

      tft.drawNumber(i+1, sRX+2*sXY+sBwlr+i*(sBwlr+sBmlr)+sBwlr-3, sRY+sHi-2*sXY+sBm-sXi);
      tft.drawFastVLine(sRX+2*sXY+sBwlr+i*(sBwlr+sBmlr)+sBwlr*2+1, sRY+sHi-2*sXY+1-sXi, sBm, TFT_LIGHTGREY);
    }    
  } else if (statTab == sum_of_both) {
    for (int i=0; i<11; i++) {
      int barHight=100*TabS[i]/drawCnt;
      // Serial.print(i+2); Serial.print(" : "); Serial.print(barHight);
      barHight = map(barHight, 0, maxProb, 0, sHi-3*sXY-sXi);
      // Serial.print(" : "); Serial.println(barHight);
      tft.fillRect (sRX+2*sXY+sBw+i*(sBw+sBm), sRY+sHi-2*sXY-barHight-sXi, sBw, barHight, TFT_BLUE);
      tft.drawNumber(i+2, sRX+2*sXY+sBw+i*(sBw+sBm), sRY+sHi-2*sXY+sBm-sXi);
    }
  }
}

void drawChart(TFT_eSPI &tft, u_int32_t* statTabX, u_int32_t statCnt, int maxIndex, int deltaIndex) {
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


void drawInfoText(TFT_eSPI &tft, int rndl, int rndr, unsigned long cnt, SdFat32 &myCard) {

  #define DY 13
  #define DYS sRY
  #define FONTn 1
  char lineBuf[MessageLen];
  int16_t dy = 0;
  unsigned long currentTime = (millis() - myStat.currentDur)/1000;
  unsigned long allDuration = myStat.allDur / 1000;

  drawInfoBox(tft);  
  tft.setTextColor(TFT_BLACK,TFT_WHITE);
  tft.setTextFont(FONTn);
  tft.setTextSize(1);
  snprintf_P(lineBuf, MessageLen, InfoText1, cnt, rndl, rndr, rndl+rndr);
  tft.drawString(lineBuf, sRX+sXY, DYS+dy, FONTn);
  dy += DY;
  snprintf_P(lineBuf, MessageLen, InfoText2, numberOfHours(currentTime), numberOfMinutes(currentTime), numberOfSeconds(currentTime));
  tft.drawString(lineBuf, sRX+sXY, DYS+dy, FONTn);
  dy += DY;
  snprintf_P(lineBuf, MessageLen, InfoText3, elapsedDays(allDuration), numberOfHours(allDuration), numberOfMinutes(allDuration), numberOfSeconds(allDuration));
  tft.drawString(lineBuf, sRX+sXY, DYS+dy, FONTn);
  dy += DY;
  snprintf_P(lineBuf, MessageLen, InfoText4, myStat.numberGames);
  tft.drawString(lineBuf, sRX+sXY, DYS+dy, FONTn);
  dy += DY;
  snprintf_P(lineBuf, MessageLen, InfoText5, myStat.numberDraws);
  tft.drawString(lineBuf, sRX+sXY, DYS+dy, FONTn);
  dy += DY;  
  if (sdCardOK) {
    long lFreeKB = myCard.vol()->freeClusterCount();
    lFreeKB *= myCard.vol()->sectorsPerCluster()/2;
    snprintf_P(lineBuf, MessageLen, InfoText8, lFreeKB);
  } else if (myStat.inputError) {
    snprintf_P(lineBuf, MessageLen, InfoText7, myStat.lineNumberError);
  } else {
    snprintf_P(lineBuf, MessageLen, InfoText6);
  }
  tft.drawString(lineBuf, sRX+sXY, DYS+dy, FONTn);


/*
 https://forum.arduino.cc/t/arduino-timer-convert-millis-to-days-hours-min/42323
*/
}

void showSDError(TFT_eSPI &tft) {
  tft.pushImage(SDX, SDY, sdcardiconWidth, sdcardiconHeight, sdcardicon, TFT_WHITE);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("SD Card", SDXt, SDYt, 2);
  tft.setTextColor(TFT_BLUE);
  tft.fillRect(SDX1e, SDYe, 45, 16, sdcardback);
  tft.setTextColor(TFT_RED);
  tft.fillRect(SDX1e, SDYe, 45, 16, sdcardback);
  tft.drawString("ERROR", SDX1e, SDYe, 2);
}

void restoreBack(TFT_eSPI &tft, unsigned int posX, unsigned int posY, unsigned int b_width, unsigned int b_hight, unsigned int s_width, const unsigned short *sourceBuf) {
  /*
  put cut out bacground image starting at posX, posY
  in size = b_hight x b_width
  background image s_width and ptr is needed
  */

  #define maxBufSize catanWidth*32 /* max size 32 lines from full width background*/

  unsigned short backBuf[maxBufSize]; // "sprite" max size screen width * 32 lines / replace with dynamic malloc?
  unsigned int addr = 0;
  unsigned int startIdx = posY*s_width + posX;
  
  if (b_width*b_hight < maxBufSize) { // check for too big sprite, which does not fit into backBuf
    for (unsigned short sprY = 0; sprY < b_hight; sprY++) {
      for (unsigned short sprX = 0; sprX < b_width; sprX++) {
        backBuf[addr++] = sourceBuf[startIdx+sprX];
      }
      startIdx += catanWidth;
    }
    tft.pushImage(posX, posY, b_width, b_hight, backBuf);  
  }
}

/* void nextBird(TFT_eSPI &tft, unsigned int BirdX, unsigned int BirdY, unsigned int i) {

  unsigned short birdBuf[MAX_BIRD_SIZE];
  unsigned int addr = 0;
  unsigned int startIdx = (BirdY + tabBirds[i].by)*catanWidth + BirdX + tabBirds[i].bx;

  tft.pushImage(BirdX + tabBirds[i].bx, BirdY + tabBirds[i].by, tabBirds[i].b_width, tabBirds[i].b_height, tabBirds[i].bird, TFT_WHITE);
  delay(BirdD);
   for (unsigned short sprY = 0; sprY < tabBirds[i].b_height; sprY++) {
    for (unsigned short sprX = 0; sprX < tabBirds[i].b_width; sprX++) {
      birdBuf[addr++] = catan[startIdx+sprX];
    }
    startIdx += catanWidth;
  }
  tft.pushImage(BirdX + tabBirds[i].bx, BirdY + tabBirds[i].by, tabBirds[i].b_width, tabBirds[i].b_height, birdBuf); 
} */


void drawTime(TFT_eSPI &tft, unsigned int ttX,  unsigned int ttY, unsigned long ttd) {

  #define DY 13
  #define FONTn 1
  char lineBuf[MessageLen];

  unsigned long gameDuration = (millis() - myStat.currentDur) / 1000;
  unsigned long drawDuration = (millis() - ttd) / 1000;
  int16_t dy = 0;

  restoreBack(tft, ttX, ttY, catanWidth-ttX-5, 22, catanWidth, catan);
  // tft.fillRect(ttX, ttY, catanWidth-ttX-5, 22, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(FONTn);
  tft.setTextSize(1);
  snprintf_P(lineBuf, MessageLen, InfoText9, numberOfHours(gameDuration), numberOfMinutes(gameDuration), numberOfSeconds(gameDuration));
  tft.drawString(lineBuf, ttX, ttY+1, FONTn);
  dy += DY;
  snprintf_P(lineBuf, MessageLen, InfoText10, numberOfHours(drawDuration), numberOfMinutes(drawDuration), numberOfSeconds(drawDuration));
  tft.drawString(lineBuf, ttX, ttY+dy, FONTn);
}
