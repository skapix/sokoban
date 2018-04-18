#pragma once

#include <cstddef>
#include <vector>
#include <memory>

#include "soko/map.h"

namespace soko
{

struct MapState
{
  std::vector<Pos> boxes;
  Pos unit;
};

enum class HeuristicType
{
  HungarianTaxicab,
  HungarianTaxicabPush,
};

class Heuristic {
public:
  static std::unique_ptr<Heuristic> create(HeuristicType type);
  Heuristic() = default;
  virtual void init(const Map &m) noexcept;

  // TODO: save some space: heuristic should have uint32_t result
  virtual size_t operator()(const MapState &boxes) const noexcept = 0;
  virtual std::string name() const noexcept = 0;
  bool inited() const noexcept { return m_inited; }
  void deinit() noexcept
  {
    m_map.clear();
    m_inited = false;
  }

  virtual ~Heuristic() {}

protected:
  MapStatic m_map;
  bool m_inited = false;
};


} // namespace soko
