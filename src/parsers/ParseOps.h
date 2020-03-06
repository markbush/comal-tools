#ifndef PARSE_OPS_H
#define PARSE_OPS_H

#include <map>
#include <vector>
#include <string>
#include <stdint.h>

std::string hexCode(uint8_t op);
std::string toHex(std::vector<uint8_t>& line, size_t& from, size_t to);
std::string text(std::vector<uint8_t>& line, size_t& from, size_t to);
std::vector<uint8_t> nextLine(std::vector<uint8_t>::iterator& start);
std::string decodeLine(std::map<size_t,std::string>& names, size_t& indent, std::vector<uint8_t> line);

#endif
