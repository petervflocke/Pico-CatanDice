/* cSpell:disable */

// Input / output pins
#define LED_PIN LED_BUILTIN

// Rotary switch
#define pinBut 7
#define pinA   8
#define pinB   9
#define analog0 26
#define analog1 27

// Select for SD card reader
#define PIN_SD_CS 17

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


const char MessageText1[] PROGMEM = "Press START to continue. [%d] L: %d %.0f%%  |  R: %d %.0f%%  |  S: %d  %.0f%%";
#define MessageLen 128 /*length of message*/
