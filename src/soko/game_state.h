#pragma once
#include <functional>
#include "soko/map.h"
#include "soko/move.h"

namespace soko
{

enum class MoveResult : uint8_t
{
  NoMove,
  UnitMove,
  UnitBoxMove,
};

struct MoveDirection
{
  MoveResult result;
  Move move;

  operator bool() { return result != MoveResult::NoMove; }
};

class GameState {
public:
  GameState(const Map &map) noexcept;

  bool canMove(Move direction) const noexcept;
  MoveDirection move(Move direction) noexcept;

  MoveResult undo(MoveDirection r) noexcept;

  bool isWinningState() const noexcept;

  Pos unit() const noexcept { return m_unit; }

  const Map &map() const noexcept { return m_map; }

private:
  MoveResult undoUp(bool boxMoved) noexcept;
  MoveResult undoDown(bool boxMoved) noexcept;
  MoveResult undoLeft(bool boxMoved) noexcept;
  MoveResult undoRight(bool boxMoved) noexcept;

  bool moveIfBoxTo(Pos from, Pos to) noexcept;
  void moveUnitTo(Pos to) noexcept;

private:
  Pos m_unit;
  Map m_map;
};

} // namespace soko
