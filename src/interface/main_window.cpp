#include "interface/main_window.h"
#include "interface/scene.h"
#include "interface/util.h"
#include "interface/scene_label.h"
#include "soko/util.h" // g_inf
#include <QDebug>
#include <QThread>
#include <QApplication>
#include <QDesktopWidget>
#include <QAction>
#include <QTime>
#include <QMenuBar>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <fstream>
#include <QTimer>

namespace
{

QRect victoryGeometry(const QRect &sz)
{
  int t = sz.top() * 1 / 4 + sz.bottom() * 3 / 4;
  int b = sz.top() * 1 / 10 + sz.bottom() * 9 / 10;
  int l = sz.left() * 9 / 10 + sz.right() * 1 / 10;
  int r = sz.left() * 1 / 10 + sz.right() * 9 / 10;
  return QRect(QPoint(l, t), QPoint(r, b));
}

QRect timeGeometry(const QRect &sz)
{
  int t = sz.top();
  int b = sz.top() * 3 / 4 + sz.bottom() * 1 / 4;
  int l = sz.left() * 1 / 4 + sz.right() * 3 / 4;
  int r = sz.left() * 1 / 20 + sz.right() * 19 / 20;
  return QRect(QPoint(l, t), QPoint(r, b));
}


const QString g_backgroundFile = ":grass.jpg";
const QString g_defaultLevelFile = "levels.txt";

} // namespace

MainWindow::MainWindow()
  : m_scene(new Scene(QPixmap(g_backgroundFile), this))
  , m_state(StateWindow::Playing)
{
  setupView();
  setupActions();
  setupMenu();
  setupInfo();
  setupSolver();

  std::ifstream f(g_defaultLevelFile.toStdString());
  if (f.is_open())
  {
    loadLevels(f);
  }

  m_scene->installEventFilter(this);
}

void MainWindow::showEvent(QShowEvent *)
{
  m_view->fitInView(m_scene->sceneRect(), Qt::IgnoreAspectRatio);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
  m_view->fitInView(m_scene->sceneRect(), Qt::IgnoreAspectRatio);
  QSize szTo = event->size();
  if (m_solvedInfo != nullptr)
  {
    m_solvedInfo->setGeometry(timeGeometry(QRect(QPoint(0, 0), szTo)));
  }
  if (m_winLabel != nullptr)
  {
    m_winLabel->setGeometry(victoryGeometry(QRect(QPoint(0, 0), szTo)));
  }
}

void MainWindow::setupView()
{
  m_view = new QGraphicsView(m_scene, this);
  m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  m_view->fitInView(m_scene->itemsBoundingRect(), Qt::IgnoreAspectRatio);
  setCentralWidget(m_view);
  connect(m_scene, &Scene::win, this, &MainWindow::onWin);
}

void MainWindow::setupMenu()
{
  auto fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(m_open);
  fileMenu->addAction(m_exit);
  auto editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(m_undo);
  editMenu->addAction(m_redo);
  editMenu->addAction(m_prev);
  editMenu->addAction(m_restart);
  editMenu->addAction(m_next);

  m_levelMenu = editMenu->addMenu(tr("&Levels"));
  auto infoMenu = editMenu->addMenu(tr("&Info"));
  infoMenu->addAction(m_toggleInfo);

  auto aiMenu = menuBar()->addMenu(tr("&AI"));
  aiMenu->addAction(m_solve);
  aiMenu->addAction(m_solverPlay);
  aiMenu->addAction(m_solverPrevStep);
  aiMenu->addAction(m_solverNextStep);
}

void MainWindow::setupActions()
{
  m_exit = new QAction(tr("&Exit"), this);
  m_exit->setShortcut(QKeySequence::Quit);
  connect(m_exit, &QAction::triggered, this, &QMainWindow::close);

  m_open = new QAction(tr("&Open"), this);
  m_open->setShortcut(QKeySequence::Open);
  connect(m_open, &QAction::triggered, this, &MainWindow::onOpen);

  m_undo = new QAction(tr("&Undo"), this);
  m_undo->setShortcut(QKeySequence::Undo);
  connect(m_undo, &QAction::triggered, m_scene, &Scene::undoMove);

  m_redo = new QAction(tr("Redo"), this);
  m_redo->setShortcut(QKeySequence::Redo);
  connect(m_redo, &QAction::triggered, m_scene, &Scene::redoMove);

  m_restart = new QAction(tr("&Restart"), this);
  m_restart->setShortcut(QKeySequence(tr("Ctrl+R", "Restart")));
  connect(m_restart, &QAction::triggered, this, &MainWindow::onRestartLevel);

  m_solve = new QAction(tr("&Solve"), this);
  m_solve->setShortcut(QKeySequence(tr("Ctrl+S", "Solve")));
  connect(m_solve, &QAction::triggered, this, &MainWindow::onSolve);

  m_next = new QAction(tr("&Next"), this);
  m_next->setShortcut(QKeySequence(tr("Ctrl+N", "Next level")));
  connect(m_next, &QAction::triggered, this, &MainWindow::onLoadNextLevel);

  m_prev = new QAction(tr("&Previous"), this);
  m_prev->setShortcut(QKeySequence(tr("Ctrl+P", "Previous level")));
  connect(m_prev, &QAction::triggered, this, &MainWindow::onLoadPrevLevel);

  m_toggleInfo = new QAction(tr("&Toggle info"), this);
  m_toggleInfo->setShortcut(QKeySequence(tr("Ctrl+T", "Toggle info")));
  connect(m_toggleInfo, &QAction::triggered, this, &MainWindow::onToggleInfo);

  m_solverPlay = new QAction(tr("Play/pause"), this);
  connect(m_solverPlay, &QAction::triggered, this, &MainWindow::onSolverPlayPause);

  m_solverNextStep = new QAction(tr("Next step"), this);
  connect(m_solverNextStep, &QAction::triggered, [this]() { this->solverMove(true); });

  m_solverPrevStep = new QAction(tr("Prev step"), this);
  connect(m_solverPrevStep, &QAction::triggered, [this]() { this->solverMove(false); });
}

