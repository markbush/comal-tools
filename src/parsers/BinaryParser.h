#ifndef BINARY_PARSER_H
#define BINARY_PARSER_H

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <stdint.h>
#include <optional>

class BinaryParser {
protected:
  std::ifstream& in;
  std::vector<uint8_t> bytes;
  std::map<size_t,std::string> names;
  std::optional<std::string> output {};
  bool parse();
public:
  BinaryParser(std::ifstream& inStream) : in (inStream) {}
  std::optional<std::string> contents();
};

#endif
