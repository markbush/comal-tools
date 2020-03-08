#ifndef PARSE_CMDS_H
#define PARSE_CMDS_H

#include <vector>
#include <string>
#include <stdint.h>

std::vector<uint8_t> encode(std::vector<std::string>& names, std::string line);

#endif
