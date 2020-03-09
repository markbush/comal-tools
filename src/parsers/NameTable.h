#ifndef NAME_TABLE_H
#define NAME_TABLE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdint.h>

class NameTable {
protected:
  std::vector<uint8_t> bytes {};
  std::map<uint16_t,std::shared_ptr<std::pair<std::string,uint8_t>>> varOffset {};
  uint16_t nextOffset {0};
public:
  void add(std::string var, uint8_t type);
  std::vector<uint8_t> contents();
  uint16_t offset(std::string var, uint8_t type);
};

#endif
