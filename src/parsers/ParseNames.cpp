#include "ParseNames.h"
#include "ParseOps.h"

#include "../logger/Logger.h"

using namespace std::string_literals;

static Logger logger {"ParseNames"s};

std::map<size_t,std::string> getNameTable(std::vector<uint8_t>& data, size_t from, size_t to) {
  std::map<size_t,std::string> names {};
  names[0xffff] = "";
  size_t offset = 0;
  while (from < to) {
    uint8_t len = data[from];
    uint8_t type = data[from+1];
    std::string rawData = "[" + toHex(data, from, from+4) + "] ";
    std::string name = text(data, from, from+len-4);
    rawData += name;
    std::string typeName;
    if (type == 0x00) typeName = " (unknown)"s;
    else if (type == 0x13) typeName = " (label)"s;
    else if (type == 0x14) typeName = " (proc)"s;
    else if (type == 0x18) typeName = " (package)"s;
    else {
      if ((type&0xfc) == 0x14) type--;
      switch (type&0x03) {
        case 0x00: typeName = " (real "s; break;
        case 0x01: typeName = " (integer "s; break;
        case 0x02: typeName = " (string "s; break;
        default: break;
      }
      switch (type&0xfc) {
        case 0x10: typeName += "var)"s; break;
        case 0x14: typeName += "func)"s; break;
        case 0x30: typeName += "param)"s; break;
        case 0x50: typeName += "ref param)"s; break;
        case 0x90: typeName += "array)"s; break;
        default: break;
      }
    }
    rawData += typeName;
    logger.info(rawData);
    names[offset] = name;
    offset += len;
  }
  return names;
}
