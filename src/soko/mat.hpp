#pragma once

#include <cassert>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "soko/pos.h"

namespace soko
{

template<class T>
class Mat {
public:
  Mat() = default;
  Mat(std::vector<T> &&map, size_t cols)
    : m_map(std::move(map))
    , m_nrow(cols == 0 ? 0 : m_map.size() / cols)
    , m_ncol(cols)
  {
    if (m_ncol == 0 && !m_map.empty())
    {
      throw std::logic_error("Invalid map");
    }

    if (m_map.size() % m_ncol != 0)
    {
      throw std::logic_error("Invalid map");
    }
  }
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

  Mat(size_t rows, size_t cols, const T &value = {}) noexcept
    : m_map(std::vector<T>(rows * cols, value))
    , m_nrow(rows)
    , m_ncol(cols)
  {}

  Mat(Mat &&) = default;
  Mat(const Mat &) = default;
  Mat &operator=(Mat &&) = default;
  Mat &operator=(const Mat &) = default;

  bool contains(Pos p) const noexcept { return p.i < m_nrow && p.j < m_ncol; }
  bool contains(size_t i, size_t j) const noexcept { return contains({i, j}); }

  const T &at(Pos pos) const noexcept
  {
    assert(contains(pos.i, pos.j));
    return m_map[pos.i * m_ncol + pos.j];
  }

  T &at(Pos pos) noexcept
  {
    assert(contains(pos.i, pos.j));
    return m_map[pos.i * m_ncol + pos.j];
  }

  const T &at(size_t i, size_t j) const noexcept { return at({i, j}); }

  T &at(size_t i, size_t j) noexcept { return at({i, j}); }

  constexpr size_t rows() const noexcept { return m_nrow; }
  constexpr size_t cols() const noexcept { return m_ncol; }
  bool empty() const noexcept { return m_map.empty(); }
  void clear() noexcept
  {
    m_nrow = m_ncol = 0;
    m_map.clear();
  }

  using Iterator = typename std::vector<T>::iterator;
  using ConstIterator = typename std::vector<T>::const_iterator;
  Iterator begin() noexcept { return m_map.begin(); }
  Iterator end() noexcept { return m_map.end(); }

  ConstIterator begin() const noexcept { return m_map.begin(); }
  ConstIterator end() const noexcept { return m_map.end(); }

  Pos iteratorToPos(ConstIterator it) const noexcept
  {
    size_t n = it - m_map.begin();
    assert(n <= m_nrow * m_ncol);
    return {n / m_ncol, n % m_ncol};
  }

private:
  std::vector<T> m_map;
  size_t m_nrow = 0;
  size_t m_ncol = 0;
};

// Copy-paste :(
template<>
class Mat<bool> {
public:
  Mat() = default;
  Mat(std::vector<bool> &&map, size_t cols)
    : m_map(std::move(map))
    , m_nrow(cols == 0 ? 0 : map.size() / cols)
    , m_ncol(cols)
  {
    if (cols == 0 && !map.empty())
    {
      throw std::logic_error("Invalid map");
    }

    if (map.size() % cols != 0)
    {
      throw std::logic_error("Invalid map");
    }
  }
  Mat(const std::vector<std::vector<bool>> &map)
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

  Mat(size_t rows, size_t cols, bool value = {})
    : m_map(std::vector<bool>(rows * cols, value))
    , m_nrow(rows)
    , m_ncol(cols)
  {}

  Mat(Mat &&) = default;
  Mat(const Mat &) = default;
  Mat &operator=(Mat &&) = default;
  Mat &operator=(const Mat &) = default;

  bool contains(Pos p) const { return p.i < m_nrow && p.j < m_ncol; }
  bool contains(size_t i, size_t j) const { return contains({i, j}); }

  bool at(Pos pos) const
  {
    assert(contains(pos.i, pos.j));
    return m_map[pos.i * m_ncol + pos.j];
  }

  void set(Pos pos)
  {
    assert(contains(pos));
    m_map[pos.i * m_ncol + pos.j] = true;
  }
  void set(size_t i, size_t j) { set({i, j}); }

  void unset(Pos pos)
  {
    assert(contains(pos));
    m_map[pos.i * m_ncol + pos.j] = false;
  }
  void unset(size_t i, size_t j) { unset({i, j}); }

  bool at(size_t i, size_t j) const { return at({i, j}); }

  size_t rows() const { return m_nrow; }
  size_t cols() const { return m_ncol; }
  bool empty() const { return m_map.empty(); }
  void clear()
  {
    m_nrow = m_ncol = 0;
    m_map.clear();
  }

  using Iterator = typename std::vector<bool>::iterator;
  using ConstIterator = typename std::vector<bool>::const_iterator;
  Iterator begin() { return m_map.begin(); }
  Iterator end() { return m_map.end(); }

  ConstIterator begin() const { return m_map.begin(); }
  ConstIterator end() const { return m_map.end(); }

  Pos iteratorToPos(ConstIterator it) const
  {
    size_t n = it - m_map.begin();
    assert(n <= m_nrow * m_ncol);
    return {n / m_ncol, n % m_ncol};
  }

  Pos numToPos(size_t n) const { return {n / m_ncol, n % m_ncol}; }


private:
  std::vector<bool> m_map;
  size_t m_nrow = 0;
  size_t m_ncol = 0;
};

} // namespace soko
