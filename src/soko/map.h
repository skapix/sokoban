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

  bool isWall(Pos p) const noexcept { return at(p) == Cell::Wall; }
  bool isBox(Pos p) const noexcept { return static_cast<int>(at(p)) & static_cast<int>(Cell::Box); }
  bool isFree(Pos p) const noexcept { return at(p) == Cell::Field || at(p) == Cell::Destination; }
  bool isUnit(Pos p) const noexcept
  {
    return static_cast<int>(at(p)) & static_cast<int>(Cell::Unit);
  }
  bool isDestination(Pos p) const noexcept
  {
    return static_cast<int>(at(p)) & static_cast<int>(Cell::Destination);
  }


  // Safe functions can be used if Pos is out of bounds
  // Pos out of bounds => Cell in pos is wall
  bool safeIsWall(Pos p) const noexcept { return !contains(p) || isWall(p); }
  bool safeIsBox(Pos p) const noexcept { return contains(p) && isBox(p); }
  bool safeIsFree(Pos p) const noexcept { return contains(p) && isFree(p); }
  bool safeIsUnit(Pos p) const noexcept { return contains(p) && isUnit(p); }
  bool safeIsDestination(Pos p) const noexcept { return contains(p) && isDestination(p); }


private:
};

// map, that contains only static object, like walls and destinations
using MapStatic = Map;

} // namespace soko
