#pragma once

#include <cstddef>

namespace soko
{

struct Pos
{
  Pos() = default;
  constexpr Pos(size_t i, size_t j)
    : i(i)
    , j(j)
  {}
  constexpr Pos operator-() const noexcept { return {static_cast<size_t>(-i), static_cast<size_t>(-j)}; }

  size_t i;
  size_t j;
};

constexpr inline Pos operator+(const Pos &p1, const Pos &p2) noexcept
{
  return Pos(p1.i + p2.i, p1.j + p2.j);
}

constexpr inline Pos operator-(const Pos &p1, const Pos &p2) noexcept
{
  return Pos(p1.i - p2.i, p1.j - p2.j);
}

constexpr inline bool operator<(const Pos &p1, const Pos &p2) noexcept
{
  return p1.i < p2.i || (p1.i == p2.i && p1.j < p2.j);
}

constexpr inline bool operator==(const Pos &p1, const Pos &p2) noexcept
{
  return p1.i == p2.i && p1.j == p2.j;
}

constexpr inline bool operator!=(const Pos &p1, const Pos &p2) noexcept { return !(p1 == p2); }

constexpr inline bool operator<=(const Pos &p1, const Pos &p2) noexcept
{
  return p1 < p2 || p1 == p2;
}

} // namespace soko
