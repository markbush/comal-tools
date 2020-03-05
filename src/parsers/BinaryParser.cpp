#include <algorithm>
#include "BinaryParser.h"
#include "ParseOps.h"

#include "../logger/Logger.h"
static Logger logger {"BinaryParser"};

#define START_OFFSET 9

static std::vector<uint8_t> magic {0xff, 0xff, 0x02, 0x00};

bool BinaryParser::parse() {
  if (output) return true;

  std::string result = "";

  std::copy(std::istream_iterator<uint8_t>(in), std::istream_iterator<uint8_t>(), std::back_inserter(bytes));
  logger.info("SIZE: {}", bytes.size());

  if (!std::equal(magic.begin(), magic.end(), bytes.begin())) {
    logger.error("NOT COMAL PRG!");
    return false;
  }

  auto iter = bytes.begin()+START_OFFSET;
  size_t indent = 0;
  while (iter < bytes.end() && !eof(iter)) {
    std::vector<uint8_t> line = nextLine(iter);
    std::string code = decodeLine(indent, line);
    result += code;
  }

  output = result;
  return true;
}

std::optional<std::string> BinaryParser::contents() {
  if (!output) {
    if (!parse()) {
      return std::nullopt;
    }
  }
  return output;
}
