#pragma once

#include "soko/map.h"
#include <sstream>


std::vector<std::pair<std::string, soko::Map>> parseFromFile(std::istream &file);
