/* cSpell:disable */
#include "Arduino.h"
#include <SD.h>
#include "logger.h"

void stringdump(char* str) {
  Serial.print("=>");
  while (*str) {
    Serial.print(*str, HEX);
    Serial.print(",");
    str += 1;
  }
}

bool readLine(File &f, char* line, size_t maxLen) {
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

bool readVals(char* line, long &v1, long &v2) {
  char *ptr, *str;
  v1 = strtol(line, &ptr, 10);
  if (ptr == line) return false;  // bad number if equal
  while (*ptr) {
    if (*ptr++ == ',') break;
  }
  v2 = strtol(ptr, &str, 10);
  return str != ptr;  // true if number found
}

bool myLogger(SDClass &myCard) {
  File myFile;
  boolean sdCardOK=true;
  bool headerOK = false;
  char lineBuf[CSVMessageLen];
  
  myFile = myCard.open(fileName, FILE_READ);
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
  if (sdCardOK && (!headerOK)) {
    // if the file didn't open, try to create one
    Serial.print("Creating a proper file: "); Serial.println(fileName);
    // try to remove
    myCard.remove(fileName);
    myFile = myCard.open(fileName, FILE_WRITE);
    if (myFile) {
      myFile.write(CSVHeader);
      myFile.write("\n");
      myFile.close();
    }
    else {
      Serial.print("New file cannot be created"); Serial.println(fileName);  
      sdCardOK = false;
    }
  }
  return sdCardOK;
}

bool saveResult(SDClass &myCard, uint32_t var1, uint32_t var2, uint32_t var3, uint32_t var4, uint32_t var5, uint32_t var6, uint32_t var7, uint32_t var8) {
  File myFile;
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
  myFile = myCard.open(fileName, FILE_WRITE);
  if (myFile) {
    myFile.write(lineBuf, strlen(lineBuf));
    myFile.close();
    return true;
  } else return false;
}