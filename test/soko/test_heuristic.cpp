#include <gtest/gtest.h>
#include "soko/heuristic.h"
#include "soko/util.h"

namespace soko
{

namespace test
{

namespace
{
size_t calculateHeuristic(Heuristic *h, const Map &map)
{
  auto boxes = getBoxes(map);
  auto unit = getUnit(map);
  h->init(map);
  return (*h)({boxes, unit});
}
} // namespace

TEST(heuristic, SimpleHungarianHeuristicTest)
{
  std::vector<std::vector<Cell>> rawM = {{Cell::Wall, Cell::Field, Cell::Field},
                                         {Cell::Unit, Cell::Box, Cell::Field},
                                         {Cell::Wall, Cell::Field, Cell::Destination}};


  Map map(rawM);
  std::unique_ptr<Heuristic> h = Heuristic::create(HeuristicType::HungarianTaxicab);
  size_t real = calculateHeuristic(h.get(), map);

  ASSERT_EQ(2, real);
}
TEST(heuristic, ZeroDistanceHungarianHeuristicTest)
{
  std::vector<std::vector<Cell>> rawM = {{Cell::Wall, Cell::Field, Cell::Field},
                                         {Cell::Unit, Cell::Field, Cell::Field},
                                         {Cell::Wall, Cell::Field, Cell::BoxDestination}};

  Map map(rawM);
  std::unique_ptr<Heuristic> h = Heuristic::create(HeuristicType::HungarianTaxicab);
  size_t real = calculateHeuristic(h.get(), map);
  ASSERT_EQ(0, real);
}

} // namespace test

} // namespace soko
