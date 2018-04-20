#pragma once

#include <cstdint>
#include "soko/cross.h"

namespace soko
{

enum class Move : uint8_t
{
  Left = 0,
  Right,
  Up,
  Down
};

constexpr int toHorizontal(Move m) noexcept
{
  return m == Move::Left ? -1 : m == Move::Right ? 1 : 0;
}

constexpr int toVertical(Move m) noexcept { return m == Move::Up ? -1 : m == Move::Down ? 1 : 0; }

constexpr Move reverse(Move m) noexcept
{
  switch (m)
  {
  case Move::Up:
    return Move::Down;
  case Move::Down:
    return Move::Up;
  case Move::Left:
    return Move::Right;
  case Move::Right:
    return Move::Left;
  }
  UNREACHABLE;
}

constexpr Move clockwiseRotate(Move m) noexcept
{
  switch (m)
  {
  case Move::Up:
    return Move::Right;
  case Move::Down:
    return Move::Left;
  case Move::Left:
    return Move::Up;
  case Move::Right:
    return Move::Down;
  }
  UNREACHABLE;
}


} // namespace soko
