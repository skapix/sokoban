#include "soko/game_state.h"

namespace soko
{

GameState::GameState(const Map &map)
  : m_map(map)
{
  auto it = std::find(m_map.begin(), m_map.end(), Cell::Unit);
  assert(it != map.end());
  assert(map.rows() >= 1);
  assert(map.cols() >= 1);
  m_unit = m_map.numToPos(it - m_map.begin());
}

bool GameState::canMoveUp() const
{
  bool canMoveUp1 = m_unit.i > 0 && isFreeSpace(m_unit.i - 1, m_unit.j);
  bool canMoveUp2 = m_unit.i > 1 && isBoxSpace(m_unit.i - 1, m_unit.j) &&
                    isFreeSpace(m_unit.i - 2, m_unit.j);
  return canMoveUp1 || canMoveUp2;
}

bool GameState::canMoveDown() const
{
  size_t max_row = m_map.rows() - 1;
  bool canMoveDown1 = m_unit.i < max_row && isFreeSpace(m_unit.i + 1, m_unit.j);
  bool canMoveDown2 = m_unit.i + 1 < max_row && isBoxSpace(m_unit.i + 1, m_unit.j) &&
                      isFreeSpace(m_unit.i + 2, m_unit.j);
  return canMoveDown1 || canMoveDown2;
}

bool GameState::canMoveLeft() const
{
  bool canMoveLeft1 = m_unit.j > 0 && isFreeSpace(m_unit.i, m_unit.j - 1);
  bool canMoveLeft2 = m_unit.j > 1 && isBoxSpace(m_unit.i, m_unit.j - 1) &&
                      isFreeSpace(m_unit.i, m_unit.j - 2);
  return canMoveLeft1 || canMoveLeft2;
}

bool GameState::canMoveRight() const
{
  size_t max_col = m_map.cols() - 1;
  bool canMoveRight1 = m_unit.j < max_col && isFreeSpace(m_unit.i, m_unit.j + 1);
  bool canMoveRight2 = m_unit.j < max_col - 1 && isBoxSpace(m_unit.i, m_unit.j + 1) &&
                       isFreeSpace(m_unit.i, m_unit.j + 2);
  return canMoveRight1 || canMoveRight2;
}

bool GameState::canMove(Move direction) const
{
  switch (direction)
  {
  case Move::Down:
    return canMoveDown();
  case Move::Up:
    return canMoveUp();
  case Move::Left:
    return canMoveLeft();
  case Move::Right:
    return canMoveRight();
  }
  assert(false);
  return false;
}

MoveDirection GameState::move(Move direction)
{
  switch (direction)
  {
  case Move::Down:
    return moveDown();
  case Move::Up:
    return moveUp();
  case Move::Left:
    return moveLeft();
  case Move::Right:
    return moveRight();
  }
  assert(false);
  return {};
}


MoveDirection GameState::moveUp()
{
  validateUnit();
  if (!canMoveUp())
  {
    return {MoveResult::NoMove, Move::Up};
  }

  size_t i = m_unit.i;
  size_t j = m_unit.j;
  bool boxMoved = moveIfBoxTo({i - 1, j}, {i - 2, j});
  moveUnitTo({i - 1, j});
  return {boxMoved ? MoveResult::UnitBoxMove : MoveResult::UnitMove, Move::Up};
}

MoveDirection GameState::moveDown()
{
  validateUnit();
  if (!canMoveDown())
  {
    return {MoveResult::NoMove, Move::Down};
  }

  size_t i = m_unit.i;
  size_t j = m_unit.j;
  bool boxMoved = moveIfBoxTo({i + 1, j}, {i + 2, j});
  moveUnitTo({i + 1, j});
  return {boxMoved ? MoveResult::UnitBoxMove : MoveResult::UnitMove, Move::Down};
}

MoveDirection GameState::moveLeft()
{
  validateUnit();
  if (!canMoveLeft())
  {
    return {MoveResult::NoMove, Move::Left};
  }

  size_t i = m_unit.i;
  size_t j = m_unit.j;
  bool boxMoved = moveIfBoxTo({i, j - 1}, {i, j - 2});
  moveUnitTo({i, j - 1});
  return {boxMoved ? MoveResult::UnitBoxMove : MoveResult::UnitMove, Move::Left};
}

MoveDirection GameState::moveRight()
{
  validateUnit();
  if (!canMoveRight())
  {
    return {MoveResult::NoMove, Move::Right};
  }

  size_t i = m_unit.i;
  size_t j = m_unit.j;
  bool boxMoved = moveIfBoxTo({i, j + 1}, {i, j + 2});
  moveUnitTo({i, j + 1});
  return {boxMoved ? MoveResult::UnitBoxMove : MoveResult::UnitMove, Move::Right};
}

MoveResult GameState::undo(MoveDirection r)
{
  if (r.result == MoveResult::NoMove)
  {
    return MoveResult::NoMove;
  }
  bool boxMoved = r.result == MoveResult::UnitBoxMove;

  switch (r.move)
  {
  case Move::Down:
    return undoDown(boxMoved);
    break;
  case Move::Up:
    return undoUp(boxMoved);
    break;
  case Move::Left:
    return undoLeft(boxMoved);
    break;
  case Move::Right:
    return undoRight(boxMoved);
    break;
  }

  assert(false);
  return MoveResult::NoMove;
}

MoveResult GameState::undoUp(bool boxMoved)
{
  size_t i = m_unit.i;
  size_t j = m_unit.j;

  moveUnitTo({i + 1, j});
  if (boxMoved && i > 0)
  {
    bool moved = moveIfBoxTo({i - 1, j}, {i, j});
    assert(moved);
    return MoveResult::UnitBoxMove;
  }
  return MoveResult::UnitMove;
}

MoveResult GameState::undoDown(bool boxMoved)
{
  size_t i = m_unit.i;
  size_t j = m_unit.j;

  moveUnitTo({i - 1, j});
  if (boxMoved && i < m_map.rows() - 1)
  {
    bool moved = moveIfBoxTo({i + 1, j}, {i, j});
    assert(moved);
    return MoveResult::UnitBoxMove;
  }
  return MoveResult::UnitMove;
}

MoveResult GameState::undoLeft(bool boxMoved)
{
  size_t i = m_unit.i;
  size_t j = m_unit.j;

  moveUnitTo({i, j + 1});
  if (boxMoved && j > 0)
  {
    bool moved = moveIfBoxTo({i, j - 1}, {i, j});
    assert(moved);
    return MoveResult::UnitBoxMove;
  }
  return MoveResult::UnitMove;
}

MoveResult GameState::undoRight(bool boxMoved)
{
  size_t i = m_unit.i;
  size_t j = m_unit.j;

  moveUnitTo({i, j - 1});
  if (boxMoved && j < m_map.cols() - 1)
  {
    bool moved = moveIfBoxTo({i, j + 1}, {i, j});
    assert(moved);
    return MoveResult::UnitBoxMove;
  }
  return MoveResult::UnitMove;
}

bool GameState::isWinningState() const
{
  auto pred = [](Cell c) { return c == Cell::Box || c == Cell::Destination; };
  return std::find_if(m_map.begin(), m_map.end(), pred) == m_map.end();
}

bool GameState::isFreeSpace(Pos p) const
{
  assert(m_map.contains(p));
  auto c = m_map.at(p);
  return c == Cell::Destination || c == Cell::Field;
}

bool GameState::isFreeSpace(size_t i, size_t j) const { return isFreeSpace({i, j}); }

bool GameState::isBoxSpace(Pos p) const
{
  assert(m_map.contains(p));
  auto c = m_map.at(p);
  return static_cast<int>(c) & static_cast<int>(Cell::Box);
}

bool GameState::isBoxSpace(size_t i, size_t j) const { return isBoxSpace({i, j}); }

bool GameState::moveIfBoxTo(Pos from, Pos to)
{
  if (!isBoxSpace(from))
  {
    return false;
  }
  removeBox(from);
  placeBox(to);
  return true;
}

void GameState::moveUnitTo(Pos to)
{
  removeUnit();
  placeUnit(to);
  m_unit = to;
}

void GameState::validateUnit() const
{
  assert(m_map.contains(m_unit));
  assert(static_cast<int>(m_map.at(m_unit)) & static_cast<int>(Cell::Unit));
}

void GameState::removeBox(Pos p)
{
  assert(isBoxSpace(p));
  auto c = m_map.at(p);
  c = static_cast<Cell>(static_cast<int>(c) & ~static_cast<int>(Cell::Box));
  m_map.at(p) = c;
}

void GameState::removeUnit()
{
  validateUnit();
  auto c = m_map.at(m_unit);
  c = static_cast<Cell>(static_cast<int>(c) & ~static_cast<int>(Cell::Unit));
  m_map.at(m_unit) = c;
  m_unit = {0, 0};
}

void GameState::placeBox(Pos p)
{
  assert(isFreeSpace(p));
  auto c = m_map.at(p);
  c = static_cast<Cell>(static_cast<int>(c) | static_cast<int>(Cell::Box));
  m_map.at(p) = c;
}

void GameState::placeUnit(Pos p)
{
  assert(isFreeSpace(p));
  auto c = m_map.at(p);
  c = static_cast<Cell>(static_cast<int>(c) | static_cast<int>(Cell::Unit));
  m_map.at(p) = c;
}

} // namespace soko
