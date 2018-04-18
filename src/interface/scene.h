#pragma once

#include <QGraphicsScene>
#include <QGraphicsObject>
#include <memory>
#include "soko/game_state.h"
#include "boost/circular_buffer.hpp"
#include "soko/heuristic.h"
#include "soko/solvability.h"
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class Box;
class Unit;

class Scene : public QGraphicsScene {
  Q_OBJECT

public:
  Scene(const QPixmap &background, QObject *parent = nullptr);

  void deinitMap();
  void initMap(const soko::Map &map);
  void initInfo();

  void move(soko::Move m);
  void undoMove();
  void redoMove();

  size_t stepCounter() const { return m_stepCounter; }
  size_t boxMovements() const { return m_boxMovements; }
  size_t heuristic() const { return m_heuristic; }

  const soko::Map *map() const noexcept { return m_map == nullptr ? nullptr : &m_map->map(); }


signals:
  void win();
  void infoChanged();

private:
  Box *boxAt(QPointF);
  soko::MoveResult moveTo(int dx, int dy, soko::Move move);
  void changeInfo(soko::MoveResult m, bool forward = true);
  void calculateInfoHeuristics();

private:
  // items
  QGraphicsItem *m_background;
  Unit *m_unit = nullptr;

  // map
  int m_dx = 0;
  int m_dy = 0;
  std::unique_ptr<soko::GameState> m_map;

  // state saving
  // TODO: remove boost dependency
  using StateSaver = boost::circular_buffer<soko::MoveDirection>;
  StateSaver m_saver;
  StateSaver::iterator m_current;

  // animation
  std::unique_ptr<QParallelAnimationGroup> m_groupAnim;

  // info
  std::unique_ptr<soko::Heuristic> m_heuristicFn;
  std::unique_ptr<soko::SolvabilityMap> m_solvabilityMap;
  size_t m_stepCounter = 0;
  size_t m_boxMovements = 0;
  size_t m_heuristic = 0;
};