void MainWindow::setupSolver()
{
  m_solver.setHeuristic(soko::Heuristic::create(soko::HeuristicType::HungarianTaxicab));
  connect(&this->m_solver, &SolverThread::finished, this, &MainWindow::onFinishedSolving);
}

void MainWindow::setupMap(bool resetAi)
{
  if (m_nCurrent >= m_maps.size())
  {
    return;
  }
  if (m_winLabel != nullptr)
  {
    // sometimes on next level "victory" still exists
    m_winLabel->hide();
    m_winLabel->deleteLater();
    m_winLabel = nullptr;
  }
  if (m_solvedInfo != nullptr)
  {
    m_solvedInfo->hide();
    m_solvedInfo->deleteLater();
    m_solvedInfo = nullptr;
  }

  m_state = StateWindow::Playing;
  if (resetAi)
  {
    m_solver.reset();
    m_solverNextStep->setEnabled(false);
    m_solverPrevStep->setEnabled(false);
    m_solverPlay->setEnabled(false);
  }
  m_scene->initMap(m_maps[m_nCurrent].second);
}

void MainWindow::onOpen()
{
  QFileDialog dialog;
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setFilter(QDir::Files);

  if (!dialog.exec())
  {
    return;
  }

  auto files = dialog.selectedFiles();
  Q_ASSERT(files.size() == 1);
  std::ifstream f(files.first().toStdString());
  if (!f.is_open())
  {
    QString error = tr("Can't open file %1");
    error = error.arg(files.first());
    QMessageBox box(this);
    box.setText(error);
    box.show();
    return;
  }

  loadLevels(f);
}

void MainWindow::onRestartLevel() { setupMap(false); }

void MainWindow::onSetLevel(size_t i)
{
  assert(i < m_maps.size());
  m_nCurrent = i;
  setupMap(true);
}

void MainWindow::setupInfo()
{
  m_info = new SceneLabel(this->centralWidget());
  m_info->setText(infoText());
  m_info->setFontSize(m_infoFontSize);
  m_info->setFontColor(m_infoFontColor);
  m_info->show();
  connect(m_scene, &Scene::infoChanged, this, &MainWindow::updateInfo);
}

QString MainWindow::infoText()
{
  QString resultStr;
  resultStr += QString("Steps: %1\n").arg(m_scene->stepCounter());
  resultStr += QString("Box Moves: %1\n").arg(m_scene->boxMovements());
  QString heuristic = m_scene->heuristic() == soko::g_inf ? QString::fromLocal8Bit("INF") :
                                                            QString::number(m_scene->heuristic());
  resultStr += QString("Heuristic: %1\n").arg(heuristic);

  return resultStr;
}

void MainWindow::updateInfo()
{
  m_info->setText(infoText());
  m_info->adjustSize();
}

void MainWindow::onLoadNextLevel()
{
  m_nCurrent = (m_nCurrent + 1) % m_maps.size();
  setupMap();
}

void MainWindow::onLoadPrevLevel()
{
  m_nCurrent = (m_nCurrent + m_maps.size() - 1) % m_maps.size();
  setupMap();
}

void MainWindow::onSolve()
{
  if (m_solver.solved() != soko::SolveState::Solved)
  {
    m_solver.runThread(m_maps[m_nCurrent].second);
    m_state = StateWindow::Solving;
  }
}

void MainWindow::onFinishedSolving()
{
  m_state = StateWindow::Solved;
  m_solvedInfo = new SceneLabel(centralWidget());
  m_solvedInfo->setText(QString("Time: %1").arg(m_solver.time()));
  if (m_solver.solved() != soko::SolveState::Solved)
  {
    m_solvedInfo->setText(m_solvedInfo->text() + "\n" + QString("Can't solve :("));
  }

  m_solvedInfo->setGeometry(timeGeometry(centralWidget()->geometry()));
  m_solvedInfo->setFontSize(std::min(m_solvedInfo->height() / 2, 20));
  m_solvedInfo->setFontColor(Qt::white);
  m_solvedInfo->setAlignment(Qt::AlignTop | Qt::AlignRight);
  m_solvedInfo->show();


  if (m_solver.solved() == soko::SolveState::Solved)
  {
    m_solverNextStep->setEnabled(true);
    m_solverPrevStep->setEnabled(true);
    m_solverPlay->setEnabled(true);
  }
}

