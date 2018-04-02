#pragma once
#include <deque>

template<typename T>
class CircularBuffer {
  using Iterator = std::deque<T>::iterator;

  CircularBuffer(size_t sz)
    : m_sz(sz)
  {}

  bool empty() { return m_buf.empty(); }
  bool full() { return m_buf.size() == m_sz; }

private:
  size_t m_sz;
  std::deque<T> m_buf;
  Iterator m_current;
};
