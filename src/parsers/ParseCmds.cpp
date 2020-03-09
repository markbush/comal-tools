#include "ParseCmds.h"

#include "../logger/Logger.h"

#include <algorithm>
#include <regex>

using namespace std::string_literals;

static Logger logger {"ParseCmds"s};

static void encodeNull(std::vector<uint8_t>& bytes, size_t& pos) {
  bytes.push_back(0x71);
  pos += 3;
}

static const std::regex regexNull("^null$", std::regex_constants::icase);
static void encodeExpr(std::vector<uint8_t>& bytes, NameTable& names, std::string expr) {
  if (expr.size() == 0) return;
  std::smatch match;
  uint8_t first = expr[0];
  if (first == '-') {
    std::string remaining = expr.substr(1, expr.size()-1);
    encodeExpr(bytes, names, remaining);
    bytes.push_back(0x21);
    return;
  } else if (first>='0' && first<='9') {
    //double valueD = std::stod(expr); // TODO encode reals
    uint16_t valueI = std::stoi(expr);
    uint8_t valHigh = valueI/256;
    uint8_t valLow = valueI&0xff;
    if (valHigh == 0x00) {
      bytes.push_back(0xce);
      bytes.push_back(valLow);
    } else {
      bytes.push_back(0x02);
      bytes.push_back(valHigh);
      bytes.push_back(valLow);
    }
  } else if (first == '"') { // TODO encode string
    bytes.push_back(0xcd);
    bytes.push_back(0x65);
  } else if (std::regex_search(expr, match, regexNull)) {
    bytes.push_back(0x71);
  } else {
    size_t varNameLen = expr.size();
    uint8_t last = expr[varNameLen-1];
    if (last == '$' || last == '#') varNameLen--;
    std::string varName = expr.substr(0, varNameLen);
    uint8_t type = 0x10;
    if (last=='#') type++;
    if (last=='$') type += 2;
    uint16_t varOffset = names.offset(varName, type);
    uint8_t offsetHigh = varOffset/256;
    uint8_t offsetLow = varOffset&0xff;
    bytes.push_back(0x04);
    bytes.push_back(offsetLow);
    bytes.push_back(offsetHigh);
  }
}

static void skipDigits(std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  while (pos<lineEnd && line[pos]>='0' && line[pos]<='9') pos++;
}

static void skipSpace(std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  while (pos<lineEnd && line[pos]==' ') pos++;
}

static void skipToSemiColon(std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  while (pos<lineEnd && line[pos]!=';') pos++;
}

static void assignDelim(std::vector<uint8_t>& bytes) {
  bytes.push_back(0x42);
}

