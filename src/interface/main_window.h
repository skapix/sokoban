#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "soko/map.h"

class Scene;

class MainWindow : public QMainWindow {
public:
  MainWindow();

private slots:
  void open();
  void restartLevel();
  void setLevel(size_t i);

  void onWin();

protected:
  virtual void showEvent(QShowEvent *ev) override;
  virtual void resizeEvent(QResizeEvent *event) override;
  virtual void keyPressEvent(QKeyEvent *event) override;
  virtual bool eventFilter(QObject *object, QEvent *event);

private:
  void setView();
  void createMenu();
  void createActions();
  void setupMap();

  void loadLevels(std::istream &f);
  void handleKey(QKeyEvent *event);

private:
  std::vector<std::pair<std::string, soko::Map>> m_maps;
  size_t m_nCurrent = static_cast<size_t>(-1);

  Scene *m_scene = nullptr;
  QGraphicsView *m_view = nullptr;
  QGraphicsPixmapItem *m_background = nullptr;

  QAction *m_exit = nullptr;
  QAction *m_open = nullptr;
  QAction *m_undo = nullptr;
  QAction *m_redo = nullptr;
  QAction *m_restart = nullptr;

  QMenu *m_levelMenu = nullptr;
};
