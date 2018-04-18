#include "soko/hungarian_algo.h"
#include "soko/util.h"

#include <queue>

namespace soko
{

std::vector<size_t> HungarianAlgo::solve(const Mat<size_t> &mat)
{
  m_mat = mat;
  prepareMat();

  while (true)
  {
    if (m_maxCardinality.solve(m_adjacent) == m_mat.rows())
    {
      return m_maxCardinality.transformedMapping();
    }
    zeroes(m_maxCardinality.mapping());
    alphaTransformation();
  }
}

void HungarianAlgo::prepareMat()
{
  m_adjacent.assign(m_mat.rows() + m_mat.cols(), {});

  for (size_t i = 0; i < m_mat.rows(); ++i)
  {
    size_t min = g_inf;
    for (size_t j = 0; j < m_mat.cols(); ++j)
    {
      min = std::min(min, m_mat.at(i, j));
    }
    // TODO: not check min here to set up adjacent values
    for (size_t j = 0; j < m_mat.cols(); ++j)
    {
      m_mat.at(i, j) -= min;
      if (m_mat.at(i, j) == 0)
      {
        addAdjacent(i, j);
      }
    }
  }

  for (size_t j = 0; j < m_mat.cols(); ++j)
  {
    size_t min = g_inf;
    for (size_t i = 0; i < m_mat.rows(); ++i)
    {
      min = std::min(min, m_mat.at(i, j));
    }
    if (min == 0)
    {
      continue;
    }
    for (size_t i = 0; i < m_mat.rows(); ++i)
    {
      m_mat.at(i, j) -= min;
      if (m_mat.at(i, j) == 0)
      {
        addAdjacent(i, j);
      }
    }
  }
}

void HungarianAlgo::zeroesSingle(const std::vector<size_t> &cardinality, size_t row)
{
  if (m_rowZeroes[row] == false)
  {
    return;
  }
  m_rowZeroes[row] = false;
  auto &neighbors = m_adjacent[row];
  for (auto it : neighbors)
  {
    if (cardinality[it] == row || m_colZeroes[it - m_rowZeroes.size()] == true)
    {
      continue;
    }
    m_colZeroes[it - m_rowZeroes.size()] = true;
    if (cardinality[it] != cardinality.size())
    {
      zeroesSingle(cardinality, cardinality[it]);
    }
  }
}


void HungarianAlgo::zeroes(const std::vector<size_t> &cardinality)
{
  m_rowZeroes.assign(cardinality.size() / 2, true);
  m_colZeroes.assign(cardinality.size() / 2, false);

  for (size_t i = 0; i < cardinality.size() / 2; ++i)
  {
    if (cardinality[i] != cardinality.size())
    {
      continue;
    }
    zeroesSingle(cardinality, i);
  }
}

void HungarianAlgo::alphaTransformation()
{
  size_t minimum = g_inf;
  for (size_t i = 0; i < m_mat.rows(); ++i)
  {
    if (m_rowZeroes[i])
    {
      continue;
    }
    for (size_t j = 0; j < m_mat.cols(); ++j)
    {
      if (!m_colZeroes[j])
      {
        minimum = std::min(minimum, m_mat.at(i, j));
      }
    }
  }
  assert(minimum != 0);

  for (size_t i = 0; i < m_mat.rows(); ++i)
  {
    for (size_t j = 0; j < m_mat.cols(); ++j)
    {
      if (!m_rowZeroes[i] && !m_colZeroes[j])
      {
        m_mat.at(i, j) -= minimum;
        if (m_mat.at(i, j) == 0)
        {
          addAdjacent(i, j);
        }
      }
      else if (m_rowZeroes[i] && m_colZeroes[j])
      {
        bool wasAdj = m_mat.at(i, j) == 0;
        if (m_mat.at(i, j) != g_inf)
        {
          m_mat.at(i, j) += minimum;
        }
        if (wasAdj)
        {
          removeFromAdjacent(i, j);
        }
      }
    }
  }
}

void HungarianAlgo::addAdjacent(size_t i, size_t j)
{
  size_t col = m_mat.rows() + j;
  assert(std::find(m_adjacent[i].begin(), m_adjacent[i].end(), col) == m_adjacent[i].end());
  m_adjacent[i].push_back(col);
  m_adjacent[col].push_back(i);
}

void HungarianAlgo::removeFromAdjacent(size_t i, size_t j)
{
  size_t col = m_mat.rows() + j;
  auto it = std::find(m_adjacent[i].begin(), m_adjacent[i].end(), col);
  if (it == m_adjacent[i].end())
  {
    return;
  }
  m_adjacent[i].erase(it);
  m_adjacent[col].erase(std::find(m_adjacent[col].begin(), m_adjacent[col].end(), i));
}

size_t HopcroftKarp::solve(const AdjacencyList &m)
{
  m_nil = m.size();

  m_distance.assign(m_nil + 1, g_inf);
  m_mapping.assign(m_nil, m_nil);

  size_t result = 0;
  while (bfs(m))
  {
    for (size_t i = 0; i < m.size() / 2; ++i)
    {
      if (m_mapping[i] == m_nil)
      {
        if (dfs(m, i))
        {
          ++result;
        }
      }
    }
  }
  return result;
}

std::vector<size_t> HopcroftKarp::transformedMapping() const
{
  const size_t amountRows = m_mapping.size() / 2;

  std::vector<size_t> result = m_mapping;
  result.resize(amountRows);
  for (auto &it : result)
  {
    it -= amountRows;
  }
  return result;
}

bool HopcroftKarp::dfs(const AdjacencyList &adjacent, size_t row)
{
  if (row == m_nil)
  {
    return true;
  }
  for (auto item : adjacent[row])
  {
    if (m_distance[m_mapping[item]] == m_distance[row] + 1)
    {
      if (dfs(adjacent, m_mapping[item]))
      {
        m_mapping[row] = item;
        m_mapping[item] = row;
        return true;
      }
    }
  }
  m_distance[row] = g_inf;
  return false;
}

bool HopcroftKarp::bfs(const AdjacencyList &adjacent)
{
  const size_t rows = adjacent.size() / 2;

  std::queue<size_t> q;
  for (size_t i = 0; i < rows; ++i)
  {
    if (m_mapping[i] == m_nil)
    {
      m_distance[i] = 0;
      q.push(i);
    }
    else
    {
      m_distance[i] = g_inf;
    }
  }

  m_distance[m_nil] = g_inf;

  while (!q.empty())
  {
    size_t u = q.front();
    q.pop();
    if (m_distance[u] < m_distance[m_nil])
    {
      for (auto item : adjacent[u])
      {
        if (m_distance[m_mapping[item]] == g_inf)
        {
          m_distance[m_mapping[item]] = m_distance[u] + 1;
          q.push(m_mapping[item]);
        }
      }
    }
  }
  return m_distance[m_nil] != g_inf;
}

} // namespace soko
