#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <optional>

#include "NameTable.h"

class TextParser {
protected:
  std::ifstream& in;
  NameTable names {};
  std::optional<std::vector<uint8_t>> output {};
  bool parse();
  void addNames(std::vector<uint8_t>& results);
public:
  TextParser(std::ifstream& inStream) : in (inStream) {}
  std::optional<std::vector<uint8_t>> contents();
};

#endif
