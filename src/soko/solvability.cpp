#include "soko/solvability.h"
#include "soko/util.h"

#include <array>

namespace soko
{

namespace
{

Pos moveTillWall(const MapStatic &m, Pos p, Move move) noexcept
{
  assert(!m.safeIsWall(p));
  while (!m.safeIsWall(p + move))
  {
    p += move;
  }
  return p;
}

constexpr std::array<Move, 4> g_moves = {Move::Left, Move::Up, Move::Right, Move::Down};

bool cantBePlaced(const MapState &) noexcept { return false; }

SolvabilityPredicate lineRestriction(const MapStatic &m, Pos p, Move move) noexcept
{
  assert(move == Move::Right || move == Move::Down);
  Pos bound1 = moveTillWall(m, p, reverse(move));
  Pos bound2 = moveTillWall(m, p, move);
  size_t destinations = 0;
  for (p = bound1; p <= bound2; p += move)
  {
    if (m.isDestination(p))
    {
      ++destinations;
    }
  }
  return SolvabilityPredicate([bound1, bound2, move, destinations](const MapState &ms) noexcept {
    size_t boxes = 0;
    // TODO: look up all boxes is more profitable
    for (Pos p = bound1; p <= bound2 && boxes <= destinations; p += move)
    {
      if (isBox(p, ms.boxes))
      {
        ++boxes;
      }
    }
    return boxes <= destinations;
  });
}

SolvabilityPredicate invalidSingle(Pos p) noexcept
{
  return SolvabilityPredicate([p](const MapState &state) noexcept {
    return !isBox(p, state.boxes);
  });
}

SolvabilityPredicate invalidPair(Pos pos1, Pos pos2) noexcept
{
  auto [p1, p2] = std::minmax(pos1, pos2);
  // :(
  return SolvabilityPredicate([p1, p2](const MapState &state) noexcept {
    // Method 1
    //    auto it1 = std::lower_bound(state.boxes.begin(), state.boxes.end(), p1);
    //    bool result1 = it1 == state.boxes.end() || *it1 != p1;
    //    auto it2 = std::lower_bound(std::next(it1), state.boxes.end(), p2);
    //    return result1 || it2 == state.boxes.end() || *it2 != p2;
    // Method 2
    //        auto it1 = std::lower_bound(state.boxes.begin(), state.boxes.end(), p1);
    //        if (it1 == state.boxes.end() || *it1 != p1)
    //        {
    //          return true;
    //        }
    //        auto it2 = std::lower_bound(std::next(it1), state.boxes.end(), p2);
    //        return it2 == state.boxes.end() || *it2 != p2;
    // Method 3
    return !(isBox(p1, state.boxes) && isBox(p2, state.boxes));
  });
}

SolvabilityPredicate invalidTriple(Pos p1, Pos p2, Pos p3) noexcept
{

  //  auto [p1, p2] = std::minmax(pos1, pos2);
  //  Pos p3;
  //  std::tie(p1, p3) = std::minmax(pos1, pos3);
  //  std::tie(p2, p3) = std::minmax(pos2, pos3);
  // TODO: replace with more smart search...

  return SolvabilityPredicate([p1, p2, p3](const MapState &state) noexcept {
    //    auto &boxes = state.boxes;
    //    auto it1 = std::lower_bound(boxes.begin(), boxes.end(), p1);
    //    if (it1 == boxes.end() || *it1 != p1)
    //    {
    //      return true;
    //    }
    //    while (++it1 != boxes.end() && *it1 < p2)
    //      ;
    //    if (it1 == boxes.end() || *it1 != p2)
    //    {
    //      return true;
    //    }
    //    while (++it1 != boxes.end() && *it1 < p3)
    //      ;
    //    if (it1 == boxes.end() || *it1 != p3)
    //    {
    //      return true;
    //    }
    //    return false;


    return !(isBox(p1, state.boxes) && isBox(p2, state.boxes) && isBox(p3, state.boxes));
  });
}

bool isCornerNoDest(const MapStatic &m, Pos p) noexcept
{
  assert(!m.isWall(p));
  if (m.isDestination(p))
  {
    return false;
  }

  bool result = false;
  for (size_t i = 0; i < g_moves.size(); ++i)
  {
    result = result ||
             (m.safeIsWall(p + g_moves[i]) && m.safeIsWall(p + clockwiseRotate(g_moves[i])));
  }
  return result;
}

bool isSquare2Box2Wall(const MapStatic &m, Pos p, Move move) noexcept
{
  // also notices following structure:
  // wall   space
  // box    box
  // space  wall
  assert(m.isFree(p));
  Pos p2 = p + move;
  if (!m.safeIsFree(p2))
  {
    return false;
  }
  if (isCornerNoDest(m, p2))
  {
    // no sence in checking corner, because they are skipped
    return false;
  }
  if (m.safeIsDestination(p) && m.safeIsDestination(p2))
  {
    return false;
  }

  Move m1 = clockwiseRotate(move);
  Move m2 = reverse(m1);

  bool result = false;
  for (auto side : {m1, m2})
  {
    result = result || (m.safeIsWall(p + side) && (m.safeIsWall(p2 + m1) || m.safeIsWall(p2 + m2)));
  }
  return result;
}

bool isLineDeadEnd(const MapStatic &m, Pos p, std::array<Move, 4> moves) noexcept
{
  bool wallsFromOneSide = true;
  bool wallsFromOtherSide = true;
  p = moveTillWall(m, p, moves[0]);

  for (; !m.safeIsWall(p); p += moves[1])
  {
    if (!m.safeIsWall(p + moves[2]))
    {
      wallsFromOneSide = false;
    }
    if (!m.safeIsWall(p + moves[3]))
    {
      wallsFromOtherSide = false;
    }
  }
  return wallsFromOneSide || wallsFromOtherSide;
}

std::vector<SolvabilityPredicate> square3Box1Wall(const MapStatic &m, Pos p, Move move)
{
  assert(m.safeIsFree(p));
  std::vector<SolvabilityPredicate> result;
  auto pWall = p + move;
  if (!m.safeIsWall(pWall))
  {
    return {};
  }
  for (auto firstMove : {clockwiseRotate(move), reverse(clockwiseRotate(move))})
  {
    Pos p2 = p + firstMove;
    Pos p3 = p2 + move;
    if (m.safeIsDestination(p) && m.safeIsDestination(p2) && m.safeIsDestination(p3))
    {
      continue;
    }
    if (m.safeIsFree(p2) && m.safeIsFree(p3))
    {
      result.push_back(invalidPair(p2, p3));
    }
  }
  return result;
}

std::vector<SolvabilityPredicate> square3Box1WallDiag(const MapStatic &m, Pos p, Move move)
{
  assert(m.safeIsFree(p));
  auto move2 = clockwiseRotate(move);
  Pos wall = p + move + move2;
  if (!m.safeIsWall(wall))
  {
    return {};
  }
  Pos p2 = p + move;
  Pos p3 = p + move2;
  if (m.safeIsDestination(p) && m.safeIsDestination(p2) && m.safeIsDestination(p3))
  {
    return {};
  }
  if (m.safeIsFree(p2) && m.safeIsFree(p3))
  {
    return {invalidPair(p2, p3)};
  }
  return {};
}

std::vector<SolvabilityPredicate> isSquare4box(const MapStatic &m, Pos p, Move move)
{
  std::array<Pos, 4> poses;
  poses[0] = p;
  assert(m.safeIsFree(p));
  auto move2 = clockwiseRotate(move);
  poses[1] = poses[0] + move;
  poses[2] = poses[0] + move2;
  poses[3] = poses[1] + move2;
  std::sort(poses.begin(), poses.end());
  if (std::all_of(poses.begin(), poses.end(), [&m](Pos p) { return m.safeIsDestination(p); }))
  {
    return {};
  }
  if (std::any_of(poses.begin(), poses.end(), [&m](Pos p) { return !m.safeIsFree(p); }))
  {
    return {};
  }
  std::array<std::pair<Pos, Pos>, 4> facets = {
      std::make_pair(poses[0] + Move::Left, poses[2] + Move::Left),
      std::make_pair(poses[0] + Move::Up, poses[1] + Move::Up),
      std::make_pair(poses[1] + Move::Right, poses[3] + Move::Right),
      std::make_pair(poses[2] + Move::Down, poses[3] + Move::Down)};
  for (auto f : facets)
  {
    // if wall is near => meaningless to add extra condition
    if (m.safeIsWall(f.first) && m.safeIsWall(f.second))
    {
      return {};
    }
  }

  std::remove(poses.begin(), poses.end(), p); // p is now the last
  return {invalidTriple(poses[0], poses[1], poses[2])};
}

} // namespace

SolvabilityMap createSolvabilityMap(const MapStatic &m, size_t nBoxes) noexcept
{
  SolvabilityMap result(m.rows(), m.cols());
  for (auto it = m.begin(); it != m.end(); ++it)
  {
    Pos p = m.iteratorToPos(it);
    if (m.isWall(p))
    {
      continue;
    }
    if (isCornerNoDest(m, p))
    {
      result.at(p).push_back(cantBePlaced);
      // no sence in adding any other restrictions for this cell
      continue;
    }

    if (isLineDeadEnd(m, p, {Move::Left, Move::Right, Move::Up, Move::Down}))
    {
      // horizontal dead end
      result.at(p).push_back(lineRestriction(m, p, Move::Right));
    }

    if (isLineDeadEnd(m, p, {Move::Up, Move::Down, Move::Left, Move::Right}))
    {
      // vertical dead end
      result.at(p).push_back(lineRestriction(m, p, Move::Down));
    }

    for (auto move : g_moves)
    {
      if (nBoxes >= 2 && isSquare2Box2Wall(m, p, move))
      {
        result.at(p).push_back(invalidSingle(p + move));
      }
    }

    if (nBoxes >= 3)
    {
      for (Move move : g_moves)
      {
        auto vc = square3Box1Wall(m, p, move);
        std::copy(vc.begin(), vc.end(), std::back_inserter(result.at(p)));
        vc = square3Box1WallDiag(m, p, move);
        std::copy(vc.begin(), vc.end(), std::back_inserter(result.at(p)));
      }
    }

    if (nBoxes >= 4)
    {
      for (Move move : g_moves)
      {
        auto vc = isSquare4box(m, p, move);
        std::copy(vc.begin(), vc.end(), std::back_inserter(result.at(p)));
      }
    }


    // TODO: add:
    // smart observer, that detects following cases:
    // wall wall  space
    // wall space box ... unit is outside this extremely limited space
    // wall space box
    // wall wall  space
  }


  return result;
}


} // namespace soko
