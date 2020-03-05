#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string>
#include <fstream>

#include "logger/Logger.h"
#include "parsers/BinaryParser.h"

using namespace std::string_literals;

static Logger logger {"main"};

static std::optional<std::string> parseInput(std::ifstream&);

int main(int argc, char* argv[]) {
  logger.info("Start"s);

  std::string outFileName = (argc<3)? "/dev/fd/1": argv[2];
  std::string inFileName = (argc<2)? "/dev/fd/0": argv[1];

  logger.info("IN:  {}", inFileName);
  logger.info("OUT: {}", outFileName);

  std::ifstream inStream(inFileName, std::ios::binary);
  inStream.unsetf(std::ios::skipws);
  std::ofstream outStream(outFileName);

  std::optional<std::string> output {};
  try {
    output = parseInput(inStream);
  } catch (std::exception& e) {
    logger.error(e.what());
  }

  if (output) {
    outStream << output.value();
    std::flush(outStream);
  } else {
    logger.warn("NO OUTPUT");
  }

  logger.info("End");
  return 0;
}

static std::optional<std::string> parseInput(std::ifstream& inStream) {
  BinaryParser parser {inStream};
  std::optional<std::string> output = parser.contents();
  return output;
}
