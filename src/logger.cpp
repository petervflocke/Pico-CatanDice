#include "Arduino.h"
#include <SD.h>


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

bool readVals(File &f, long &v1, long &v2) {
  char line[40], *ptr, *str;
  if (!readLine(f, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  v1 = strtol(line, &ptr, 10);
  if (ptr == line) return false;  // bad number if equal
  while (*ptr) {
    if (*ptr++ == ',') break;
  }
  v2 = strtol(ptr, &str, 10);
  return str != ptr;  // true if number found
}