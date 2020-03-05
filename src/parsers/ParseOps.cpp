#include "ParseOps.h"

using namespace std::string_literals;

static char ascii[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static std::string hexCode(uint8_t op) {
  return " "s + ascii[(op&0xf0)/16] + ascii[op&0xf];;
}

static std::string toHex(std::vector<uint8_t>& line, size_t& from, size_t to) {
  std::string code = "";
  while (from < to) {
    code += hexCode(line[from++]);
  }
  return code;
}

static std::string indentFor(size_t indent) {
  return std::string(indent, ' ');
}

static std::string text(std::vector<uint8_t>& line, size_t& from, size_t to) {
  std::string code = "";
  while (from < to) {
    auto& c = line[from];
    if (c > 128) {
      code += (char)(c-128);
    } else if (c>=65 && c<=90) {
      code += (char)(c+32);
    } else {
      code += (char)c;
    }
    from++;
  }
  return code;
}

static std::string rem(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " //";
  code += text(line, from, line.size());
  return code;
}

static std::string endProc(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " ENDPROC";
  code += toHex(line, from, line.size());
  return code;
}

static std::string proc(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " PROC";
  code += toHex(line, from, line.size());
  return code;
}

static std::string dimString(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " DIM xxx$ OF nn";
  code += toHex(line, from, line.size());
  return code;
}

static std::string parseVal(std::vector<uint8_t>& line, size_t& from, size_t to) {
  uint8_t op = line[from++];
  if (op == 0xce) {
    uint8_t valByte = line[from++];
    return std::to_string(valByte);
  }
  if (op == 0x02) {
    uint16_t valInt = line[from++]*256;
    valInt += line[from++];
    return std::to_string(valInt);
  }
  if (op == 0x01) {
    // TODO decode floating point
    return "["+toHex(line, from, from+5)+"]";
  }
  return toHex(line, from, to);
}

static std::string forLoop(std::string suffix, std::vector<uint8_t>& line, size_t& from) {
  size_t lineSize = line.size();
  std::string code = " FOR [";
  code += toHex(line, from, from+4);
  code += "]"+suffix+":=";
  size_t pos = from;
  while (pos<lineSize && line[pos]!=0x84) pos++;
  code += parseVal(line, from, pos) + " TO ";
  from++; pos = from;
  while (pos<lineSize && line[pos]!=0x85) pos++;
  code += parseVal(line, from, pos);
  from++; pos = from;
  while (pos<lineSize && line[pos]!=0x86) pos++;
  if (pos<lineSize) {
    code += " STEP " + parseVal(line, from, pos);
    from++;
  }
  code += " DO";
  pos = from;
  uint8_t doOp = line[from++];
  if (doOp == 0x88) {
    if (line[from]==0x71 && line[from+1]==0x89) {
      code += " NULL";
      from += 2;
    }
  }
  code += toHex(line, from, line.size());
  return code;
}

static std::string forReal(std::vector<uint8_t>& line, size_t& from) {
  return forLoop("", line, from);
}

static std::string forInteger(std::vector<uint8_t>& line, size_t& from) {
  return forLoop("#", line, from);
}

static std::string endFor(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " ENDFOR [";
  code += toHex(line, from, from+4);
  code += "]";
  return code;
}

static std::string endForReal(std::vector<uint8_t>& line, size_t& from) {
  return endFor(line, from);
}

static std::string endForInteger(std::vector<uint8_t>& line, size_t& from) {
  return endFor(line, from);
}

static std::string whileCond(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " WHILE";
  code += toHex(line, from, line.size());
  return code;
}

static std::string end(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " END";
  code += toHex(line, from, line.size());
  return code;
}

bool eof(std::vector<uint8_t>::iterator& start) {
  return (start[2]==0);
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

std::string decodeLine(size_t& indent, std::vector<uint8_t> line) {
  size_t lineSize = line.size();
  std::string code = "";
  uint16_t lineNum = ((line[0]*256) + line[1]) - 10000;
  if (lineNum < 1000) code += "0";
  if (lineNum < 100) code += "0";
  if (lineNum < 10) code += "0";
  code += std::to_string(lineNum);
  size_t pos = 2;
  while (pos < lineSize) {
    uint8_t op = line[pos++];
    uint8_t endOp = line[lineSize-1];
    if (op == 0x00) {
      code += indentFor(indent) + rem(line, pos);
    } else if (op == 0x0e) {
      indent -= 2;
      code += indentFor(indent) + endProc(line, pos);
    } else if (op == 0x70) {
      code += indentFor(indent) + proc(line, pos);
      indent += 2;
    } else if (op == 0x82) {
      code += indentFor(indent) + forReal(line, pos);
      if (endOp != 0x89) indent += 2;
    } else if (op == 0x83) {
      code += indentFor(indent) + forInteger(line, pos);
      if (endOp != 0x89) indent += 2;
    } else if (op == 0x8a) {
      indent -= 2;
      code += indentFor(indent) + endForReal(line, pos);
    } else if (op == 0x8b) {
      indent -= 2;
      code += indentFor(indent) + endForInteger(line, pos);
    } else if (op == 0x8f) {
      code += indentFor(indent) + dimString(line, pos);
    } else if (op == 0x97) {
      code += indentFor(indent) + whileCond(line, pos);
      if (endOp != 0x9a) indent += 2;
    } else if (op == 0x9F) {
      code += indentFor(indent) + end(line, pos);
    } else {
      code += indentFor(indent) + hexCode(op) + toHex(line, pos, lineSize);
    }
  }

  code += "\n";
  return code;
}
