/* cSpell:disable */
// name of the file on SD card to store results
#define fileName  "dice.csv"

const char CSVHeader[]  PROGMEM = "1.NewGame, 2.LeftRandom, 3.RightRandom, 4.LeftDelay, 5.RightDelay, 6.RandomSeedL, 7.RandomSeedR, 8.Time";
const char CSVMessage[] PROGMEM = "%ld,%ld,%ld,%010lu,%010lu,%010lu,%010lu,%010lu\n";
#define CSVMessageLen 128 

bool myLogger(SDClass &myCard);
bool saveResult(SDClass &myCard, uint32_t var1, uint32_t var2, uint32_t var3, uint32_t var4, uint32_t var5, uint32_t var6, uint32_t var7, uint32_t var8);
bool readVals(File &f, long &v1, long &v2);
bool readLine(File &f, char* line, size_t maxLen);