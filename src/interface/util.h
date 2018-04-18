#pragma once

#include "soko/map.h"
#include <sstream>

namespace soko
{
class Heuristic;
}

std::vector<std::pair<std::string, soko::Map>> parseFromFile(std::istream &file);

size_t calculateHeuristic(const soko::Map &m, const soko::Heuristic &initedHeuristic);
