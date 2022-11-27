/* cSpell:disable */
#include "Arduino.h"
#include "logger.h"
#include <string.h>


statTabS_t statTabSg = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
statTab_t  statTabLg = {0, 0, 0, 0, 0, 0};
statTab_t  statTabRg = {0, 0, 0, 0, 0, 0};
  
stat_t myStat = {
  .allDur=0,
  .currentDur=0,
  .numberGames=0,
  .numberDraws=0,
  .inputError=false,
  .lineNumberError=0
};

void stringdump(char* str) {
  Serial.print("=>");
  while (*str) {
    Serial.print(*str, HEX);
    Serial.print(",");
    str += 1;
  }
}

bool readLine(File32 &f, char* line, size_t maxLen) {
  for (size_t n = 0; n < maxLen; n++) {
    int c = f.read();
    if ( c < 0 && n == 0) return false;  // EOF
    if (c < 0 || c == '\n') {
      line[n] = 0;
      return true;
    }
    line[n] = c;
  }
  return false; // line too long
}

bool readVals(char* line, variableTab_t variableTab) {
  char *ptr;
  int i;
  ptr = line;
  i = 0;
  while (*ptr && i < numberVariables) {
    variableTab[i] = strtoul(ptr, &ptr, 10);
// Serial.print(variableTab[i]);Serial.print(",");
    i ++;
    while (*ptr) {
      if (*ptr++ == ',') break;
    }
  }
  return (!(*ptr || i != 8));
}

bool myLogger(SdFat32 &myCard) {
  File32 myFile;
  boolean sdCardOK=true;
  bool headerOK = false;
  char lineBuf[CSVMessageLen];
  
  // myFile = myCard.open(fileName, FILE_READ);
  if (myFile.open(fileName, FILE_READ)) {
    Serial.print("File OK1: "); Serial.println(fileName);
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
  if (sdCardOK && (!headerOK)) {
    // if the file didn't open, try to create one
    Serial.print("Creating a proper file: "); Serial.println(fileName);
    // try to remove
    myCard.remove(fileName);
    // myFile = myCard.open(fileName, FILE_WRITE);
    if (myFile.open(fileName, FILE_WRITE)) {
      myFile.write(CSVHeader);
      myFile.write("\n");
      myFile.close();
    }
    else {
      Serial.print("New file cannot be created: "); Serial.println(fileName);  
      sdCardOK = false;
    }
  }
  return sdCardOK;
}

bool saveResult(uint32_t var1, uint32_t var2, uint32_t var3, uint32_t var4, uint32_t var5, uint32_t var6, uint32_t var7, uint32_t var8) {
  File32 myFile;
  char lineBuf[CSVMessageLen];
  snprintf_P(lineBuf, CSVMessageLen, 
        CSVMessage,
        var1, var2, var3, var4, var5, var6, var7, var8
  );
  // Serial.print("Input:");
  // Serial.print(var1);Serial.print(",");
  // Serial.print(var2);Serial.print(",");
  // Serial.print(var3);Serial.print(",");
  // Serial.print(var4);Serial.print(",");
  // Serial.print(var5);Serial.print(",");
  // Serial.print(var6);Serial.print(",");
  // Serial.print(var7);Serial.print(",");
  // Serial.print(var8);Serial.println();
  Serial.print("Data :"); Serial.println(lineBuf);
  // Serial.print("Str :"); stringdump(lineBuf);
  // myFile = myCard.open(fileName, FILE_WRITE);
  if (myFile.open(fileName, FILE_WRITE)) {
    myFile.write(lineBuf, strlen(lineBuf));
    myFile.close();
    return true;
  } else return false;
}

bool readResult() {
  File32 myFile;
  char lineBuf[CSVMessageLen];
  variableTab_t variableTab;
  
  long ln=1;
  unsigned long firstDur = 0;
  unsigned long lastDur = 0;
  myStat.inputError = false;
  myStat.lineNumberError = 0;

  // myFile = myCard.open(fileName, FILE_READ);
  if (myFile.open(fileName, FILE_READ)) {
    readLine(myFile, lineBuf, CSVMessageLen);
    Serial.print(ln); Serial.print(": "); Serial.println(lineBuf);
    while (myFile.available() && !myStat.inputError) {
      ln += 1;
      Serial.print(ln); Serial.print(": ");
      readLine(myFile, lineBuf, CSVMessageLen);
      if (readVals(lineBuf, variableTab)) {
        for (int j=0; j < numberVariables; j++) {
          Serial.print(variableTab[j]); Serial.print(", ");
        }
        if (variableTab[0] == 1) {
          myStat.allDur += lastDur - firstDur;
          myStat.numberGames += 1;
          firstDur = variableTab[7];
        } else {
          if (variableTab[1] <= 6 && variableTab[2] <= 6) {
            statTabLg[variableTab[1]-1]++;
            statTabRg[variableTab[2]-1]++;
            statTabSg[variableTab[1]+variableTab[2]-2]++;
            myStat.numberDraws++;
            lastDur = variableTab[7];
          } else {
            myStat.inputError = true;
            myStat.lineNumberError = ln;
            Serial.println(": Data Error");
          }
        }
        Serial.println();
      } else {
        Serial.println(": Data Error");
        myStat.inputError = true;
        myStat.lineNumberError = ln;        
      }
    }
    myFile.close();
  }
  return myStat.inputError;
}