#pragma once

#include "soko/map.h"
#include "soko/move.h"

namespace soko
{
constexpr size_t g_inf = std::numeric_limits<size_t>::max();

constexpr Pos operator+(Pos p, Move m) noexcept
{
  p.j += toHorizontal(m);
  p.i += toVertical(m);
  return p;
}
constexpr Pos operator+=(Pos &p, Move m) noexcept
{
  p = p + m;
  return p;
}

constexpr Pos operator-(Pos p, Move m) noexcept
{
  p.j -= toHorizontal(m);
  p.i -= toVertical(m);
  return p;
}

constexpr Pos operator-=(Pos &p, Move m) noexcept
{
  p = p - m;
  return p;
}

inline bool isBox(Pos p, const std::vector<Pos> &boxes) noexcept
{
  assert(std::is_sorted(boxes.begin(), boxes.end()));
  return std::binary_search(boxes.begin(), boxes.end(), p);
}

inline bool safeIsFree(const MapStatic &m, Pos p, const std::vector<Pos> &boxes) noexcept
{
  return !m.safeIsWall(p) && !isBox(p, boxes);
}

std::vector<Move> unitPathTo(const Map &m, Pos p) noexcept;
Move restoreMove(const Pos &from, const Pos &to) noexcept;


Cell removeItem(Cell from, Cell item) noexcept;
Cell placeItem(Cell cell, Cell item) noexcept;

Pos getUnit(const Map &m) noexcept;
Pos extractUnit(Map &m) noexcept;

std::vector<Pos> getBoxes(const Map &m) noexcept;
std::vector<Pos> extractBoxes(Map &m) noexcept;

MapStatic mapToMapStatic(const Map &m, std::vector<Pos> *boxes = nullptr, Pos *unit = nullptr);
Mat<bool> drawUnitMap(const Map &map, const Pos &unit, const std::vector<Pos> &boxes);

} // namespace soko
