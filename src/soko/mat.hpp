#pragma once

#include <cassert>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace soko
{

struct Pos
{
  Pos() = default;
  Pos(size_t i, size_t j)
    : i(i)
    , j(j)
  {}

  size_t i;
  size_t j;
};

inline bool operator<(const Pos &p1, const Pos &p2) { return p1.i < p2.i || (p1.i == p2.i && p1.j < p2.j); }

template<class T, class T2 = typename std::enable_if<!std::is_same<T, bool>::value>::type>
class Mat {
public:
  Mat() = default;
  Mat(const std::vector<std::vector<T>> &map)
  {
    if (map.empty())
    {
      throw std::logic_error("Empty map");
    }

    bool isValid = std::equal(std::next(map.begin()), map.end(), map.begin(),
                              [](auto row1, auto row2) { return row1.size() == row2.size(); });
    if (!isValid)
    {
      throw std::logic_error("Invalid map");
    }

    m_map.reserve(map.size() * map.front().size());
    for (auto row : map)
    {
      std::copy(row.begin(), row.end(), std::back_inserter(m_map));
    }
    m_nrow = map.size();
    m_ncol = map.front().size();
    assert(m_nrow * m_ncol == m_map.size());
  }

  Mat(Mat &&) = default;
  Mat(const Mat &) = default;
  Mat &operator=(Mat &&) = default;
  Mat &operator=(const Mat &) = default;

  bool contains(Pos p) const { return p.i < m_nrow && p.j < m_ncol; }
  bool contains(size_t i, size_t j) const { return contains({i, j}); }

  const T &at(Pos pos) const
  {
    assert(contains(pos.i, pos.j));
    return m_map[pos.i * m_ncol + pos.j];
  }

  T &at(Pos pos)
  {
    assert(contains(pos.i, pos.j));
    return m_map[pos.i * m_ncol + pos.j];
  }

  const T &at(size_t i, size_t j) const { return at({i, j}); }

  T &at(size_t i, size_t j) { return at({i, j}); }

  size_t rows() const { return m_nrow; }
  size_t cols() const { return m_ncol; }
  bool empty() const { return m_map.empty(); }
  void clear()
  {
    m_nrow = m_ncol = 0;
    m_map.clear();
  }

  using Iterator = typename std::vector<T>::iterator;
  using ConstIterator = typename std::vector<T>::const_iterator;
  Iterator begin() { return m_map.begin(); }
  Iterator end() { return m_map.end(); }

  ConstIterator begin() const { return m_map.begin(); }
  ConstIterator end() const { return m_map.end(); }

  Pos numToPos(size_t n) { return {n / m_ncol, n % m_ncol}; }


private:
  size_t m_nrow = 0;
  size_t m_ncol = 0;
  std::vector<T> m_map;
};

} // namespace soko