void MainWindow::onWin()
{
  m_state = StateWindow::Win;
  m_winLabel = new SceneLabel(centralWidget());
  m_winLabel->setGeometry(victoryGeometry(centralWidget()->geometry()));
  m_winLabel->setText(tr("Victory"));
  m_winLabel->setAlignment(Qt::AlignCenter);
  m_winLabel->setFontSize(m_winLabel->geometry().height() / 2);
  m_winLabel->setFontColor(Qt::red);
  m_winLabel->show();
}

void MainWindow::onToggleInfo() { m_info->setHidden(!m_info->isHidden()); }

void MainWindow::solverStep()
{
  assert(m_state == StateWindow::SolverPaused || m_state == StateWindow::SolverPlaying);
  bool moved = solverMove(true);
  if (!moved)
  {
    m_state = StateWindow::Win;
    return;
  }
  if (m_state == StateWindow::SolverPlaying)
  {
    QTimer::singleShot(300, Qt::TimerType::PreciseTimer, this, &MainWindow::solverStep);
  }
}

void MainWindow::onSolverPlayPause()
{
  if (m_state == StateWindow::SolverPlaying)
  {
    m_state = StateWindow::SolverPaused;
    return;
  }
  if (m_state == StateWindow::SolverPaused)
  {
    m_state = StateWindow::SolverPlaying;
    solverStep();
    return;
  }

  if (m_state == StateWindow::Solved)
  {
    m_currentStep = m_solver.result().begin();
    m_scene->initMap(m_solver.map());
    m_state = StateWindow::SolverPlaying;
    solverStep();
  }
}

void MainWindow::delay(int ms)
{
  QTime delayTime = QTime::currentTime().addMSecs(ms);
  while (QTime::currentTime() < delayTime)
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }
}

bool MainWindow::solverMove(bool forward)
{
  if (m_state != StateWindow::SolverPaused && m_state != StateWindow::SolverPlaying)
  {
    return false;
  }
  if (forward)
  {
    if (m_currentStep == m_solver.result().end())
    {
      return false;
    }
    m_scene->move(*m_currentStep);
    ++m_currentStep;
    return true;
  }
  if (m_currentStep == m_solver.result().begin())
  {
    return false;
  }
  --m_currentStep;
  m_scene->undoMove();
  return true;
}


void MainWindow::loadLevels(std::istream &f)
{
  try
  {
    auto newMaps = parseFromFile(f);
    size_t oldSz = m_maps.size();
    std::copy(newMaps.begin(), newMaps.end(), std::back_inserter(m_maps));
    for (size_t i = oldSz; i < m_maps.size(); ++i)
    {
      QString name = QString::fromStdString(newMaps[i].first);
      QAction *action = new QAction(name, this);
      m_levelMenu->addAction(action);
      connect(action, &QAction::triggered, [this, i]() { onSetLevel(i); });
    }

    if (m_nCurrent >= m_maps.size() && !m_maps.empty())
    {
      m_nCurrent = 0;
      setupMap();
    }
  }
  catch (const std::exception &e)
  {
    QMessageBox box(this);
    box.setText(e.what());
    box.show();
    box.exec();
    return;
  }
}

bool MainWindow::eventFilter(QObject *, QEvent *event)
{
  if (event->type() != QEvent::KeyPress)
  {
    return false;
  }

  auto keyEvent = static_cast<QKeyEvent *>(event);
  int key = keyEvent->key();

  switch (m_state)
  {
  case StateWindow::Solved:
    if (key == Qt::Key_Space)
    {
      onSolverPlayPause();
    }
  case StateWindow::Playing:
  case StateWindow::Solving:
    handleKey(keyEvent);
    break;
  case StateWindow::SolverPlaying:
    if (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Space)
    {
      onSolverPlayPause();
    }
    break;
  case StateWindow::SolverPaused:
    if (key == Qt::Key_Space)
    {
      onSolverPlayPause();
    }
    if (key == Qt::Key_Left || key == Qt::Key_Right)
    {
      solverMove(key == Qt::Key_Right);
    }
    break;
  case StateWindow::Win:
    if (key == Qt::Key_R)
    {
      setupMap(false);
    }
    else if (key == Qt::Key_Space || key == Qt::Key_N)
    {
      onLoadNextLevel();
    }
    break;
  }

  return false;
}

void MainWindow::handleKey(QKeyEvent *event)
{
  switch (event->key())
  {
  case Qt::Key::Key_Left:
  {
    m_scene->move(soko::Move::Left);
    break;
  }
  case Qt::Key::Key_Right:
  {
    m_scene->move(soko::Move::Right);
    break;
  }
  case Qt::Key::Key_Down:
  {
    m_scene->move(soko::Move::Down);
    break;
  }
  case Qt::Key::Key_Up:
  {
    m_scene->move(soko::Move::Up);
    break;
  }
  case Qt::Key::Key_Backspace:
  {
    m_scene->undoMove();
    break;
  }
  default:
    break;
  }
}
