#include <algorithm>
#include "BinaryParser.h"
#include "ParseOps.h"
#include "ParseNames.h"

#include "../logger/Logger.h"

using namespace std::string_literals;

static Logger logger {"BinaryParser"s};

#define START_OFFSET 9

static std::vector<uint8_t> magic {0xff, 0xff, 0x02, 0x00};

bool BinaryParser::parse() {
  if (output) return true;

  std::string result = ""s;

  std::copy(std::istream_iterator<uint8_t>(in), std::istream_iterator<uint8_t>(), std::back_inserter(bytes));
  size_t fileSize = bytes.size();
  logger.info("Total {} bytes", fileSize);

  if (!std::equal(magic.begin(), magic.end(), bytes.begin())) {
    logger.error("NOT COMAL PRG!");
    return false;
  }
  std::string version = "2.0"+std::to_string(bytes[4]);
  logger.info("COMAL 80 v{}", version);
  size_t len = (bytes[6]*256)+bytes[5];
  size_t endOffset = (bytes[8]*256)+bytes[7];
  size_t nameTableStart = len+START_OFFSET;
  size_t codeLen = len-4;
  logger.info("{} bytes", codeLen);
  size_t codeEnd = nameTableStart-4;
  logger.info("End: {}", codeEnd);
  size_t nameTableSize = fileSize - nameTableStart - 4;
  logger.info("Name table: {} bytes", nameTableSize);

  names = getNameTable(bytes, nameTableStart, fileSize-4);
  for (auto& [offset, name] : names) {
    uint8_t high = (offset&0xff00)/256;
    uint8_t low = offset&0xff;
    std::string key = hexCode(low) + " " + hexCode(high);
    logger.info("  {} = {}", key, name);
  }

  auto iter = bytes.begin()+START_OFFSET;
  auto codeLimit = bytes.begin()+codeEnd;
  size_t indent = 0;
  while (iter < codeLimit) {
    std::vector<uint8_t> line = nextLine(iter);
    std::string code = decodeLine(names, indent, line);
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
