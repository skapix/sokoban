#include "interface/elements.h"

#include <QPropertyAnimation>

ResizedPixmap::ResizedPixmap(const char *pixmapName, QGraphicsItem *parent)
  : QGraphicsPixmapItem(QPixmap(pixmapName), parent)
  , m_pixmapName(pixmapName)
{}

void ResizedPixmap::setSize(int x, int y)
{
  if (m_pixmapName == nullptr)
  {
    return;
  }

  auto map = QPixmap(m_pixmapName);
  if (map.isNull())
  {
    return;
  }
  setPixmap(map.scaled(x, y));
  return;
}

Box::Box(QGraphicsItem *parent)
  : ResizedPixmap(":box.png", parent)
{
  QPixmap pm = QPixmap(":box.png");
  setPixmap(pm);
}

Wall::Wall(QGraphicsItem *parent)
  : ResizedPixmap(":wall.png", parent)
{}

Unit::Unit(QGraphicsItem *parent)
  : ResizedPixmap(":loader.png", parent)
{}

Destination::Destination(QGraphicsItem *parent)
  : ResizedPixmap(":cross.png", parent)
{}
