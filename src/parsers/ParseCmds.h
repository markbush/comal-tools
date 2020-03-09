#ifndef PARSE_CMDS_H
#define PARSE_CMDS_H

#include <vector>
#include <string>
#include <stdint.h>

#include "NameTable.h"

using Names = std::vector<std::pair<std::string,uint8_t>>;

std::vector<uint8_t> encode(uint16_t& autoLineNum, NameTable& names, std::string line);

#endif
