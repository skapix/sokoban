#include "soko/map.h"
#include <map>
#include <cassert>

namespace soko
{

namespace
{

bool eraseColumnIfWall(std::vector<std::vector<Cell>> &map, size_t n)
{
  if (map.empty() || map.front().empty())
  {
    return false;
  }
  assert(n < map.front().size());
  if (std::all_of(map.begin(), map.end(), [n](auto r) { return r[n] == Cell::Wall; }))
  {
    for (auto &r : map)
    {
      r.erase(r.begin() + n);
    }
    return true;
  }
  return false;
}

bool eraseRawIfWall(std::vector<std::vector<Cell>> &map, size_t n)
{
  if (map.empty())
  {
    return false;
  }
  assert(n < map.size());
  if (std::all_of(map[n].begin(), map[n].end(), [](auto c) { return c == Cell::Wall; }))
  {
    map.erase(map.begin() + n);
    return true;
  }
  return false;
}


std::vector<std::vector<Cell>> trimWalls(const std::vector<std::vector<Cell>> &map)
{
  std::vector<std::vector<Cell>> result = map;
  while (eraseRawIfWall(result, result.size() - 1))
    ;
  while (eraseRawIfWall(result, 0))
    ;
  while (eraseColumnIfWall(result, 0))
    ;
  while (!result.empty() && !result.front().empty() &&
         eraseColumnIfWall(result, result.front().size() - 1))
    ;
  return result;
}


void validateMap(const Map &map)
{
  auto nUnits = std::count(map.begin(), map.end(), Cell::Unit);
  nUnits += std::count(map.begin(), map.end(), Cell::UnitDestination);
  if (nUnits != 1)
  {
    throw std::logic_error("Bad map. Wrong level of units (" + std::to_string(nUnits) + ")");
  }
  auto nBoxes = std::count(map.begin(), map.end(), Cell::Box);
  nBoxes += std::count(map.begin(), map.end(), Cell::BoxDestination);
  auto nDestinations = std::count(map.begin(), map.end(), Cell::Box);
  nDestinations += std::count(map.begin(), map.end(), Cell::BoxDestination);
  if (nBoxes != nDestinations)
  {
    std::string err = "Bad map. Amount of boxes (" + std::to_string(nBoxes) + ") ";
    err += "is not equal to number of destinations (" + std::to_string(nDestinations) + ")";
    throw std::logic_error(err);
  }
}

} // namespace


Map::Map(const std::vector<std::vector<Cell>> &str)
  : Mat(trimWalls(str))
{
  validateMap(*this);
}

} // namespace soko
