#ifndef PARSE_OPS_H
#define PARSE_OPS_H

#include <vector>
#include <string>
#include <stdint.h>

bool eof(std::vector<uint8_t>::iterator& start);
std::vector<uint8_t> nextLine(std::vector<uint8_t>::iterator& start);
std::string decodeLine(size_t& indent, std::vector<uint8_t> line);

#endif
