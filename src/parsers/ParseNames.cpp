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
    std::string rawData = "[" + toHex(data, from, from+4) + "] ";
    std::string name = text(data, from, from+len-4);
    rawData += name;
    logger.info(rawData);
    names[offset] = name;
    offset += len;
  }
  return names;
}
