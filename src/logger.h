/* cSpell:disable */
#pragma once
#ifndef LOGER_H
#define LOGER_H

#include "Arduino.h"
#include "SdFat.h"
// Select for SD card reader
const uint8_t SD_CS_PIN = 17;

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(4))
// #define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(4))


// name of the file on SD card to store results
#define fileName "dice.csv"

extern boolean sdCardOK;

const char CSVHeader[]  PROGMEM = "1.NewGame, 2.LeftRandom, 3.RightRandom, 4.LeftDelay, 5.RightDelay, 6.RandomSeedL, 7.RandomSeedR, 8.Time";
const char CSVMessage[] PROGMEM = "%ld,%ld,%ld,%010lu,%010lu,%010lu,%010lu,%010lu\n";
#define CSVMessageLen 128 


#define numberVariables 8
typedef unsigned long variableTab_t[numberVariables];

#define diceSize 6
#define sumDiceSize 11
typedef unsigned long statTab_t[diceSize];
typedef unsigned long statTabS_t[sumDiceSize];
typedef struct STAT {
    unsigned long allDur;
    unsigned long currentDur;
    unsigned long numberGames;
    unsigned long numberDraws;
    bool inputError;
    unsigned long lineNumberError;
} stat_t;

extern statTabS_t statTabSg;
extern statTab_t  statTabLg;
extern statTab_t  statTabRg;
  
extern stat_t myStat;

bool myLogger(SdFat32 &myCard);
bool saveResult(uint32_t var1, uint32_t var2, uint32_t var3, uint32_t var4, uint32_t var5, uint32_t var6, uint32_t var7, uint32_t var8);
bool readVals(File32 &f, long &v1, long &v2);
bool readLine(File32 &f, char* line, size_t maxLen);
bool readResult();

#endif