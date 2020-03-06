#include "ParseOps.h"

using namespace std::string_literals;

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
    } else {
      code += (char)c;
    }
  }
  return code;
}

static std::string parseVal(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  uint8_t op = line[from++];
  if (op == 0xce) {
    uint8_t valByte = line[from++];
    return std::to_string(valByte);
  }
  if (op == 0xcd) {
    return "\""+text(line, from, from+1)+"\"";
  }
  if (op == 0x05) {
    size_t key = line[from++];
    key += line[from++]*256;
    return names[key]+"#";
  }
  if (op == 0x04) {
    size_t key = line[from++];
    key += line[from++]*256;
    return names[key];
  }
  if (op == 0x03) {
    uint8_t strLen = line[from++];
    return "\""+text(line, from, from+strLen)+"\"";
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
  return toHex(line, from, line.size());
}

static std::string indentFor(size_t indent) {
  return std::string(indent, ' ');
}

static std::string rem(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " //"s;
  code += text(line, from, line.size());
  return code;
}

static std::string endProc(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  std::string code = " ENDPROC"s;
  size_t key = line[from++];
  key += line[from++]*256;
  if (key < 0xffff) code += names[key];
  return code;
}

static std::string exec(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  size_t lineEnd = line.size();
  std::string code = " "s;
  size_t key = line[from++];
  key += line[from++]*256;
  if (key < 0xffff) code += names[key];
  code += "(";
  while (from < lineEnd) {
    uint8_t byte = line[from];
    if (byte == 0x0d) {
      code += ",";
      from++;
    } else if (byte == 0x13 || byte == 0x15) {
      from++;
    } else {
      code += parseVal(names, line, from);
    }
  }
  code += ")";
  return code;
}

static std::string proc(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  std::string code = " PROC "s;
  size_t key = line[from++];
  key += line[from++]*256;
  code += names[key];
  code += toHex(line, from, line.size());
  return code;
}

static std::string dimString(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  std::string code = " DIM "s;
  size_t key = line[from++];
  key += line[from++]*256;
  code += names[key]+"$ OF ";
  from++;
  code += parseVal(names, line, from);
  from += 2;
  return code;
}

static std::string forLoop(std::map<size_t,std::string>& names, std::string suffix, std::vector<uint8_t>& line, size_t& from) {
  size_t lineEnd = line.size();
  std::string code = " FOR "s;
  size_t key = line[from++];
  key += line[from++]*256;
  std::string varName = names[key];
  code += "["+toHex(line, from, from+2)+"]";
  code += varName+suffix+":=";
  code += parseVal(names, line, from) + " TO ";
  from++;
  code += parseVal(names, line, from);
  from++; size_t pos = from;
  while (pos<lineEnd && line[pos]!=0x86) pos++;
  if (pos<lineEnd) {
    code += " STEP " + parseVal(names, line, from);
    from++;
  }
  code += " DO";
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

static std::string forReal(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  return forLoop(names, ""s, line, from);
}

static std::string forInteger(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  return forLoop(names, "#"s, line, from);
}

static std::string endFor(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  std::string code = " ENDFOR"s;
  size_t key = line[from++];
  key += line[from++]*256;
  if (key < 0xffff) code += " "+names[key];
  code += " ["+toHex(line, from, from+2)+"]";
  return code;
}

static std::string whileCond(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " WHILE"s;
  code += toHex(line, from, line.size());
  return code;
}

static std::string end(std::vector<uint8_t>& line, size_t& from) {
  std::string code = " END"s;
  code += toHex(line, from, line.size());
  return code;
}

static std::string use(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  std::string code = " USE "s;
  size_t key = line[from++];
  key += line[from++]*256;
  code += names[key];
  return code;
}

static std::string assign(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  std::string code = " "s;
  std::string value = parseVal(names, line, from);
  uint8_t op = line[from++];
  size_t key = line[from++];
  key += line[from++]*256;
  code += names[key];
  if (op == 0xfb) {
    code += "#";
  } else if (op == 0xf8) {
    code += "$";
  }
  code += ":="+value;
  return code;
}

static std::string import(std::map<size_t,std::string>& names, std::vector<uint8_t>& line, size_t& from) {
  size_t lineEnd = line.size();
  std::string code = " IMPORT "s;
  code += toHex(line, from, from+3);
  while (from < lineEnd) {
    uint8_t byte = line[from];
    if (byte == 0x72) {
      code += ",";
      from++;
    } else if (byte == 0xef) {
      from++;
    } else {
      size_t key = line[from++];
      key += line[from++]*256;
      code += names[key];
    }
  }
  return code;
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
  size_t lineSize = line.size();
  std::string code = ""s;
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
    } else if (op == 0x03) {
      pos--;
      code += indentFor(indent) + assign(names, line, pos);
    } else if (op == 0x05) {
      pos--;
      code += indentFor(indent) + assign(names, line, pos);
    } else if (op == 0x0e) {
      indent -= 2;
      code += indentFor(indent) + endProc(names, line, pos);
    } else if (op == 0x1a) {
      code += indentFor(indent) + exec(names, line, pos);
    } else if (op == 0x70) {
      code += indentFor(indent) + proc(names, line, pos);
      indent += 2;
    } else if (op == 0x82) {
      code += indentFor(indent) + forReal(names, line, pos);
      if (endOp != 0x89) indent += 2;
    } else if (op == 0x83) {
      code += indentFor(indent) + forInteger(names, line, pos);
      if (endOp != 0x89) indent += 2;
    } else if (op == 0x8a) {
      indent -= 2;
      code += indentFor(indent) + endFor(names, line, pos);
    } else if (op == 0x8b) {
      indent -= 2;
      code += indentFor(indent) + endFor(names, line, pos);
    } else if (op == 0x8f) {
      code += indentFor(indent) + dimString(names, line, pos);
    } else if (op == 0x97) {
      code += indentFor(indent) + whileCond(line, pos);
      if (endOp != 0x9a) indent += 2;
    } else if (op == 0x9f) {
      code += indentFor(indent) + end(line, pos);
    } else if (op == 0xd4) {
      code += indentFor(indent) + use(names, line, pos);
    } else if (op == 0xce) {
      pos--;
      code += indentFor(indent) + assign(names, line, pos);
    } else if (op == 0xcf) {
    } else if (op == 0xeb) {
      code += indentFor(indent) + import(names, line, pos);
    } else {
      code += indentFor(indent) + " " + hexCode(op) + toHex(line, pos, lineSize);
    }
  }

  code += "\n";
  return code;
}
