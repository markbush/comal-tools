#include "ParseCmds.h"

#include "../logger/Logger.h"

#include <regex>

using namespace std::string_literals;

static Logger logger {"ParseCmds"s};

static const std::regex assignRe("^([a-zA-Z]+)(#|\\$)?:=");

static void skipDigits(std::string& line, size_t& pos) {
  while (line[pos]>='0' && line[pos]<='9') pos++;
}

static void skipSpace(std::string& line, size_t& pos) {
  while (line[pos]==' ') pos++;
}

static void assign(std::vector<uint8_t>& bytes, std::vector<std::string>& names, std::string& line, size_t& pos, bool addDelim) {
  skipSpace(line, pos);
  if (addDelim) {
    bytes.push_back(0x42);
  }
}

static bool checkAssign(std::vector<uint8_t>& bytes, std::vector<std::string>& names, std::string& line, size_t& pos) {
  std::smatch match;
  std::string possibleAssign {};
  for (size_t i=pos; i<line.size(); i++) possibleAssign += line[i];
  if (std::regex_search(possibleAssign, match, assignRe)) {
    std::string varName = match[1];
    logger.info("MATCH: {}", varName);
  }
  return false;
}

static void rem(std::vector<uint8_t>& bytes, std::string& line, size_t& pos) {
  if (pos >= line.size()) return;
  if (line[pos++] != '/') return;
  bytes.push_back(0x00); // REM
  while (pos < line.size()) {
    uint8_t c = line[pos++];
    if (c>=97 && c<=122) {
      bytes.push_back(c-32);
    } else if (c>=65 && c<=90) {
      bytes.push_back(c+128);
    } else {
      bytes.push_back(c);
    }
  }
}

static void encodeCmd(std::vector<uint8_t>& bytes, std::vector<std::string>& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  if (checkAssign(bytes, names, line, pos)) return;

  uint8_t c = line[pos++];
  if (c>=97 && c<=122) c -= 32;
  switch (c) {
    case '/': rem(bytes, line, pos); break;
    case ';': assign(bytes, names, line, pos, true); break;
    default: break;
  }
  pos = lineEnd;
}

std::vector<uint8_t> encode(std::vector<std::string>& names, std::string line) {
  size_t lineEnd = line.size();
  uint16_t lineNum = std::stoi(line)+10000; // offset used by COMAL 2
  uint8_t lineNumHigh = lineNum/256;
  uint8_t lineNumLow = lineNum&0xff;
  std::vector<uint8_t> lineBytes {lineNumHigh, lineNumLow, 0x00};

  size_t pos = 0;
  skipDigits(line, pos); // line number

  while (pos < lineEnd) {
    skipSpace(line, pos); // space to command
    encodeCmd(lineBytes, names, line, pos);
  }

  if (lineBytes.size() == 3) {
    lineBytes.push_back(0xcf); // blank line
  }
  lineBytes[2] = lineBytes.size();
  return lineBytes;
}
