/* cSpell:disable */
#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "Arduino.h"
#include "counter.h"
#include "reelpic.h"

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

void drawBarChart(TFT_eSPI &tft, u_int32_t statTabL[], u_int32_t statTabR[], u_int32_t statTabS[], u_int32_t statCnt, BarChartType statTab);
void drawChart(TFT_eSPI &tft, u_int32_t* statTabX, int maxIndex, int deltaIndex);

#endif