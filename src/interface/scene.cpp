#include "interface/scene.h"
#include "interface/elements.h"
#include "interface/util.h"
#include "soko/util.h" // g_inf
#include <QPainter>
#include <QDebug>

namespace
{

// TODO: ? rotating

const size_t g_saverCapacity = 4096;
const qreal g_destinationZValue = -1; // should be < 0
const qreal g_backgroundZValue = -2; // should be < g_destinationZValue
const QByteArray g_propAnimation = "pos";

const int g_animationLength = 200;

} // namespace

Scene::Scene(const QPixmap &background, QObject *parent)
  : QGraphicsScene(parent)
  , m_background(addPixmap(background))
  , m_saver(g_saverCapacity)
  , m_heuristicFn(soko::Heuristic::create(soko::HeuristicType::HungarianTaxicab))
{
  m_background->setZValue(g_backgroundZValue);
}

void Scene::deinitMap()
{
  if (m_map == nullptr)
  {
    return;
  }

  m_unit = nullptr; // will be deleted by clearing scene
  m_map.reset();
  m_groupAnim.reset();
  auto created = items();
  for (auto item : created)
  {
    if (item != m_background)
    {
      removeItem(item);
      delete item;
    }
  }
  m_dx = m_dy = 0;
}

void Scene::initMap(const soko::Map &map)
{
  if (m_map != nullptr)
  {
    deinitMap();
    m_saver.clear();
  }

  m_map = std::make_unique<soko::GameState>(map);
  m_groupAnim = std::make_unique<QParallelAnimationGroup>();

  m_dx = sceneRect().width() / (map.cols() + 2);
  m_dy = sceneRect().height() / (map.rows() + 2);

  auto addCreatedItem = [this](ResizedPixmap *item, int i, int j) {
    item->setPos(j * m_dx, i * m_dy);
    item->setSize(m_dx, m_dy);
    addItem(item);
    return item;
  };

  auto addUnit = [this, addCreatedItem](int i, int j) {
    Q_ASSERT(m_unit == nullptr && "Unit already exists");
    m_unit = new Unit();
    addCreatedItem(m_unit, i, j);
  };

  for (size_t i = 0; i <= map.rows() + 1; ++i)
  {
    for (size_t j = 0; j <= map.cols() + 1; ++j)
    {
      if (i == 0 || j == 0 || i == map.rows() + 1 || j == map.cols() + 1)
      {
        addCreatedItem(new Wall(), i, j);
        continue;
      }

      soko::Cell cell = map.at(i - 1, j - 1);

      switch (cell)
      {
      case soko::Cell::Wall:
      {
        addCreatedItem(new Wall(), i, j);
        break;
      }
      case soko::Cell::BoxDestination:
      {
        auto dest = addCreatedItem(new Destination(), i, j);
        dest->setZValue(g_destinationZValue);
        addCreatedItem(new Box(), i, j);
        break;
      }
      case soko::Cell::Box:
      {
        addCreatedItem(new Box(), i, j);
        break;
      }
      case soko::Cell::UnitDestination:
      {
        auto dest = addCreatedItem(new Destination(), i, j);
        dest->setZValue(g_destinationZValue);
        addUnit(i, j);
        break;
      }
      case soko::Cell::Destination:
      {
        auto dest = addCreatedItem(new Destination(), i, j);
        dest->setZValue(g_destinationZValue);
        break;
      }
      case soko::Cell::Unit:
      {
        addUnit(i, j);
        break;
      }
      default:
        break;
      }
    }
  }

  initInfo();
}

void Scene::initInfo()
{
  std::vector<soko::Pos> boxes;
  auto staticMap = mapToMapStatic(m_map->map(), &boxes);

  m_heuristicFn->init(m_map->map());
  m_solvabilityMap =
      std::make_unique<soko::SolvabilityMap>(soko::createSolvabilityMap(staticMap, boxes.size()));

  m_stepCounter = 0;
  m_boxMovements = 0;
  calculateInfoHeuristics();
  emit infoChanged();
}

void Scene::move(soko::Move m)
{
  int dx = soko::toHorizontal(m) * m_dx;
  int dy = soko::toVertical(m) * m_dy;
  auto r = moveTo(dx, dy, m);

  changeInfo(r);

  if (m_map && m_map->isWinningState())
  {
    emit win();
  }
}

