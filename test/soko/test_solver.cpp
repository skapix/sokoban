#include <gtest/gtest.h>
#include "soko/solver.h"
#include "soko/util.h"

namespace soko
{

namespace test
{

TEST(solver, simpleSolverTest)
{
  std::vector<std::vector<Cell>> rawM = {{Cell::Wall, Cell::Field, Cell::Field},
                                         {Cell::Unit, Cell::Box, Cell::Field},
                                         {Cell::Wall, Cell::Field, Cell::Destination}};


  Map map(rawM);
  Solver s;
  s.setHeuristic(Heuristic::create(HeuristicType::HungarianTaxicab));
  s.solve(map);
  auto result = s.result();
  ASSERT_TRUE(s.solved() == SolveState::Solved);
  ASSERT_EQ(std::vector<Move>({Move::Right, Move::Up, Move::Right, Move::Down}), result);
}

} // namespace test

} // namespace soko
