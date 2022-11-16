/* cSpell:disable */
#include "Arduino.h"
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "graphics.h"

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

void drawBarChart(TFT_eSPI &tft, u_int32_t TabL[], u_int32_t TabR[], u_int32_t statTabS[], u_int32_t statCnt, BarChartType statTab) {
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
      maxProb = (maxProb < TabL[i]? TabL[i] : maxProb);
      maxProb = (maxProb < TabR[i]? TabR[i] : maxProb);
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
      int barHight=100*TabL[i]/statCnt;
      barHight = map(barHight, 0, maxProb, 0, sHi-3*sXY);
      tft.fillRect (sRX+2*sXY+sBwlr+i*(sBwlr+sBmlr), sRY+sHi-2*sXY-barHight, sBwlr, barHight, TFT_GREEN);
      
      barHight=100*TabR[i]/statCnt;
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


