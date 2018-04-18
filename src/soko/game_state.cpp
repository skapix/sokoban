#include "soko/game_state.h"
#include "soko/util.h"

namespace soko
{

GameState::GameState(const Map &map) noexcept
  : m_map(map)
{
  auto it = std::find(m_map.begin(), m_map.end(), Cell::Unit);
  assert(it != map.end());
  assert(map.rows() >= 1);
  assert(map.cols() >= 1);
  m_unit = m_map.iteratorToPos(it);
}

bool GameState::canMove(Move direction) const noexcept
{
  Pos move1 = m_unit + direction;
  Pos move2 = m_unit + direction + direction;

  bool canMove1 = m_map.safeIsFree(move1);
  bool canMove2 = m_map.safeIsBox(move1) && m_map.safeIsFree(move2);

  return canMove1 || canMove2;
}

MoveDirection GameState::move(Move direction) noexcept
{
  int h = toHorizontal(direction);
  int v = toVertical(direction);

  Pos possibleBoxFrom(v, h);
  Pos possibleBoxTo(2 * v, 2 * h);

  if (!canMove(direction))
  {
    return {MoveResult::NoMove, direction};
  }

  bool boxMoved = moveIfBoxTo(m_unit + possibleBoxFrom, m_unit + possibleBoxTo);
  moveUnitTo(m_unit + possibleBoxFrom);
  return {boxMoved ? MoveResult::UnitBoxMove : MoveResult::UnitMove, direction};
}

MoveResult GameState::undo(MoveDirection r) noexcept
{
  if (r.result == MoveResult::NoMove)
  {
    return MoveResult::NoMove;
  }

  Pos boxFrom = m_unit + r.move;
  Pos boxTo = m_unit;
  Pos unitTo = m_unit - r.move;

  moveUnitTo(unitTo);
  if (r.result == MoveResult::UnitBoxMove)
  {
    [[maybe_unused]] bool result = moveIfBoxTo(boxFrom, boxTo);
    assert(result);
    return MoveResult::UnitBoxMove;
  }
  return MoveResult::UnitMove;
}

bool GameState::isWinningState() const noexcept
{
  auto pred = [](Cell c) { return c == Cell::Box || c == Cell::Destination; };
  return std::find_if(m_map.begin(), m_map.end(), pred) == m_map.end();
}

bool GameState::moveIfBoxTo(Pos from, Pos to) noexcept
{
  if (!m_map.safeIsBox(from))
  {
    return false;
  }

  m_map.at(from) = removeItem(m_map.at(from), Cell::Box);
  m_map.at(to) = placeItem(m_map.at(to), Cell::Box);
  return true;
}

void GameState::moveUnitTo(Pos to) noexcept
{
  assert(m_map.isUnit(m_unit));
  m_map.at(m_unit) = removeItem(m_map.at(m_unit), Cell::Unit);
  m_unit = to;
  m_map.at(m_unit) = placeItem(m_map.at(m_unit), Cell::Unit);
}

} // namespace soko
