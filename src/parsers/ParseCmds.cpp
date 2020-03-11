#include "ParseCmds.h"
#include "ExprTokeniser.h"

#include "../logger/Logger.h"

#include <algorithm>
#include <regex>

using namespace std::string_literals;

static Logger logger {"ParseCmds"s};

static void encodeNull(std::vector<uint8_t>& bytes, size_t& pos) {
  bytes.push_back(0x71);
  pos += 3;
}

static void showTokens(std::vector<std::string>& tokens) {
  std::string tokenString {};
  for (auto& token : tokens) {
    tokenString += token + " ";
  }
  logger.info("PARSE: {}", tokenString);
}

static uint8_t toLower(uint8_t c) {
  if (c>=65 && c<=90) return c+32;
  return c;
}

static std::string toLower(std::string value) {
  std::string result {};
  for (auto& c : value) {
    result += toLower(c);
  }
  return result;
}

static uint8_t encodeChar(uint8_t c) {
  if (c>=97 && c<=122) {
    return c - 32;
  } else if (c>=65 && c<=90) {
    return c + 128;
  } else {
    return c;
  }
}

static void encodeDouble(std::vector<uint8_t>& bytes, double value) {
  uint8_t exponent = 0xa0;
  while (value > 0xffffffffl) {
    value /= 2;
    exponent++;
  }
  while (value < 0x80000000l) {
    value *= 2;
    exponent--;
  }
  uint32_t mantissa = value;
  bytes.push_back(0x01);
  bytes.push_back(exponent);
  bytes.push_back((mantissa/0x1000000)&0x7f);
  bytes.push_back((mantissa/0x10000)&0xff);
  bytes.push_back((mantissa/0x100)&0xff);
  bytes.push_back(mantissa&0xff);
}

static void encodeNumber(std::vector<uint8_t>& bytes, std::string token) {
  double valueD = std::stod(token);
  uint16_t valueI = std::stoi(token);
  if (valueD != valueI) {
    encodeDouble(bytes, valueD);
    return;
  }
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
}

static void encodeBool(std::vector<uint8_t>& bytes, std::string token) {
  std::string boolVal = toLower(token);
  if (boolVal == "true") bytes.push_back(0x43);
  if (boolVal == "false") bytes.push_back(0x44);
}
// abs|ord|atn|chr\\$|cos|eof|exp|int|len|log|rnd|sgn|sin|spc\\$|sqr|str\\$|tan
static void encodeFunc(std::vector<uint8_t>& bytes, std::string token) {
  std::string func = toLower(token);
  if (func == "abs") bytes.push_back(0x48);
  else if (func == "ord") bytes.push_back(0x49);
  else if (func == "atn") bytes.push_back(0x4a);
  else if (func == "chr$") bytes.push_back(0x4b);
  else if (func == "cos") bytes.push_back(0x4c);
  else if (func == "eod") bytes.push_back(0x5c);
  else if (func == "eof") bytes.push_back(0x5d);
  else if (func == "esc") bytes.push_back(0x4d);
  else if (func == "exp") bytes.push_back(0x4e);
  else if (func == "int") bytes.push_back(0x4f);
  else if (func == "key$") { bytes.push_back(0xff); bytes.push_back(0x1f); }
  else if (func == "len") bytes.push_back(0x51);
  else if (func == "log") bytes.push_back(0x52);
  else if (func == "rnd") bytes.push_back(0x53);
  else if (func == "rnd(") bytes.push_back(0x54);
  else if (func == "sgn") bytes.push_back(0x56);
  else if (func == "sin") bytes.push_back(0x57);
  else if (func == "spc$") bytes.push_back(0x58);
  else if (func == "sqr") bytes.push_back(0x59);
  else if (func == "str$") bytes.push_back(0xea);
  else if (func == "tan") bytes.push_back(0x5a);
  else if (func == "zone") bytes.push_back(0x45);
  else if (func == "status$") bytes.push_back(0xde);
  else if (func == "time") bytes.push_back(0x5b);
}

