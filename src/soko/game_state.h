#pragma once
#include <functional>
#include "soko/map.h"

namespace soko
{

enum class Move : uint8_t
{
  Left,
  Right,
  Up,
  Down
};

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
  GameState(const Map &map);

  bool canMove(Move direction) const;
  MoveDirection move(Move direction);

  MoveResult undo(MoveDirection r);

  bool isWinningState() const;

  Pos unit() const { return m_unit; }

private:
  bool canMoveUp() const;
  bool canMoveDown() const;
  bool canMoveLeft() const;
  bool canMoveRight() const;

  MoveDirection moveUp();
  MoveDirection moveDown();
  MoveDirection moveLeft();
  MoveDirection moveRight();

  MoveResult undoUp(bool boxMoved);
  MoveResult undoDown(bool boxMoved);
  MoveResult undoLeft(bool boxMoved);
  MoveResult undoRight(bool boxMoved);

  bool isFreeSpace(Pos p) const;
  bool isFreeSpace(size_t i, size_t j) const;

  bool isBoxSpace(Pos p) const;
  bool isBoxSpace(size_t i, size_t j) const;

  bool moveIfBoxTo(Pos from, Pos to);
  void moveUnitTo(Pos to);

  void validateUnit() const;

  void removeBox(Pos p);
  void removeUnit();
  void placeBox(Pos p);
  void placeUnit(Pos p);

private:
  Pos m_unit;
  Map m_map;
};

} // namespace soko
