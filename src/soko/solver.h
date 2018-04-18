#pragma once
#include "soko/map.h"
#include "soko/move.h"
#include "soko/heuristic.h"
#include <memory>

namespace soko
{

enum class SolveState
{
  NotSolved,
  Solving,
  Solved
};

class Solver {
public:
  Solver()
    : m_solved(SolveState::NotSolved)
  {}
  ~Solver() {}

  // TODO: add pause, single step, watch current state
  void solve(const Map &map);
  void setHeuristic(std::unique_ptr<Heuristic> &&h) noexcept { m_heuristic = std::move(h); }
  SolveState solved() const noexcept { return m_solved; }
  const std::vector<Move> &result() const noexcept { return m_result; }
  size_t boxMovements() const noexcept { return m_boxMovements; }
  const Heuristic *heuristic() const noexcept { return m_heuristic.get(); }
  void reset() noexcept { m_solved = SolveState::NotSolved; }

private:
  std::unique_ptr<Heuristic> m_heuristic;
  SolveState m_solved;
  size_t m_boxMovements;
  std::vector<Move> m_result;
};

} // namespace soko
