#include "NameTable.h"

void NameTable::add(std::string var, uint8_t type) {
  if (offset(var, type) != 0xffff) return;

  varOffset[nextOffset] = std::make_shared<std::pair<std::string,uint8_t>>(var, type);
  uint8_t len = 4+var.size();
  bytes.push_back(len);
  bytes.push_back(type);
  bytes.push_back(0x00);
  bytes.push_back(0x00);
  for (uint8_t ch : var) {
    if (ch>=97 && ch<=122) {
      ch -= 32;
    } else if (ch>=65 && ch<=90) {
      ch += 128;
    }
    bytes.push_back(ch);
  }
  nextOffset += len;
}

std::vector<uint8_t> NameTable::contents() {
  return bytes;
}

uint16_t NameTable::offset(std::string var, uint8_t type) {
  std::pair<std::string,uint8_t> entry {var, type};
  for (auto& [key, value] : varOffset) {
    if (*value == entry) return key;
  }
  return 0xffff;
}
