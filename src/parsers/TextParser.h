#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <optional>

class TextParser {
protected:
  std::ifstream& in;
  std::vector<std::string> names {};
  std::optional<std::vector<uint8_t>> output {};
  bool parse();
  void addNames(std::vector<uint8_t>& results);
public:
  TextParser(std::ifstream& inStream) : in (inStream) {}
  std::optional<std::vector<uint8_t>> contents();
};

#endif