static void encodeOp(std::vector<uint8_t>& bytes, std::string token, std::string prev) {
  if (prev[0] == 'S' || prev[prev.size()-1] == '$') { // must be string op
    logger.info("string op");
    switch (token[0]) {
      case '+': bytes.push_back(0x28); break;
      case '=': bytes.push_back(0x2d); break;
      case '<':
        if (token.size() == 1) {
          bytes.push_back(0x2b);
        } else if (token[1] == '=') {
          bytes.push_back(0x2f);
        } else {
          bytes.push_back(0x33); // <>
        }
        break;
      case '>':
        if (token.size() == 1) {
          bytes.push_back(0x31);
        } else {
          bytes.push_back(0x35); // >=
        }
        break;
    }
  }
  switch (toLower(token[0])) {
    case '+': bytes.push_back(0x27); break;
    case '-': bytes.push_back(0x29); break;
    case '*': bytes.push_back(0x24); break;
    case '/': bytes.push_back(0x23); break;
    case '^': bytes.push_back(0x22); break;
    case 'd': bytes.push_back(0x25); break; // div
    case 'm': bytes.push_back(0x26); break; // mod
    case 'i': bytes.push_back(0x36); break; // in
    case '=': bytes.push_back(0x2c); break;
    case '<':
      if (token.size() == 1) {
        bytes.push_back(0x2a);
      } else if (token[1] == '=') {
        bytes.push_back(0x2e);
      } else {
        bytes.push_back(0x32); // <>
      }
      break;
    case '>':
      if (token.size() == 1) {
        bytes.push_back(0x30);
      } else {
        bytes.push_back(0x34); // >=
      }
      break;
    case 'a': bytes.push_back(0x38); break; // and
    case 'o': bytes.push_back(0x39); break; // or
    case 'n': bytes.push_back(0x37); break; // not
  }
}

static void encodeVar(std::vector<uint8_t>& bytes, NameTable& names, std::string token) {
  size_t varNameLen = token.size();
  if (varNameLen == 0) return;
  uint8_t last = token[varNameLen-1];
  if (last == '$' || last == '#') varNameLen--;
  std::string varName = token.substr(0, varNameLen);
  uint8_t type = 0x10;
  uint8_t op = 0x04;
  if (last=='#') { type++; op = 0x05; }
  if (last=='$') { type += 2; op = 0x06; }
  names.add(varName, type);
  uint16_t varOffset = names.offset(varName, type);
  uint8_t offsetHigh = varOffset/256;
  uint8_t offsetLow = varOffset&0xff;
  bytes.push_back(op);
  bytes.push_back(offsetLow);
  bytes.push_back(offsetHigh);
}

static void encodeUnary(std::vector<uint8_t>& bytes, std::string token) {
  if (token == "+") bytes.push_back(0x20);
  if (token == "-") bytes.push_back(0x21);
  if (toLower(token) == "not") bytes.push_back(0x37);
}

static void encodeString(std::vector<uint8_t>& bytes, std::string token) {
  if (token.size() == 0) {
    bytes.push_back(0xcd);
    bytes.push_back(0x00);
  } else if (token.size() == 1) {
    bytes.push_back(0xcd);
    bytes.push_back(encodeChar(token[0]));
  } else {
    bytes.push_back(0x03);
    bytes.push_back(token.size());
    for (uint8_t c : token) bytes.push_back(encodeChar(c));
  }
}

static void encodeGroup(std::vector<uint8_t>& bytes) {
  bytes.push_back(0x47);
}

static void encodeComma(std::vector<uint8_t>& bytes, std::string token) {
  if (token == "comma") {
    bytes.push_back(0x55);
  } else {
    bytes.push_back(0x0d);
  }
}

static void encodeToken(std::vector<uint8_t>& bytes, NameTable& names, std::string token, std::string prev) {
  size_t tokenSize = token.size()-1;
  if (tokenSize < 0) return;
  uint8_t type = token[0];
  std::string value = token.substr(1, tokenSize);
  switch (type) {
    case 'N': encodeNumber(bytes, value); break;
    case 'B': encodeBool(bytes, value); break;
    case 'F': encodeFunc(bytes, value); break;
    case 'O': encodeOp(bytes, value, prev); break;
    case 'V': encodeVar(bytes, names, value); break;
    case 'U': encodeUnary(bytes, value); break;
    case 'S': encodeString(bytes, value); break;
    case 'G': encodeGroup(bytes); break;
    case 'C': encodeComma(bytes, value); break;
    default: logger.error("UNKNOWN TOKEN <{}>", token);
  }
}

