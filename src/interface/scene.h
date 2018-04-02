#pragma once

#include <QGraphicsScene>
#include <QGraphicsObject>
#include <memory>
#include "soko/game_state.h"
#include "boost/circular_buffer.hpp"
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

  void moveLeft();
  void moveRight();
  void moveUp();
  void moveDown();
  void undoMove();
  void redoMove();

signals:
  void win();

private:
  Box *boxAt(QPointF);
  void moveTo(int dx, int dy, soko::Move move);

private:
  // items
  QGraphicsItem *m_background;
  Unit *m_unit = nullptr;

  // map
  int m_dx = 0;
  int m_dy = 0;
  std::unique_ptr<soko::GameState> m_map;

  // state saving
  using StateSaver = boost::circular_buffer<soko::MoveDirection>;
  StateSaver m_saver;
  StateSaver::iterator m_current;

  // animation
  std::unique_ptr<QParallelAnimationGroup> m_groupAnim;
};
