#include "soko/heuristic.h"
#include "soko/move.h"
#include "soko/util.h"
#include "soko/hungarian_algo.h"
#include <queue>

namespace soko
{

namespace
{

constexpr std::array<Move, 4> g_moves = {Move::Left, Move::Right, Move::Up, Move::Down};

Mat<size_t> createDistanceMat(const MapStatic &m, const Pos from)
{
  Mat<size_t> result(std::vector<size_t>(m.rows() * m.cols(), g_inf), m.cols());

  result.at(from) = 0;

  std::queue<Pos> observe;
  observe.push(from);
  while (!observe.empty())
  {
    Pos cur = observe.front();
    observe.pop();
    for (auto move : g_moves)
    {
      Pos newPos = cur + move;
      Pos fromPos = newPos + move;
      if (m.safeIsFree(fromPos) && result.contains(newPos) && result.at(newPos) == g_inf &&
          m.isFree(newPos))
      {
        result.at(newPos) = result.at(cur) + 1;
        observe.push(newPos);
      }
    }
  }


  return result;
}

Mat<size_t> createExtendedDistanceMat(const MapStatic &m, const Pos from)
{
  // TODO: add real box movement by unit
  // TODO: create soko solver engine
  Mat<size_t> result(std::vector<size_t>(m.rows() * m.cols(), g_inf), m.cols());

  result.at(from) = 0;

  std::queue<Pos> observe;
  observe.push(from);
  while (!observe.empty())
  {
    Pos cur = observe.front();
    observe.pop();
    for (auto move : g_moves)
    {
      Pos fromPos = cur - move;
      Pos newPos = cur + move;
      if (m.safeIsFree(fromPos) && result.contains(newPos) && result.at(newPos) == g_inf &&
          m.isFree(newPos))
      {
        result.at(newPos) = result.at(cur) + 1;
        observe.push(newPos);
      }
    }
  }

  return result;
}

class HungarianHeuristic : public Heuristic {
public:
  using ShortestPathsPos = Mat<size_t>;
  using ShortestPaths = std::pair<Pos, ShortestPathsPos>;

  HungarianHeuristic(bool extendedDistance) noexcept
    : m_extendedDistance(extendedDistance)
  {}
  virtual void init(const Map &m) noexcept override;
  virtual std::string name() const noexcept override { return "Hungarian"; }

  virtual size_t operator()(const MapState &boxes) const noexcept override;

private:
  const bool m_extendedDistance;
  std::vector<ShortestPaths> m_destinationsPaths;

  mutable HungarianAlgo m_algo;
};

std::vector<HungarianHeuristic::ShortestPaths> createDestinationMat(const MapStatic &m,
                                                                    bool extended)
{
  std::vector<HungarianHeuristic::ShortestPaths> result;
  for (size_t i = 0; i < m.rows(); ++i)
  {
    for (size_t j = 0; j < m.cols(); ++j)
    {
      if (m.at(i, j) == Cell::Destination)
      {
        Pos cur = {i, j};
        auto distanceMap = extended ? createExtendedDistanceMat(m, cur) : createDistanceMat(m, cur);
        result.push_back({cur, std::move(distanceMap)});
      }
    }
  }
  return result;
}

size_t sumElems(const Mat<size_t> &m, const std::vector<size_t> &p)
{
  size_t result = 0;
  for (size_t i = 0; i < p.size(); ++i)
  {
    result += m.at(i, p[i]);
  }
  return result;
}


void HungarianHeuristic::init(const Map &m) noexcept
{
  Heuristic::init(m);
  m_destinationsPaths = createDestinationMat(m_map, m_extendedDistance);
}

size_t HungarianHeuristic::operator()(const MapState &state) const noexcept
{
  auto &boxes = state.boxes;
  assert(boxes.size() == m_destinationsPaths.size());

  std::vector<size_t> resultVec;
  resultVec.reserve(boxes.size() * boxes.size());

  std::vector<Pos> filterPoses;
  //  for (auto box : boxes)
  //  {
  //    if (m_map.isDestination(box))
  //  }

  for (auto box : boxes)
  {
    [[maybe_unused]] auto min = g_inf;
    for (auto dest : m_destinationsPaths)
    {
      resultVec.push_back(dest.second.at(box));
      min = std::min(min, dest.second.at(box));
    }
    // One of boxes can't reach any destination. This condition should be caught earlier.
    assert(min != g_inf);
  }

  Mat<size_t> resultMat(std::move(resultVec), boxes.size());

  auto resultArr = m_algo.solve(resultMat);
  size_t result = sumElems(resultMat, resultArr);
  assert(result < g_inf / 100); // we need this result to be sured that no overflow happened
  return result;
}

} // namespace

std::unique_ptr<Heuristic> Heuristic::create(HeuristicType type)
{
  switch (type)
  {
  case HeuristicType::HungarianTaxicab:
    return std::make_unique<HungarianHeuristic>(false);
  case HeuristicType::HungarianTaxicabPush:
    return std::make_unique<HungarianHeuristic>(true);
  default:
    assert(false);
    return nullptr;
  }
}

void Heuristic::init(const Map &m) noexcept
{
  m_map = mapToMapStatic(m); // removeMovable(m);
  m_inited = true;
}


} // namespace soko