static const std::regex regexNull("^null$", std::regex_constants::icase);
static void encodeExpr(std::vector<uint8_t>& bytes, NameTable& names, std::string expr) {
  if (expr.size() == 0) return;
  std::smatch match;
  if (std::regex_search(expr, match, regexNull)) {
    bytes.push_back(0x71);
    return;
  }
  ExprTokeniser tokeniser {expr};
  std::vector<std::string> tokens = tokeniser.getTokens();
  showTokens(tokens);
  std::string prev {"X"s};
  for (auto& token : tokens) {
    encodeToken(bytes, names, token, prev);
    prev = token;
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

static const std::regex regexAssign("^([a-z][0-9a-z']*)(#|\\$)?:([=+-])");
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
  uint16_t offset = names.offset(varName, type);
  uint8_t offsetLow = offset&0xff;
  uint8_t offsetHigh = offset/256;
  std::string assignType = match[3];
  std::string matched = match[0];
  pos += matched.size();

  std::string expr = line.substr(pos, lineEnd-pos);
  if (assignType == "=") {
    encodeExpr(bytes, names, expr);
    switch (type) {
      case 0x11: bytes.push_back(0xfb); break; // int
      case 0x12: bytes.push_back(0xf8); break; // string
      default: bytes.push_back(0xfa); break; // real
    }
    bytes.push_back(offsetLow); bytes.push_back(offsetHigh);
  } else {
    switch (type) {
      case 0x11: bytes.push_back(0x08); break; // int
      case 0x12: bytes.push_back(0x09); break; // string
      default: bytes.push_back(0x07); break; // real
    }
    bytes.push_back(offsetLow); bytes.push_back(offsetHigh);
    encodeExpr(bytes, names, expr);
    switch (type) {
      case 0x11: // int
        if (assignType == "+") {
          bytes.push_back(0x3e);
        } else {
          bytes.push_back(0x41);
        }
        break;
      case 0x12: // string
          bytes.push_back(0x3f);
        break;
      default: // real
        if (assignType == "+") {
          bytes.push_back(0x3d);
        } else {
          bytes.push_back(0x40);
        }
        break;
    }
  }
  skipToSemiColon(line, pos);
  return true;
}

static const std::regex regexExec("^(exec\\s+)?([a-z][0-9a-z']*)(\\(([^\\)]+)\\))?");
static bool checkExec(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::smatch match;
  std::string possibleExec = line.substr(pos, lineEnd-pos);
  if (!std::regex_search(possibleExec, match, regexExec)) return false;

  std::string fullMatch = match[0];
  pos += fullMatch.size();
  std::string funcName = match[2];
  std::string argString = match[4];

  uint8_t type = 0x14;
  names.add(funcName, type);

  ExprTokeniser tokeniser {argString};
  std::vector<std::string> tokens = tokeniser.getTokens();
  showTokens(tokens);
  if (tokens.size() > 0) {
    bytes.push_back(0x1a);
  } else {
    bytes.push_back(0x81);
  }

  uint16_t offset = names.offset(funcName, type);
  uint8_t offsetHigh = offset/256;
  uint8_t offsetLow = offset&0xff;
  bytes.push_back(offsetLow);
  bytes.push_back(offsetHigh);

  if (tokens.size() > 0) {
    std::string prev {"X"s};
    for (auto& token : tokens) {
      if (token == "Ccomma") {
        bytes.push_back(0x0d);
      } else {
        encodeToken(bytes, names, token, prev);
      }
      prev = token;
    }
    bytes.push_back(0x15);
  }

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

static void encodeStringDim(std::smatch& match, std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  std::string varName = match[1];
  std::string size = match[2];
  uint8_t type = 0x12;
  names.add(varName, type);

  bytes.push_back(0x8f);
  uint16_t offset = names.offset(varName, type);
  uint8_t offsetHigh = offset/256;
  uint8_t offsetLow = offset&0xff;
  bytes.push_back(offsetLow);
  bytes.push_back(offsetHigh);
  bytes.push_back(0x00); // number of dimensions
  encodeExpr(bytes, names, size);
  bytes.push_back(0x93);
  bytes.push_back(0x8c);

  std::string matched = match[0];
  pos += matched.size();
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
  std::string matched = match[0];
  pos += matched.size();
  std::string varPart = match[1];
  if (varPart.size() == 0) { // TODO work out matching FOR to get type
    bytes.push_back(0x8a); // FIXME assuming real
    bytes.push_back(0xff);
    bytes.push_back(0xff);
    bytes.push_back(0x00); bytes.push_back(0x00); // TODO target of line after FOR
    return;
  }
  std::string varName = match[2];
  std::string varType = match[3];
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
}

static std::vector<std::string> findProcParams(std::string paramString) {
  std::vector<std::string> params {};
  size_t pos = 0;
  size_t len = paramString.size();
  std::string param {};
  while (pos < len) {
    param = "";
    while (pos < len && paramString[pos] == ' ') pos++; // skip leading blanks
    while (pos < len && paramString[pos] != ',') {
      param += paramString[pos++];
    }
    pos++; // skip any comma
    params.push_back(param);
  }
  return params;
}

// proc\\s+([a-z][0-9a-z']+)(\\(([^\\)]+)\\))?(\\s*closed)?
static void encodeProc(std::smatch& match, std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  std::string fullMatch = match[0];
  std::string varName = match[1];
  std::string paramString = match[3];
  std::string closed = match[4];

  uint8_t type = 0x14;
  names.add(varName, type);
  bytes.push_back(0x70);

  uint16_t offset = names.offset(varName, type);
  uint8_t offsetHigh = offset/256;
  uint8_t offsetLow = offset&0xff;
  bytes.push_back(offsetLow);
  bytes.push_back(offsetHigh);

  bytes.push_back(0x00); // TODO offset to after ENDPROC
  bytes.push_back(0x00);

  bytes.push_back(0x00);
  bytes.push_back(0x00);

  std::vector<std::string> params = findProcParams(paramString);
  bytes.push_back(params.size());
  for (auto param : params) {
    uint8_t paramType = 0x30;
    uint8_t paramOp = 0x72;
    if (param.size() == 0) continue;
    if (param.size() > 4 && toLower(param.substr(0, 4)) == "ref ") {
      paramType += 0x20;
      paramOp += 3;
      param = param.substr(4, param.size()-4);
      while (param.size() > 0 && param[0] == ' ') {
        param = param.substr(1, param.size()-1);
      }
    }
    uint8_t lastChar = param[param.size()-1];
    if (lastChar == '#') { paramType += 1; paramOp += 1; param = param.substr(0, param.size()-1); }
    if (lastChar == '$') { paramType += 2; paramOp += 2; param = param.substr(0, param.size()-1); }
    names.add(param, paramType);

    bytes.push_back(paramOp);
    uint16_t paramOffset = names.offset(param, paramType);
    uint8_t paramOffsetHigh = paramOffset/256;
    uint8_t paramOffsetLow = paramOffset&0xff;
    bytes.push_back(paramOffsetLow);
    bytes.push_back(paramOffsetHigh);
  }

  if (closed.size() != 0) {
    bytes.push_back(0x7f);
    bytes.push_back(0x00);
    bytes.push_back(0x00);
  } else {
    bytes.push_back(0x7e);
  }
  bytes.push_back(0x00);
  bytes.push_back(0x00);

  pos += fullMatch.size();
}

static void encodeEndProc(std::smatch& match, std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  std::string matched = match[0];
  pos += matched.size();
  std::string varPart = match[1];
  bytes.push_back(0x0e);

  if (varPart.size() == 0) {
    bytes.push_back(0xff);
    bytes.push_back(0xff);
    return;
  }
  std::string varName = match[2];
  uint8_t type = 0x14;
  names.add(varName, type);
  uint16_t offset = names.offset(varName, type);
  uint8_t offsetHigh = offset/256;
  uint8_t offsetLow = offset&0xff;
  bytes.push_back(offsetLow);
  bytes.push_back(offsetHigh);
}

static void encodeStop(std::vector<uint8_t>& bytes, size_t& pos) {
  bytes.push_back(0xa0);
  pos += 4;
}

static void encodeTrap(std::smatch& match, std::vector<uint8_t>& bytes, NameTable& names, size_t& pos) {
  std::string fullLine = match[0];
  std::string type = match[1];

  bytes.push_back(0xbf);
  bytes.push_back(0xc0);
  if (type == "+") {
    bytes.push_back(0xc1);
  } else {
    bytes.push_back(0xc2);
  }
  pos += fullLine.size();
}

static void encodeTime(std::smatch& match, std::vector<uint8_t>& bytes, NameTable& names, size_t& pos) {
  std::string fullLine = match[0];
  std::string expr = match[1];

  encodeExpr(bytes, names, expr);
  bytes.push_back(0xff);
  bytes.push_back(0x31);
  pos += fullLine.size();
}

static void encodeZone(std::smatch& match, std::vector<uint8_t>& bytes, NameTable& names, size_t& pos) {
  std::string fullLine = match[0];
  std::string expr = match[1];

  encodeExpr(bytes, names, expr);
  bytes.push_back(0x46);
  pos += fullLine.size();
}

static const std::regex regexStringDim("^dim\\s+([a-z][0-9a-z']*)\\$\\s+of\\s+(.*)", std::regex_constants::icase);
static void encodeD(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::string lineToMatch = line.substr(pos-1, lineEnd-pos+1);
  std::smatch match;

  if (std::regex_search(lineToMatch, match, regexStringDim)) { encodeStringDim(match, bytes, names, line, pos); return; }
}

static const std::regex regexEndFor("^endfor(\\s+([a-z][0-9a-z']*)(#|))?", std::regex_constants::icase);
static const std::regex regexEndProc("^endproc(\\s+([a-z][0-9a-z']*))?", std::regex_constants::icase);
static void encodeE(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::string lineToMatch = line.substr(pos-1, lineEnd-pos+1);
  std::smatch match;

  if (std::regex_search(lineToMatch, match, regexEndFor)) { encodeEndFor(match, bytes, names, line, pos); return; }
  if (std::regex_search(lineToMatch, match, regexEndProc)) { encodeEndProc(match, bytes, names, line, pos); return; }
}

static const std::regex regexFor("^for\\s+([a-z][0-9a-z']*)(#|):=\\s*(.+)\\s+to\\s+(.+)\\s+do(\\s*(.*))", std::regex_constants::icase);
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

static const std::regex regexProc("^proc\\s+([a-z][0-9a-z']*)(\\(([^\\)]+)\\))?(\\s*closed)?", std::regex_constants::icase);
static void encodeP(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::string lineToMatch = line.substr(pos-1, lineEnd-pos+1);
  std::smatch match;

  if (std::regex_search(lineToMatch, match, regexProc)) { encodeProc(match, bytes, names, line, pos); return; }
}

static const std::regex regexStop("^stop\\b", std::regex_constants::icase);
static void encodeS(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::string lineToMatch = line.substr(pos-1, lineEnd-pos+1);
  std::smatch match;

  if (std::regex_search(lineToMatch, match, regexStop)) { encodeStop(bytes, pos); return; }
}

static const std::regex regexTrap("^trap\\s+esc\\s*(\\+|-)", std::regex_constants::icase);
static const std::regex regexTime("^time\\s*(.+)", std::regex_constants::icase);
static void encodeT(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::string lineToMatch = line.substr(pos-1, lineEnd-pos+1);
  std::smatch match;

  if (std::regex_search(lineToMatch, match, regexTrap)) { encodeTrap(match, bytes, names, pos); return; }
  if (std::regex_search(lineToMatch, match, regexTime)) { encodeTime(match, bytes, names, pos); return; }
}

static const std::regex regexZone("^zone\\s*(.+)", std::regex_constants::icase);
static void encodeZ(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  std::string lineToMatch = line.substr(pos-1, lineEnd-pos+1);
  std::smatch match;

  if (std::regex_search(lineToMatch, match, regexZone)) { encodeZone(match, bytes, names, pos); return; }
}

static void encodeCmd(std::vector<uint8_t>& bytes, NameTable& names, std::string& line, size_t& pos) {
  size_t lineEnd = line.size();
  if (checkAssign(bytes, names, line, pos)) return;
  if (checkExec(bytes, names, line, pos)) return;

  uint8_t c = line[pos++];
  if (c>=97 && c<=122) c -= 32;
  switch (c) {
    case '/': rem(bytes, line, pos); break;
    case ';': if (checkAssign(bytes, names, line, pos)) assignDelim(bytes); break;
    case 'D': encodeD(bytes, names, line, pos); break;
    case 'E': encodeE(bytes, names, line, pos); break;
    case 'F': encodeF(bytes, names, line, pos); break;
    case 'N': encodeN(bytes, names, line, pos); break;
    case 'P': encodeP(bytes, names, line, pos); break;
    case 'S': encodeS(bytes, names, line, pos); break;
    case 'T': encodeT(bytes, names, line, pos); break;
    case 'Z': encodeZ(bytes, names, line, pos); break;
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
