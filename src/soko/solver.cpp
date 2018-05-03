#include "soko/solver.h"
#include <unordered_set>
#include <set>
#include <queue>

#include "soko/solvability.h"
#include "soko/util.h"

namespace soko
{

namespace
{

template<typename T, typename Seq, typename Cmp>
class PriorityQueue : public std::priority_queue<T, Seq, Cmp> {
public:
  PriorityQueue() = default;

  T extract() noexcept
  {
    T result = std::move(this->c.front());
    this->pop();
    return result;
  }
};

size_t sizeHash(const size_t sz) noexcept
{
  std::hash<size_t> hash;
  return hash(sz);
}

struct SavedState
{
  const SavedState *prev;
  MapState state;
};

bool operator==(const SavedState &l, const SavedState &r) noexcept
{
  return l.state.unit == r.state.unit && l.state.boxes == r.state.boxes;
}

struct SavedStateHash
{
  size_t operator()(const SavedState &sp) const noexcept
  {
    const MapState &s = sp.state;
    const size_t ci = 21589;
    const size_t cj = 1;
    size_t hash = 0;
    for (size_t i = 0; i < s.boxes.size(); ++i)
    {
      hash += (i + ci) * sizeHash(s.boxes[i].i) + (i + cj) * sizeHash(s.boxes[i].j);
    }
    return hash ^ (ci * sizeHash(s.unit.i) + cj * sizeHash(s.unit.j));
  }
};


struct QueuedState
{
  QueuedState(const SavedState &state, const Heuristic &h, size_t nMove)
    : calculatedHeuristics(h(state.state))
    , associatedState(&state)
    , nMove(nMove)
  {}

  size_t calculatedHeuristics;
  const SavedState *associatedState;
  size_t nMove;
};

struct CalculatedStateComparator
{
  // Greater
  bool operator()(const QueuedState &left, const QueuedState &right) const
  {
    return left.nMove + left.calculatedHeuristics > right.nMove + right.calculatedHeuristics;
  }
};


// Units are placed into fixed (i->0, j->0) places for easier MapState comparing
Pos topLeft(const Mat<bool> &unitMap)
{
  auto it = std::find(unitMap.begin(), unitMap.end(), true);
  assert(it != unitMap.end());
  return unitMap.iteratorToPos(it);
}

std::vector<Pos> moveBox(const std::vector<Pos> &other, size_t oldIdx, Pos newPos)
{
  std::vector<Pos> result = other;
  result.erase(result.begin() + oldIdx);
  result.insert(std::upper_bound(result.begin(), result.end(), newPos), newPos);
  return result;
}

using BoxMovement = std::pair<Pos, Move>;

BoxMovement restoreSingleStep(const MapState &currentState, const MapState &nextState)
{
  auto &current = currentState.boxes;
  auto &next = nextState.boxes;
  assert(current.size() == next.size());
  std::vector<Pos> diffBoxes;
  std::set_difference(current.begin(), current.end(), next.begin(), next.end(),
                      std::back_inserter(diffBoxes));
  assert(diffBoxes.size() == 1);
  std::set_difference(next.begin(), next.end(), current.begin(), current.end(),
                      std::back_inserter(diffBoxes));
  assert(diffBoxes.size() == 2);

  return {diffBoxes[0], restoreMove(diffBoxes[0], diffBoxes[1])};
}

std::vector<BoxMovement> restoreSteps(const SavedState &last)
{
  assert(last.prev != nullptr);
  std::vector<BoxMovement> result;
  const SavedState *current = &last;
  const SavedState *previous = current->prev;
  while (previous != nullptr)
  {
    result.push_back(restoreSingleStep(previous->state, current->state));
    current = previous;
    previous = previous->prev;
  }
  std::reverse(result.begin(), result.end());
  return result;
}


std::vector<Move> changeRepresentation(const std::vector<BoxMovement> &res, const Map &originalMap)
{
  Map map = originalMap;
  std::vector<Move> result;

  for (auto it : res)
  {
    Pos boxFrom = it.first;
    Move m = it.second;
    Pos boxTo = boxFrom + m;
    Pos unitBeforePush = boxFrom - m;

    auto local = unitPathTo(map, unitBeforePush);
    std::copy(local.begin(), local.end(), std::back_inserter(result));

    extractUnit(map);
    Cell boxFromCell = removeItem(map.at(boxFrom), Cell::Box);
    boxFromCell = placeItem(boxFromCell, Cell::Unit);
    map.at(boxFrom) = boxFromCell;
    map.at(boxTo) = placeItem(map.at(boxTo), Cell::Box);
    result.push_back(m);
  }

  return result;
}

} // namespace


void Solver::solve(const Map &originalMap)
{
  assert(m_heuristic.get() != nullptr);
  m_solved = SolveState::Solving;
  m_heuristic->init(originalMap);

  MapState originalState;
  const MapStatic map = mapToMapStatic(originalMap, &originalState.boxes, &originalState.unit);

  std::unordered_set<SavedState, SavedStateHash> possibleStates;
  PriorityQueue<QueuedState, std::deque<QueuedState>, CalculatedStateComparator> toBeWatched;

  auto solvabilityMap = createSolvabilityMap(map, originalState.boxes.size());
  Mat<bool> unitMap = drawUnitMap(map, originalState.unit, originalState.boxes);
  possibleStates.insert({nullptr, {originalState.boxes, topLeft(unitMap)}});

  if (std::any_of(originalState.boxes.begin(), originalState.boxes.end(),
                  [&solvabilityMap, originalState](Pos p) {
                    return !solvabilityMap.isValid(p, originalState);
                  }))
  {
    return;
  }

  toBeWatched.emplace(*possibleStates.begin(), *m_heuristic, 0);

  while (!toBeWatched.empty())
  {
    auto calculatedState = toBeWatched.extract();
    if (calculatedState.calculatedHeuristics == 0)
    {
      auto boxMoves = restoreSteps(*calculatedState.associatedState);
      m_boxMovements = boxMoves.size();
      m_result = changeRepresentation(boxMoves, originalMap);
      m_solved = SolveState::Solved;
      break;
    }

    auto &state = calculatedState.associatedState->state;
    unitMap = drawUnitMap(map, state.unit, state.boxes); // called twice :(

    for (size_t i = 0; i < state.boxes.size(); ++i)
    {
      auto &box = state.boxes[i];
      for (auto m : {Move::Left, Move::Up, Move::Right, Move::Down})
      {
        Pos unitPushPos = box - m;
        if (!unitMap.contains(unitPushPos) || !unitMap.at(unitPushPos))
        {
          continue;
        }
        auto newPos = box + m;
        if (!safeIsFree(map, newPos, state.boxes))
        {
          continue;
        }

        auto newBoxes = moveBox(state.boxes, i, newPos);
        Pos newUnit = box;
        auto newUnitMap = drawUnitMap(map, newUnit, newBoxes);
        newUnit = topLeft(newUnitMap);

        auto inserted = possibleStates.insert(
            {calculatedState.associatedState, {std::move(newBoxes), newUnit}});
        if (inserted.second && solvabilityMap.isValid(newPos, inserted.first->state))
        {
          toBeWatched.emplace(*inserted.first, *m_heuristic, calculatedState.nMove + 1);
        }
      }
    }
  }
  if (m_solved == SolveState::Solving)
  {
    m_solved = SolveState::NotSolved;
  }
}

} // namespace soko
