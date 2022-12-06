/* cSpell:disable */
#ifndef SETTINGS_H
#define SETTINGS_H

#define screenSaverTimeout 180000UL
//#define screenSaverTimeout 2000UL

#define ScreenSaverAnimationDelay 250000U

// how to slow down the real
const char SlowDownEl = 10;
unsigned int slowdown[SlowDownEl][2]={  // lines to move in one stepo, delay in ms
{ 8, 10}, // 0 10
{ 8, 15}, // 1 15
{ 7, 20}, // 2
{ 7, 25}, // 3
{ 6, 30}, // 4
{ 6, 35}, // 5  
{ 6, 40}, // 6
{ 1,  0}, // 7
{ 1,  0}, // 8
{ 5,  1}  // 9 SlowDownEl-1
};

#endif