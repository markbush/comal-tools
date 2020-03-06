#ifndef PARSE_NAMES_H
#define PARSE_NAMES_H

#include <map>
#include <vector>
#include <string>
#include <stdint.h>

std::map<size_t,std::string> getNameTable(std::vector<uint8_t>& data, size_t from, size_t to);

#endif