static const std::regex regexAssign("^([a-zA-Z']+)(#|\\$)?:[=+-]");
static bool checkAssign(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  skipSpace(line, pos);
  size_t lineEnd = line.size();
  std::smatch match;
  std::string possibleAssign = line.substr(pos, lineEnd-pos);
  if (!std::regex_search(possibleAssign, match, regexAssign)) return false;

  std::string varName = match[1];
  uint8_t type = 0x10;
  if (match[2]=="#") type++;
  if (match[2]=="$") type += 2;
  names.add(varName, type);
  std::string matched = match[0];
  pos += matched.size();

  std::string expr = line.substr(pos, lineEnd-pos);
  encodeExpr(bytes, names, expr);
  skipToSemiColon(line, pos); // TODO parse assigned expression
  switch (type) {
    case 0x11: bytes.push_back(0xfb); break; // int
    case 0x12: bytes.push_back(0xf8); break; // string
    default: bytes.push_back(0xfa); break; // real
  }
  uint16_t offset = names.offset(varName, type);
  uint8_t offsetLow = offset&0xff;
  uint8_t offsetHigh = offset/256;
  bytes.push_back(offsetLow); bytes.push_back(offsetHigh);
  return true;
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

static const std::regex regexForStep("(.+)\\s+step\\s+(.+)", std::regex_constants::icase);
static void encodeFor(std::smatch& match, std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  std::string varName = match[1];
  std::string varType = match[2];
  std::string fromExpr = match[3];
  std::string toExpr = match[4];
  std::string stepExpr = ""s;
  std::string doExpr = match[6];

  std::smatch stepMatch;
  if (std::regex_search(toExpr, stepMatch, regexForStep)) {
    stepExpr = stepMatch[2];
    toExpr = stepMatch[1];
  }

  uint8_t type = 0x10;
  if (varType=="#") type++;
  names.add(varName, type);
  if (type == 0x10) {
    bytes.push_back(0x82);
  } else {
    bytes.push_back(0x83);
  }
  uint16_t offset = names.offset(varName, type);
  uint8_t offsetHigh = offset/256;
  uint8_t offsetLow = offset&0xff;
  bytes.push_back(offsetLow);
  bytes.push_back(offsetHigh);
  size_t offsetPos = bytes.size();
  bytes.push_back(0x00); bytes.push_back(0x00); // TODO target of line after ENDFOR
  encodeExpr(bytes, names, fromExpr);
  bytes.push_back(0x84);
  encodeExpr(bytes, names, toExpr);
  bytes.push_back(0x85);
  if (stepExpr != "") {
    encodeExpr(bytes, names, stepExpr);
    bytes.push_back(0x86);
  }
  if (doExpr == "") {
    bytes.push_back(0x87); // multi line for
  } else {
    bytes.push_back(0x88); // one line for
    size_t exprPos = 0;
    if (!checkAssign(bytes, names, doExpr, exprPos)) {
      encodeExpr(bytes, names, doExpr);
    }
    bytes.push_back(0x89);
  }
  pos = line.size();
}

static void encodeEndFor(std::smatch& match, std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  std::string varName = match[1];
  std::string varType = match[2];
  uint8_t type = 0x10;
  if (varType=="#") type++;
  names.add(varName, type);
  if (type == 0x10) {
    bytes.push_back(0x8a);
  } else {
    bytes.push_back(0x8b);
  }
  uint16_t offset = names.offset(varName, type);
  uint8_t offsetHigh = offset/256;
  uint8_t offsetLow = offset&0xff;
  bytes.push_back(offsetLow);
  bytes.push_back(offsetHigh);
  bytes.push_back(0x00); bytes.push_back(0x00); // TODO target of line after FOR
  std::string matched = match[0];
  pos += matched.size();
}

static const std::regex regexEndFor("^endfor\\s+([a-zA-Z']*)(#|)", std::regex_constants::icase);
static void encodeE(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::string lineToMatch = line.substr(pos-1, lineEnd-pos+1);
  std::smatch match;

  if (std::regex_search(lineToMatch, match, regexEndFor)) { encodeEndFor(match, bytes, names, line, pos); return; }
}

static const std::regex regexFor("^for\\s+([a-zA-Z']+)(#|):=\\s*(.+)\\s+to\\s+(.+)\\s+do(\\s*(.*))", std::regex_constants::icase);
static void encodeF(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::string lineToMatch = line.substr(pos-1, lineEnd-pos+1);
  std::smatch match;

  if (std::regex_search(lineToMatch, match, regexFor)) { encodeFor(match, bytes, names, line, pos); return; }
}

static void encodeN(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::string lineToMatch = line.substr(pos-1, lineEnd-pos+1);
  std::smatch match;

  if (std::regex_search(lineToMatch, match, regexNull)) { encodeNull(bytes, pos); return; }
}

static void encodeCmd(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  if (checkAssign(bytes, names, line, pos)) return;

  uint8_t c = line[pos++];
  if (c>=97 && c<=122) c -= 32;
  switch (c) {
    case '/': rem(bytes, line, pos); break;
    case ';': if (checkAssign(bytes, names, line, pos)) assignDelim(bytes); break;
    case 'E': encodeE(bytes, names, line, pos); break;
    case 'F': encodeF(bytes, names, line, pos); break;
    case 'N': encodeN(bytes, names, line, pos); break;
    default: pos = lineEnd; break;
  }
}

std::vector<uint8_t> encode(uint16_t& autoLineNum, NameTable& names, std::string line) {
  size_t lineEnd = line.size();
  size_t pos = 0;
  skipSpace(line, pos); // space to line number or command
  if (line[pos]>='0' && line[pos]<='9') {
    autoLineNum = std::stoi(line);
  } else {
    autoLineNum += 10;
  }
  uint16_t lineNum = autoLineNum+10000; // offset used by COMAL 2
  uint8_t lineNumHigh = lineNum/256;
  uint8_t lineNumLow = lineNum&0xff;
  std::vector<uint8_t> lineBytes {lineNumHigh, lineNumLow, 0x00};

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
