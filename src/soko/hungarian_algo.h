#pragma once
#include "soko/mat.hpp"

namespace soko
{
using AdjacencyList = std::vector<std::vector<size_t>>;

class HopcroftKarp {
public:
  HopcroftKarp() = default;

  size_t solve(const AdjacencyList &m);
  const std::vector<size_t> &mapping() const { return m_mapping; }
  std::vector<size_t> transformedMapping() const;

private:
  bool dfs(const AdjacencyList &m, size_t row);
  bool bfs(const AdjacencyList &m);

private:
  size_t m_nil;
  std::vector<size_t> m_distance;
  std::vector<size_t> m_mapping;
};


class HungarianAlgo {
public:
  HungarianAlgo() = default;
  HungarianAlgo(const HungarianAlgo &other) = delete;
  HungarianAlgo &operator=(const HungarianAlgo &) = delete;

  std::vector<size_t> solve(const Mat<size_t> &mat);

private:
  void prepareMat();
  void zeroesSingle(const std::vector<size_t> &cardinality, size_t row);
  void zeroes(const std::vector<size_t> &cardinality);
  void alphaTransformation();

  void addAdjacent(size_t i, size_t j);
  void removeFromAdjacent(size_t i, size_t j);


private:
  Mat<size_t> m_mat;

  std::vector<std::vector<size_t>> m_adjacent;
  HopcroftKarp m_maxCardinality;
  std::vector<bool> m_rowZeroes;
  std::vector<bool> m_colZeroes;
};

} // namespace soko
