/* cSpell:disable */
#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "Arduino.h"
#include "counter.h"
#include "reelpic.h"

// define posistion of the graphic elements
// position of the left and right reel of digits
#define lx 26
#define ly 24
#define rx 90
#define ry 24


// Position of status LED on screen
#define ledSize  16
#define ledPosX 135
#define ledPosY   0

// Result windows position
#define rRX  30 /* result box upper x*/
#define rRY  15 /* result box uper y*/
#define rWi  96 /* result box width */
#define rHi  62 /* result box high */
#define mHi  47 /* minimal digit size */
#define dSt  18 /* digit start line*/
#define rBl 128 /* scroll bar lenghth*/
#define rBh  20 /* scroll bar high */

// Display statistics
#define sRX   7 /* stat box upper x*/
#define sRY  20 /* stat box uper y*/
#define sWi 143 /* stat box width */
#define sHi  80 /* stat box high */
#define sXY  10 /* margin of 0,0 from boarders*/
#define sBw   5 /* bar width */
#define sBm   5 /* gap between bars */


typedef enum {
  left_and_Right,  
  sum_of_both
} BarChartType;

extern const unsigned short *space;
extern const unsigned short *qq;
extern const unsigned short *digits[];
extern const unsigned short *ptr;
extern const unsigned short *ptr_start;
extern const unsigned short *ptr_stop;
extern const unsigned int top_pos;

const char MessageText1[] PROGMEM = "Press ^ to continue or rotate <--> for statistics. [%d] L: %d %.0f%%  |  R: %d %.0f%%  |  S: %d  %.0f%%";
#define MessageLen 128 /*length of message*/

void drawBarChart(TFT_eSPI &tft, u_int32_t statTabL[], u_int32_t statTabR[], u_int32_t statTabS[], u_int32_t statCnt, BarChartType statTab, char* titel);
void drawChart(TFT_eSPI &tft, u_int32_t* statTabX, int maxIndex, int deltaIndex);
void showSDError(TFT_eSPI &tft);

#endif