#include "ParseNames.h"
#include "ParseOps.h"

std::map<size_t,std::string> getNameTable(std::vector<uint8_t>& data, size_t from, size_t to) {
  std::map<size_t,std::string> names {};
  names[0xffff] = "";
  size_t offset = 0;
  while (from < to) {
    uint8_t len = data[from++];
    std::string name = "["+toHex(data, from, from+3)+"]";
    name += text(data, from, from+len-4);
    names[offset] = name;
    offset += len;
  }
  return names;
}
