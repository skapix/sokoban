#include "soko/util.h"
#include "soko/pos.h"

#include <queue>

namespace soko
{

Cell removeItem(Cell from, Cell item) noexcept
{
  int casted = static_cast<int>(from);
  casted &= ~static_cast<int>(item);
  return static_cast<Cell>(casted);
}

Cell placeItem(Cell cell, Cell item) noexcept
{
  int casted = static_cast<int>(cell);
  casted |= static_cast<int>(item);
  return static_cast<Cell>(casted);
}

Pos getUnit(const Map &m) noexcept
{
  auto it = std::find_if(m.begin(), m.end(),
                         [](Cell c) { return static_cast<int>(c) & static_cast<int>(Cell::Unit); });
  assert(it != m.end());
  return m.iteratorToPos(it);
}

Pos extractUnit(Map &m) noexcept
{
  Pos result = getUnit(m);
  m.at(result) = removeItem(m.at(result), Cell::Unit);
  return result;
}

std::vector<Pos> getBoxesEx(Map &m, bool extract) noexcept
{
  std::vector<Pos> boxes;
  auto it = m.begin();
  while (true)
  {
    it = std::find_if(it, m.end(),
                      [](Cell c) { return c == Cell::Box || c == Cell::BoxDestination; });
    if (it == m.end())
    {
      return boxes;
    }
    boxes.push_back(m.iteratorToPos(it));
    if (extract)
    {
      m.at(boxes.back()) = removeItem(m.at(boxes.back()), Cell::Box);
    }
    ++it;
  }
}

std::vector<Pos> getBoxes(const Map &m) noexcept { return getBoxesEx(const_cast<Map &>(m), false); }

std::vector<Pos> extractBoxes(Map &m) noexcept { return getBoxesEx(m, true); }

std::vector<Move> unitPathTo(const Map &m, Pos destPos) noexcept
{
  std::vector<Move> result;
  auto unit = getUnit(m);
  result.clear();
  std::map<Pos, Pos> visited;
  visited[unit] = unit;
  std::queue<Pos> posToVisit;
  posToVisit.push(unit);

  while (!posToVisit.empty())
  {
    auto current = posToVisit.front();
    if (current == destPos)
    {
      break;
    }
    posToVisit.pop();

    for (auto it : {Move::Left, Move::Up, Move::Right, Move::Down})
    {
      Pos p = current + it;
      if (m.safeIsFree(p) && !visited.count(p))
      {
        visited[p] = current;
        posToVisit.push(p);
      }
    }
  }
  auto dest = visited.find(destPos);
  if (dest == visited.end())
  {
    return {};
  }
  while (dest->first != dest->second)
  {
    result.push_back(restoreMove(dest->second, dest->first));
    dest = visited.find(dest->second);
    assert(dest != visited.end());
  }
  std::reverse(result.begin(), result.end());

  return result;
}

Move restoreMove(const Pos &from, const Pos &to) noexcept
{
  assert(from != to);
  if (from.i == to.i)
  {
    return from.j < to.j ? Move::Right : Move::Left;
  }
  if (from.j == to.j)
  {
    return from.i < to.i ? Move::Down : Move::Up;
  }
  UNREACHABLE;
}

soko::MapStatic mapToMapStatic(const soko::Map &m, std::vector<soko::Pos> *boxes, soko::Pos *unit)
{
  soko::MapStatic result = m;
  auto b = soko::extractBoxes(result);
  auto u = soko::extractUnit(result);


  if (boxes != nullptr)
  {
    *boxes = std::move(b);
  }
  if (unit != nullptr)
  {
    *unit = std::move(u);
  }
  return result;
}

Mat<bool> drawUnitMap(const Map &map, const Pos &unit, const std::vector<Pos> &boxes)
{
  Mat<bool> result(map.rows(), map.cols(), false);
  result.set(unit);

  std::queue<Pos> posesToWatch;
  posesToWatch.push(unit);
  while (!posesToWatch.empty())
  {
    Pos current = posesToWatch.front();
    posesToWatch.pop();
    for (auto m : {Move::Up, Move::Left, Move::Right, Move::Down})
    {
      Pos p = current + m;
      if (safeIsFree(map, p, boxes) && !result.at(p))
      {
        posesToWatch.push(p);
        result.set(p);
      }
    }
  }

  return result;
}

} // namespace soko
