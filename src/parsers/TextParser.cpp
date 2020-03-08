#include "TextParser.h"
#include "ParseCmds.h"

#include "../logger/Logger.h"

using namespace std::string_literals;

static Logger logger {"TextParser"s};

void TextParser::addNames(std::vector<uint8_t>& results) {
  for (auto& name : names) {
    uint8_t nameLen = name.size()+4;
    results.push_back(nameLen);
    for (size_t i=0; i<3; i++) results.push_back(0x00);
    for (auto& c : name) {
      uint8_t ch = c;
      if (ch>=97 && ch<=122) {
        ch -= 32;
      }
      results.push_back(ch);
    }
  }
}

bool TextParser::parse() {
  if (output) return true;
  std::vector<uint8_t> results {0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  std::string line;

  while (std::getline(in, line)) {
    logger.info("{}", line);

    std::vector<uint8_t> lineBytes = encode(names, line);

    results.insert(results.end(), lineBytes.begin(), lineBytes.end());
  }
  for (size_t i=0; i<4; i++) results.push_back(0x00);

  uint16_t codeSize = results.size()-9;
  results[5] = codeSize&0xff;
  results[6] = codeSize/256;

  addNames(results);

  uint16_t fullSize = results.size()-8;
  results[7] = fullSize&0xff;
  results[8] = fullSize/256;

  for (size_t i=0; i<4; i++) results.push_back(0x00);

  output = results;
  return true;
}

std::optional<std::vector<uint8_t>> TextParser::contents() {
  if (!output) {
    if (!parse()) {
      return std::nullopt;
    }
  }
  return output;
}
