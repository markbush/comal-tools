#include "ParseOps.h"
#include "../logger/Logger.h"

#include <cstdio>

using namespace std::string_literals;

static Logger logger {"ParseOps"s};

static char ascii[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

std::string hexCode(uint8_t op) {
  return ""s + ascii[(op&0xf0)/16] + ascii[op&0xf];;
}

std::string toHex(std::vector<uint8_t>& line, size_t& from, size_t to) {
  std::string code = ""s;
  while (from < to) {
    code += " " + hexCode(line[from++]);
  }
  return code;
}

std::string text(std::vector<uint8_t>& line, size_t& from, size_t to) {
  std::string code = ""s;
  while (from < to) {
    auto& c = line[from++];
    if (c > 128) {
      code += (char)(c-128);
    } else if (c>=65 && c<=90) {
      code += (char)(c+32);
    } else if (c < 32) {
      code += "\""+std::to_string(c)+"\"";
    } else {
      code += (char)c;
    }
  }
  return code;
}

static void constByte(std::vector<std::string>& stack, std::vector<uint8_t>& line, size_t& pos) {
  uint8_t valByte = line[pos++];
  stack.push_back(std::to_string(valByte));
}

static void constReal(std::vector<std::string>& stack, std::vector<uint8_t>& line, size_t& pos) {
  // TODO decode floating point
  int exponent = line[pos++];
  if (exponent == 0) {
    stack.push_back("0"s);
    return;
  }
  exponent -= 0x81;
  uint8_t first = line[pos++];
  int sign = 1;
  if (first > 0x7f) {
    sign = -1;
  } else {
    first |= 0x80;
  }
  long double mantissa = first;
  mantissa = (mantissa*256)+line[pos++];
  mantissa = (mantissa*256)+line[pos++];
  mantissa = (mantissa*256)+line[pos++];
  long double multiplier = 1ll<<(31-exponent);
  mantissa = mantissa*sign/multiplier;
  char buf[64];
  snprintf(buf, 64, "%Lg", mantissa);
  std::string result(buf, strlen(buf));
  stack.push_back(result);
}

static void constInt(std::vector<std::string>& stack, std::vector<uint8_t>& line, size_t& pos) {
  uint16_t valInt = line[pos++]*256;
  valInt += line[pos++];
  stack.push_back(std::to_string(valInt));
}

static void constIntRev(std::vector<std::string>& stack, std::vector<uint8_t>& line, size_t& pos) {
  uint16_t valInt = line[pos++];
  valInt += line[pos++]*256;
  stack.push_back(std::to_string(valInt));
}

static void constString(std::vector<std::string>& stack, std::vector<uint8_t>& line, size_t& pos) {
  uint8_t strLen = line[pos++];
  stack.push_back("\""+text(line, pos, pos+strLen)+"\"");
}

static void constByteString(std::vector<std::string>& stack, std::vector<uint8_t>& line, size_t& pos) {
  stack.push_back("\""+text(line, pos, pos+1)+"\"");
}

static void varReal(std::vector<std::string>& stack, std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& pos) {
  size_t key = line[pos++];
  key += line[pos++]*256;
  stack.push_back(names[key]);
}

static void varInt(std::vector<std::string>& stack, std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& pos) {
  size_t key = line[pos++];
  key += line[pos++]*256;
  stack.push_back(names[key]+"#");
}

static void varString(std::vector<std::string>& stack, std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& pos) {
  size_t key = line[pos++];
  key += line[pos++]*256;
  stack.push_back(names[key]+"$");
}

static void dimensions(std::vector<std::string>& stack, std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& pos) {
  size_t value = line[pos++];
  stack.push_back(""s);
}

static void group(std::vector<std::string>& stack) {
  if (stack.size() < 1) return;
  std::string expr = stack.back(); stack.pop_back();
  stack.push_back("("+expr+")");
}

static void indentStack(std::vector<std::string>& stack) {
  if (stack.size() < 1) return;
  std::string expr = stack.back(); stack.pop_back();
  stack.push_back(" "+expr);
}

static void func(std::vector<std::string>& stack, std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& pos) {
  uint8_t op = line[pos++];
  switch (op) {
    case 0x1f: stack.push_back("KEY$"s); break;
    default: break;
  }
}

static void func1(std::vector<std::string>& stack, std::string funcName) {
  if (stack.size() < 1) return;
  std::string expr = stack.back(); stack.pop_back();
  stack.push_back(funcName+"("+expr+")");
}

static void binaryOp(std::vector<std::string>& stack, std::string op) {
  if (stack.size() < 2) return;
  std::string expr2 = stack.back(); stack.pop_back();
  std::string expr1 = stack.back(); stack.pop_back();
  stack.push_back(expr1+op+expr2);
}

static void binaryOpReversed(std::vector<std::string>& stack, std::string op) {
  if (stack.size() < 2) return;
  std::string expr1 = stack.back(); stack.pop_back();
  std::string expr2 = stack.back(); stack.pop_back();
  stack.push_back(expr1+op+expr2);
}

static void add(std::vector<std::string>& stack) {
  binaryOp(stack, "+"s);
}

static void incr(std::vector<std::string>& stack) {
  binaryOp(stack, ":+"s);
  indentStack(stack);
}

static void subtract(std::vector<std::string>& stack) {
  binaryOp(stack, "-"s);
}

static void decr(std::vector<std::string>& stack) {
  binaryOp(stack, ":-"s);
  indentStack(stack);
}

static void multiply(std::vector<std::string>& stack) {
  binaryOp(stack, "*"s);
}

static void divide(std::vector<std::string>& stack) {
  binaryOp(stack, "/"s);
}

static void abs(std::vector<std::string>& stack) {
  func1(stack, "ABS"s);
}

static void sin(std::vector<std::string>& stack) {
  func1(stack, "SIN"s);
}

static void keyword(std::vector<std::string>& stack, std::string op) {
  stack.push_back(op);
}

static void rem(std::vector<std::string>& stack, std::vector<uint8_t>& line, size_t& pos) {
  stack.push_back(" //"+text(line, pos, line.size()));
}

static void dim(std::vector<std::string>& stack) {
  stack.push_back(" DIM"s);
}

static void null(std::vector<std::string>& stack) {
  stack.push_back(" NULL"s);
}

static void use(std::vector<std::string>& stack) {
  stack.push_back(" USE "s);
}

static void end(std::vector<std::string>& stack) {
  stack.push_back(" END "s);
}

static void import(std::vector<std::string>& stack) {
  stack.push_back(" IMPORT"s);
}

static void endImport(std::vector<std::string>& stack) {
  if (stack.size() < 3) return;
  std::string args = ""s;
  while (stack.size()>2) {
    std::string item = stack.back(); stack.pop_back();
    args = item+args;
    if (stack.size()>2) args = ","+args;
  }
  std::string flags = stack.back(); stack.pop_back();
  std::string op = stack.back(); stack.pop_back();
  stack.push_back(op+" "+args);
}

static void whileLoop(std::vector<std::string>& stack) {
  stack.push_back(" WHILE "s);
}

static void whileDo(std::vector<std::string>& stack) {
  if (stack.size() < 1) return;
  std::string forContent = stack.back(); stack.pop_back();
  stack.push_back(forContent+" DO");
}

static void whileEnd(std::vector<std::string>& stack) {
  binaryOp(stack, ""s);
}

static void proc(std::vector<std::string>& stack) {
  stack.push_back(" PROC"s);
}

static void procFlags(std::vector<std::string>& stack, std::vector<uint8_t>& line, size_t& pos) {
  stack.push_back(toHex(line, pos, pos+3));
}

static void procEnd(std::vector<std::string>& stack, std::vector<uint8_t>& line, size_t& pos) {
  if (stack.size() < 4) return;
  std::string args = ""s;
  while (stack.size()>4) {
    std::string item = stack.back(); stack.pop_back();
    args = item+args;
    if (stack.size()>4) args = ","+args;
  }
  std::string flags = stack.back(); stack.pop_back();
  std::string offset = stack.back(); stack.pop_back();
  std::string name = stack.back(); stack.pop_back();
  std::string op = stack.back(); stack.pop_back();
  stack.push_back(op+" "+name+"("+args+")");
  pos += 4;
}

static void endProc(std::vector<std::string>& stack) {
  if (stack.empty()) {
    stack.push_back(" ENDPROC"s);
    return;
  }
  if (stack.size() < 2) return;
  std::string var = stack.back(); stack.pop_back();
  std::string result = stack.back(); stack.pop_back();
  if (var.size()>0) {
    result += " "+var;
  }
  stack.push_back(result);
}

static void comma(std::vector<std::string>& stack) {
  stack.push_back(","s);
}

static void exec(std::vector<std::string>& stack) {
  if (stack.size() < 2) return;
  std::string args = ""s;
  while (stack.size()>1) {
    std::string item = stack.back(); stack.pop_back();
    args = item+args;
  }
  std::string op = stack.back(); stack.pop_back();
  stack.push_back(" "+op+"("+args+")");
}

static void forStart(std::vector<std::string>& stack) {
  stack.push_back(" FOR"s);
}

static void forFrom(std::vector<std::string>& stack) {
  if (stack.size() < 4) return;
  std::string start = stack.back(); stack.pop_back();
  std::string offset = stack.back(); stack.pop_back();
  std::string var = stack.back(); stack.pop_back();
  std::string opening = stack.back(); stack.pop_back();
  stack.push_back(opening+" "+var+":="+start);
}

static void forTo(std::vector<std::string>& stack) {
  if (stack.size() < 2) return;
  std::string end = stack.back(); stack.pop_back();
  std::string opening = stack.back(); stack.pop_back();
  stack.push_back(opening+" TO "+end);
}

static void forStep(std::vector<std::string>& stack) {
  if (stack.size() < 2) return;
  std::string step = stack.back(); stack.pop_back();
  std::string opening = stack.back(); stack.pop_back();
  stack.push_back(opening+" STEP "+step);
}

static void forDo(std::vector<std::string>& stack) {
  if (stack.size() < 1) return;
  std::string forContent = stack.back(); stack.pop_back();
  stack.push_back(forContent+" DO");
}

static void forEnd(std::vector<std::string>& stack) {
  binaryOp(stack, ""s);
}

static void endFor(std::vector<std::string>& stack) {
  if (stack.empty()) {
    stack.push_back(" ENDFOR"s);
    return;
  }
  if (stack.size() < 3) return;
  std::string offset = stack.back(); stack.pop_back();
  std::string var = stack.back(); stack.pop_back();
  std::string result = stack.back(); stack.pop_back();
  if (var.size()>0) {
    result += " "+var;
  }
  stack.push_back(result);
}

static void dimString(std::vector<std::string>& stack) {
  if (stack.size() < 4) return;
  std::string stringSize = stack.back(); stack.pop_back();
  std::string dimensions = stack.back(); stack.pop_back();
  std::string var = stack.back(); stack.pop_back();
  std::string keyword = stack.back(); stack.pop_back();
  stack.push_back(keyword+" "+var+dimensions+" OF "+stringSize);
}

static void assign(std::vector<std::string>& stack) {
  binaryOpReversed(stack, ":="s);
  indentStack(stack);
}

static void equals(std::vector<std::string>& stack) {
  binaryOp(stack, "="s);
}

static void compose(std::vector<std::string>& stack) {
  binaryOp(stack, ";"s);
}

static void handleOp(std::vector<std::string>& stack, std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& pos) {
  uint8_t op = line[pos++];
  switch (op) {
    case 0x00: rem(stack, line, pos); break;
    case 0x01: constReal(stack, line, pos); break;
    case 0x02: constInt(stack, line, pos); break;
    case 0x03: constString(stack, line, pos); break;
    case 0x04: varReal(stack, names, line, pos); break;
    case 0x05: varInt(stack, names, line, pos); break;
    case 0x06: varString(stack, names, line, pos); break;
    case 0x07: varReal(stack, names, line, pos); break;
    case 0x08: varInt(stack, names, line, pos); break;
    case 0x09: varString(stack, names, line, pos); break;
    case 0x0d: comma(stack); break;
    case 0x0e: endProc(stack); varReal(stack, names, line, pos); endProc(stack); break;
    case 0x13: exec(stack); break;
    case 0x15: exec(stack); break;
    case 0x18: exec(stack); break;
    case 0x1a: varReal(stack, names, line, pos); break;
    case 0x23: divide(stack); break;
    case 0x24: multiply(stack); break;
    case 0x27: add(stack); break;
    case 0x28: add(stack); break;
    case 0x29: subtract(stack); break;
    case 0x2d: equals(stack); break;
    case 0x3d: incr(stack); break;
    case 0x3e: incr(stack); break;
    case 0x40: decr(stack); break;
    case 0x41: decr(stack); break;
    case 0x42: compose(stack); break;
    case 0x43: keyword(stack, "TRUE"s); break;
    case 0x44: keyword(stack, "FALSE"s); break;
    case 0x47: group(stack); break;
    case 0x48: abs(stack); break;
    case 0x57: sin(stack); break;
    case 0x70: proc(stack); varReal(stack, names, line, pos); constIntRev(stack, line, pos); procFlags(stack, line, pos); break;
    case 0x71: null(stack); break;
    case 0x72: varReal(stack, names, line, pos); break;
    case 0x73: varInt(stack, names, line, pos); break;
    case 0x74: varString(stack, names, line, pos); break;
    case 0x7f: procEnd(stack, line, pos); break;
    case 0x82: forStart(stack); varReal(stack, names, line, pos); constIntRev(stack, line, pos); break;
    case 0x83: forStart(stack); varInt(stack, names, line, pos); constIntRev(stack, line, pos); break;
    case 0x84: forFrom(stack); break;
    case 0x85: forTo(stack); break;
    case 0x86: forStep(stack); break;
    case 0x87: forDo(stack); break;
    case 0x88: forDo(stack); break;
    case 0x89: forEnd(stack); break;
    case 0x8a: endFor(stack); varReal(stack, names, line, pos); constIntRev(stack, line, pos); endFor(stack); break;
    case 0x8b: endFor(stack); varInt(stack, names, line, pos); constIntRev(stack, line, pos); endFor(stack); break;
    case 0x8c: break; // end of DIM
    case 0x8f: dim(stack); varString(stack, names, line, pos); dimensions(stack, names, line, pos); break;
    case 0x93: dimString(stack); break;
    case 0x97: whileLoop(stack); break;
    case 0x99: whileDo(stack); break;
    case 0x9a: whileEnd(stack); break;
    case 0x9f: end(stack); break;
    case 0xcd: constByteString(stack, line, pos); break;
    case 0xce: constByte(stack, line, pos); break;
    case 0xcf: break; // blank line
    case 0xd4: use(stack); varReal(stack, names, line, pos); break;
    case 0xeb: import(stack); constIntRev(stack, line, pos); break;
    case 0xef: endImport(stack); break;
    case 0xf8: varString(stack, names, line, pos); assign(stack); break;
    case 0xfa: varReal(stack, names, line, pos); assign(stack); break;
    case 0xfb: varInt(stack, names, line, pos); assign(stack); break;
    case 0xff: func(stack, names, line, pos); break;
    default:
      break;
  }
}

std::vector<uint8_t> nextLine(std::vector<uint8_t>::iterator& start) {
  std::vector<uint8_t> line {};
  line.push_back(*start++);
  line.push_back(*start++);
  uint8_t length = *start++ - 3;
  while (length-- > 0) {
    line.push_back(*start++);
  }
  return line;
}

std::string decodeLine(std::map<size_t,std::string>& names, size_t& indent, std::vector<uint8_t> line) {
  static size_t codePos = 2048;
  std::string memLoc = hexCode(codePos&0xff)+" "+hexCode((codePos&0xff00)/256);
  size_t lineSize = line.size();
  std::vector<std::string> stack {};
  std::string code = ""s;
  uint16_t lineNum = ((line[0]*256) + line[1]) - 10000;
  if (lineNum < 1000) code += "0";
  if (lineNum < 100) code += "0";
  if (lineNum < 10) code += "0";
  code += std::to_string(lineNum);
  size_t debugPos = 2;
  logger.info("[{}] {}{}", memLoc, code, toHex(line, debugPos, lineSize));
  size_t pos = 2;
  uint8_t startOp = line[pos];
  uint8_t endOp = line[lineSize-1];
  if (startOp == 0x0e || startOp == 0x8a || startOp == 0x8b) {
    if (indent>0) indent--;
  }
  code += std::string(indent, ' ');
  while (pos < lineSize) {
    handleOp(stack, names, line, pos);
  }
  if (startOp == 0x70 || startOp == 0x82 || startOp == 0x83) {
    if (endOp != 0x89) indent++;
  }

  for (auto& part : stack) {
    code += part;
  }
  code += "\n";
  codePos += lineSize+1;
  return code;
}
