#include "interface/main_window.h"
#include "interface/scene.h"
#include "interface/util.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QAction>
#include <QMenuBar>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <fstream>

namespace
{

const QString g_backgroundFile = ":grass.jpg";
const QString g_defaultLevelFile = "levels.txt";

} // namespace

MainWindow::MainWindow()
  : m_scene(new Scene(QPixmap(g_backgroundFile), this))
{
  setView();
  createActions();
  createMenu();

  std::ifstream f(g_defaultLevelFile.toStdString());
  if (f.is_open())
  {
    loadLevels(f);
  }

  m_scene->installEventFilter(this);
}

void MainWindow::setView()
{
  m_view = new QGraphicsView(m_scene, this);
  m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  m_view->fitInView(m_scene->itemsBoundingRect(), Qt::IgnoreAspectRatio);
  setCentralWidget(m_view);
  connect(m_scene, &Scene::win, this, &MainWindow::onWin);
}

void MainWindow::createMenu()
{
  QPalette palette;
  palette.setColor(menuBar()->backgroundRole(), Qt::gray);
  palette.setColor(menuBar()->foregroundRole(), Qt::white);
  menuWidget()->setPalette(palette);

  auto fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(m_open);
  fileMenu->addAction(m_exit);
  auto editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(m_undo);
  editMenu->addAction(m_redo);
  editMenu->addAction(m_restart);
  m_levelMenu = editMenu->addMenu(tr("&Levels"));
}

void MainWindow::createActions()
{
  m_exit = new QAction(tr("&Exit"), this);
  m_exit->setShortcut(QKeySequence::Quit);
  connect(m_exit, &QAction::triggered, this, &QMainWindow::close);

  m_open = new QAction(tr("&Open"), this);
  m_open->setShortcut(QKeySequence::Open);
  connect(m_open, &QAction::triggered, this, &MainWindow::open);

  m_undo = new QAction(tr("&Undo"), this);
  m_undo->setShortcut(QKeySequence::Undo);
  connect(m_undo, &QAction::triggered, m_scene, &Scene::undoMove);

  m_redo = new QAction(tr("&Redo"), this);
  m_redo->setShortcut(QKeySequence::Redo);
  connect(m_redo, &QAction::triggered, m_scene, &Scene::redoMove);

  m_restart = new QAction(tr("&Restart"), this);
  m_restart->setShortcut(QKeySequence(tr("Ctrl+R", "Restart")));
  connect(m_restart, &QAction::triggered, this, &MainWindow::restartLevel);
}

void MainWindow::open()
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

void MainWindow::restartLevel() { setupMap(); }

void MainWindow::setLevel(size_t i)
{
  assert(i < m_maps.size());
  m_nCurrent = i;
  setupMap();
}

void MainWindow::setupMap()
{
  if (m_nCurrent >= m_maps.size())
  {
    return;
  }
  m_scene->initMap(m_maps[m_nCurrent].second);
}

void MainWindow::onWin()
{
  m_nCurrent = (m_nCurrent + 1) % m_maps.size();
  setupMap();
}

void MainWindow::showEvent(QShowEvent *)
{
  m_view->fitInView(m_scene->sceneRect(), Qt::IgnoreAspectRatio);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
  m_view->fitInView(m_scene->sceneRect(), Qt::IgnoreAspectRatio);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  handleKey(event);

  QMainWindow::keyPressEvent(event);
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
  if (event->type() == QEvent::KeyPress)
  {
    auto keyEvent = static_cast<QKeyEvent *>(event);
    int key = keyEvent->key();
    if (key == Qt::Key::Key_Down || key == Qt::Key::Key_Up || key == Qt::Key::Key_Left ||
        key == Qt::Key::Key_Right)
    {
      handleKey(keyEvent);
    }
  }

  return false;
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
      connect(action, &QAction::triggered, [this, i]() { setLevel(i); });
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

void MainWindow::handleKey(QKeyEvent *event)
{
  switch (event->key())
  {
  case Qt::Key::Key_Left:
  {
    m_scene->moveLeft();
    break;
  }
  case Qt::Key::Key_Right:
  {
    m_scene->moveRight();
    break;
  }
  case Qt::Key::Key_Down:
  {
    m_scene->moveDown();
    break;
  }
  case Qt::Key::Key_Up:
  {
    m_scene->moveUp();
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
