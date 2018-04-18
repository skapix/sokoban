#pragma once

#include <future>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "interface/solver_wrapper.h"
#include "soko/map.h"
#include "soko/heuristic.h"
#include "soko/solvability.h"


// TODO: add info

// AI info updates once a second:
// * Progress:
//   Watched states (1k)
//   Current heuristics
//
// * total steps
// * total box moves

// TODO: ? add more noticeable box on destination visualization
class SceneLabel;
class Scene;

class MainWindow : public QMainWindow {
public:
  MainWindow();

protected:
  virtual void showEvent(QShowEvent *ev) override;
  virtual void resizeEvent(QResizeEvent *event) override;
  virtual bool eventFilter(QObject *object, QEvent *event);

private slots:
  void onOpen();
  void onRestartLevel();
  void onSetLevel(size_t i);

  void onLoadNextLevel();
  void onLoadPrevLevel();
  void onSolve();
  void onFinishedSolving();
  void onWin();
  void onToggleInfo();
  void solverStep();
  void onSolverPlayPause();

private:
  void setupView();
  void setupMap(bool resetAi = true);
  void setupInfo();
  void setupMenu();
  void setupActions();
  void setupSolver();

  void delay(int ms);
  QString infoText();
  void updateInfo();

  void loadLevels(std::istream &f);
  void handleKey(QKeyEvent *event);
  bool solverMove(bool forward);

private:
  std::vector<std::pair<std::string, soko::Map>> m_maps;
  size_t m_nCurrent = static_cast<size_t>(-1);

  Scene *m_scene = nullptr;
  QGraphicsView *m_view = nullptr;
  QGraphicsPixmapItem *m_background = nullptr;

  QMenu *m_levelMenu = nullptr;
  QAction *m_exit = nullptr;
  QAction *m_open = nullptr;
  QAction *m_undo = nullptr;
  QAction *m_redo = nullptr;
  QAction *m_restart = nullptr;
  QAction *m_solve = nullptr;
  QAction *m_next = nullptr;
  QAction *m_prev = nullptr;
  QAction *m_toggleInfo = nullptr;
  QAction *m_solverPlay = nullptr;
  QAction *m_solverNextStep = nullptr;
  QAction *m_solverPrevStep = nullptr;

  SolverThread m_solver;
  std::vector<soko::Move>::const_iterator m_currentStep;

  SceneLabel *m_info = nullptr;
  SceneLabel *m_winLabel = nullptr;
  SceneLabel *m_solvedInfo = nullptr;
  int m_infoFontSize = 20;
  QColor m_infoFontColor = Qt::white;

  enum class StateWindow
  {
    Playing, // usual mode
    Solving, // show/hide ai stats
    Solved, // ability to launch ai
    SolverPlaying, // can be paused and quit to playing mode
    SolverPaused, // move forward and backward
    Win,
  } m_state;
};
