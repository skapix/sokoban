#include <gtest/gtest.h>
#include "soko/hungarian_algo.h"

namespace soko
{

namespace test
{

namespace
{

std::vector<std::vector<size_t>> toAdjacency(const std::vector<std::vector<bool>> &m)
{
  if (m.size() == 0)
  {
    return {};
  }

  std::vector<std::vector<size_t>> result;
  result.resize(m.size() + m.front().size());

  for (size_t i = 0; i < m.size(); ++i)
  {
    for (size_t j = 0; j < m[i].size(); ++j)
    {
      if (m[i][j])
      {
        result[i].push_back(m.size() + j);
        result[m.size() + j].push_back(i);
      }
    }
  }
  return result;
}

} // namespace

TEST(hungarian, HopcroftKarp_test)
{
  HopcroftKarp algo;
  std::vector<std::vector<bool>> data = {
      {0, 1, 1},
      {0, 1, 0},
      {1, 0, 1},
  };
  algo.solve(toAdjacency(data));
  auto result = algo.transformedMapping();

  EXPECT_EQ(std::vector<size_t>({2, 1, 0}), result);


  data = std::vector<std::vector<bool>>({
      {0, 0, 1, 0, 1},
      {0, 0, 1, 0, 1},
      {1, 1, 0, 1, 0},
      {0, 1, 1, 1, 1},
      {0, 0, 0, 1, 0},
  });

  algo.solve(toAdjacency(data));
  result = algo.transformedMapping();
  EXPECT_EQ(std::vector<size_t>({2, 4, 0, 1, 3}), result);


  data = std::vector<std::vector<bool>>({
      {1, 0, 0},
      {0, 0, 1},
      {0, 0, 0},
  });

  algo.solve(toAdjacency(data));
  result = algo.transformedMapping();
  EXPECT_EQ(std::vector<size_t>({0, 2, 3}), result);

  data = std::vector<std::vector<bool>>({
      {1, 0, 0},
      {0, 0, 0},
      {0, 0, 1},
  });

  algo.solve(toAdjacency(data));
  result = algo.transformedMapping();
  EXPECT_EQ(std::vector<size_t>({0, 3, 2}), result);
}


TEST(hungarian, HungarianAlgo_test)
{
  HungarianAlgo algo;

  std::vector<std::vector<size_t>> data = {{32, 28, 4, 26, 4},
                                           {17, 19, 4, 17, 4},
                                           {4, 4, 5, 4, 4},
                                           {17, 14, 4, 14, 4},
                                           {21, 16, 4, 13, 4}};
  auto result = algo.solve(data);
  EXPECT_EQ(std::vector<size_t>({2, 4, 0, 1, 3}), result);

  data = std::vector<std::vector<size_t>>{
      {4, 6, 8},
      {7, 5, 6},
      {1, 8, 6},
  };
  result = algo.solve(data);
  EXPECT_EQ(std::vector<size_t>({1, 2, 0}), result);

  data = {{0, 5}, {4, 10}};
  result = algo.solve(data);
  EXPECT_EQ(std::vector<size_t>({1, 0}), result);
}

} // namespace test

} // namespace soko
