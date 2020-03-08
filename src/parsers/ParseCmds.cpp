#include "ParseCmds.h"

static void skipDigits(std::string& line, size_t& pos) {
  while (line[pos]>='0' && line[pos]<='9') pos++;
}

static void skipSpace(std::string& line, size_t& pos) {
  while (line[pos]==' ') pos++;
}

std::vector<uint8_t> encode(std::vector<std::string>& names, std::string line) {
  uint16_t lineNum = std::stoi(line)+10000;
  uint8_t lineNumHigh = lineNum/256;
  uint8_t lineNumLow = lineNum&0xff;
  std::vector<uint8_t> lineBytes {lineNumHigh, lineNumLow, 0x00};

  size_t pos = 0;
  skipDigits(line, pos);
  skipSpace(line, pos);

  if (lineBytes.size() == 3) {
    lineBytes.push_back(0xcf);
  }
  lineBytes[2] = lineBytes.size();
  return lineBytes;
}