void Scene::undoMove()
{
  if (m_map == nullptr || m_saver.empty() || m_current == m_saver.begin())
  {
    return;
  }
  m_current = std::prev(m_current);

  soko::MoveResult result = m_map->undo(*m_current);

  if (m_groupAnim->state() == QParallelAnimationGroup::Running)
  {
    m_groupAnim->setCurrentTime(g_animationLength);
  }

  int dx = -soko::toHorizontal(m_current->move) * m_dx;
  int dy = -soko::toVertical(m_current->move) * m_dy;

  auto unitPos = m_unit->pos();
  m_unit->moveBy(dx, dy);

  if (result == soko::MoveResult::UnitBoxMove)
  {
    auto box = boxAt({unitPos.x() - dx, unitPos.y() - dy});
    assert(box != nullptr);
    box->moveBy(dx, dy);
  }

  changeInfo(result, false);
}

void Scene::redoMove()
{
  if (m_map == nullptr || m_saver.empty() || m_current == m_saver.end())
  {
    return;
  }

  soko::MoveDirection result = m_map->move(m_current->move);
  assert(result);
  int dx = soko::toHorizontal(m_current->move) * m_dx;
  int dy = soko::toVertical(m_current->move) * m_dy;
  m_unit->moveBy(dx, dy);

  m_current = std::next(m_current);
  if (result.result == soko::MoveResult::UnitBoxMove)
  {
    auto box = boxAt(m_unit->pos());
    assert(box != nullptr);
    box->moveBy(dx, dy);
  }

  changeInfo(result.result, true);
}

soko::MoveResult Scene::moveTo(int dx, int dy, soko::Move move)
{
  if (m_map == nullptr)
  {
    return soko::MoveResult::NoMove;
  }

  auto moveResult = m_map->move(move);
  if (!moveResult)
  {
    return soko::MoveResult::NoMove;
  }

  m_saver.erase(m_current, m_saver.end());
  m_saver.push_back(moveResult);
  m_current = m_saver.end();


  if (m_groupAnim->state() == QParallelAnimationGroup::Running)
  {
    m_groupAnim->setCurrentTime(g_animationLength);
  }
  m_groupAnim->clear();


  QPointF startUnitValue = m_unit->pos();
  QPointF endUnitValue = QPointF(startUnitValue.x() + dx, startUnitValue.y() + dy);

  auto unitAnim = new QPropertyAnimation(m_unit, g_propAnimation);
  unitAnim->setStartValue(startUnitValue);
  unitAnim->setEndValue(endUnitValue);
  unitAnim->setDuration(g_animationLength);
  m_groupAnim->addAnimation(unitAnim);

  if (moveResult.result == soko::MoveResult::UnitBoxMove)
  {
    QPointF startBoxValue = endUnitValue;
    Box *box = boxAt(startBoxValue);
    QPointF endUnitValue = QPointF(startBoxValue.x() + dx, startBoxValue.y() + dy);

    auto boxAnim = new QPropertyAnimation(box, g_propAnimation);
    boxAnim->setStartValue(startBoxValue);
    boxAnim->setEndValue(endUnitValue);
    boxAnim->setDuration(g_animationLength);
    m_groupAnim->addAnimation(boxAnim);
  }
  m_groupAnim->start();

  return moveResult.result;
}

void Scene::changeInfo(soko::MoveResult m, bool forward)
{
  int cntChange = forward ? 1 : -1;
  if (m == soko::MoveResult::UnitMove)
  {
    m_stepCounter += cntChange;
    emit infoChanged();
  }

  if (m == soko::MoveResult::UnitBoxMove)
  {
    calculateInfoHeuristics();
    m_stepCounter += cntChange;
    m_boxMovements += cntChange;
    emit infoChanged();
  }
}

void Scene::calculateInfoHeuristics()
{
  soko::MapState state;
  mapToMapStatic(m_map->map(), &state.boxes, &state.unit);
  bool isSolvable =
      std::all_of(state.boxes.begin(), state.boxes.end(),
                  [this, &state](soko::Pos p) { return m_solvabilityMap->isValid(p, state); });
  m_heuristic = isSolvable ? calculateHeuristic(m_map->map(), *m_heuristicFn) : soko::g_inf;
}

Box *Scene::boxAt(QPointF pt)
{
  auto in_rect = items(QRectF(pt.x(), pt.y(), m_dx, m_dy));

  for (auto item : in_rect)
  {
    if (Box *box = qgraphicsitem_cast<Box *>(item))
    {
      return box;
    }
  }
  return nullptr;
}
