#include "soko/map.h"
#include <map>
#include <cassert>

#include "soko/util.h"

namespace soko
{

namespace
{

soko::Map fillUnreachable(const soko::Map &originalMap)
{
  if (originalMap.empty())
  {
    return originalMap;
  }
  auto result = originalMap;
  Pos unit;
  std::vector<Pos> boxes;
  mapToMapStatic(result, &boxes, &unit);
  auto reachable = drawUnitMap(result, unit, {});
  for (size_t i = 0; i < originalMap.rows(); ++i)
  {
    for (size_t j = 0; j < originalMap.cols(); ++j)
    {
      if (!reachable.at(i, j))
      {
        result.at(i, j) = Cell::Wall;
      }
    }
  }

  result.at(unit) = placeItem(result.at(unit), Cell::Unit);
  for (auto box : boxes)
  {
    if (result.at(box) != Cell::Wall)
    {
      result.at(box) = placeItem(result.at(box), Cell::Box);
    }
  }

  return result;
}

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

std::vector<std::vector<Cell>> toVector(const Map &m)
{
  std::vector<std::vector<Cell>> result;
  for (size_t i = 0; i < m.rows(); ++i)
  {
    result.push_back({});
    for (size_t j = 0; j < m.cols(); ++j)
    {
      result.back().push_back(m.at(i, j));
    }
  }
  return result;
}

std::vector<std::vector<Cell>> trimWalls(const Map &map)
{
  if (map.empty())
  {
    return {};
  }
  std::vector<std::vector<Cell>> result = toVector(map);
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
    throw std::logic_error("Bad map. Wrong amount of units (" + std::to_string(nUnits) + ")");
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
  : Mat(str)
{
  *this = fillUnreachable(*this);


  auto res = trimWalls(*this);
  size_t strCols = str.empty() ? 0 : str.front().size();
  size_t resCols = res.empty() ? 0 : res.front().size();

  if (res.size() != str.size() || strCols != resCols)
  {
    *this = res;
  }

  validateMap(*this);
}

} // namespace soko
