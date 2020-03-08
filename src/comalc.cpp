#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string>
#include <fstream>
#include <stdint.h>

#include "logger/Logger.h"
#include "parsers/TextParser.h"

using namespace std::string_literals;

static Logger logger {"main"s};

static std::optional<std::vector<uint8_t>> parseInput(std::ifstream&);

int main(int argc, char* argv[]) {
  logger.info("Start"s);

  std::string outFileName = (argc<3)? "/dev/fd/1"s: ""s+argv[2];
  std::string inFileName = (argc<2)? "/dev/fd/0"s: ""s+argv[1];

  logger.info("IN:  {}", inFileName);
  logger.info("OUT: {}", outFileName);

  std::ifstream inStream(inFileName, std::ios::binary);
  inStream.unsetf(std::ios::skipws);
  std::ofstream outStream(outFileName);

  std::optional<std::vector<uint8_t>> output {};
  try {
    output = parseInput(inStream);
  } catch (std::exception& e) {
    logger.error(e.what());
  }

  if (output) {
    outStream.write((char *) &(output.value().data()[0]), output.value().size());
    std::flush(outStream);
  } else {
    logger.warn("NO OUTPUT");
  }

  logger.info("End");
  return 0;
}

static std::optional<std::vector<uint8_t>> parseInput(std::ifstream& inStream) {
  TextParser parser {inStream};
  std::optional<std::vector<uint8_t>> output = parser.contents();
  return output;
}
