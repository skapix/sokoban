#pragma once
#include <functional>

#include "soko/map.h"
#include "soko/heuristic.h"

namespace soko
{

using SolvabilityPredicate = std::function<bool(const MapState &)>;
using SolvabilityCell = std::vector<SolvabilityPredicate>;

class SolvabilityMap : public Mat<SolvabilityCell> {
public:
  SolvabilityMap(size_t rows, size_t cols, const SolvabilityCell &value = {}) noexcept
    : Mat(rows, cols, value)
  {}

  SolvabilityMap(SolvabilityMap &&other) = default;

  // Pos is the position, where the latest moved box was placed
  bool isValid(Pos p, const MapState &m) const noexcept
  {
    return std::all_of(at(p).begin(), at(p).end(), [&m](auto fn) { return fn(m); });
  }

private:
  // map, which will be used by predicates
  // MapStatic m_map;
};


SolvabilityMap createSolvabilityMap(const MapStatic &m, size_t nBoxes) noexcept;

} // namespace soko
