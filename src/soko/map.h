#pragma once

#include <string>
#include <map>
#include "soko/cell.h"
#include "soko/mat.hpp"
#include "sstream"

namespace soko
{

class Map : public Mat<Cell> {
public:
  Map(const std::vector<std::vector<Cell>> &str);

  Map() = default;
  Map(Map &&m) = default;
  Map(const Map &m) = default;
  Map &operator=(Map &&) = default;
  Map &operator=(const Map &) = default;

private:
};

} // namespace soko
