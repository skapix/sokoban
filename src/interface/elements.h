#pragma once

#include <QGraphicsPixmapItem>

class ResizedPixmap
  : public QObject
  , public QGraphicsPixmapItem {

  Q_OBJECT
  Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
  ResizedPixmap(const char *pixmapName, QGraphicsItem *parent = nullptr);
  void setSize(int x, int y);

private:
  const char *m_pixmapName;
};

enum Types
{
  TypeUnit = QGraphicsItem::UserType + 1,
  TypeWall,
  TypeBox,
  TypeDestination,
};

class Box : public ResizedPixmap {
public:
  enum
  {
    Type = TypeBox
  };
  Box(QGraphicsItem *parent = nullptr);
  virtual int type() const override { return Type; }
};

class Wall : public ResizedPixmap {
public:
  enum
  {
    Type = TypeWall
  };
  Wall(QGraphicsItem *parent = nullptr);
  virtual int type() const override { return Type; }
};

class Unit : public ResizedPixmap {
public:
  enum
  {
    Type = TypeUnit
  };
  Unit(QGraphicsItem *parent = nullptr);
  virtual int type() const override { return Type; }
};


class Destination : public ResizedPixmap {
public:
  enum
  {
    Type = TypeDestination
  };
  Destination(QGraphicsItem *parent = nullptr);
  virtual int type() const override { return Type; }
};
