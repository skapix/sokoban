#include "interface/main_window.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QTextCodec>
#include <memory>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QApplication::setApplicationName("Sokoban-solver");
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

  auto window = std::make_shared<MainWindow>();
  QRect screenGeometry = QApplication::desktop()->screenGeometry();
  int screenWidth = screenGeometry.width();
  int screenHeight = screenGeometry.height();
  // resize window
  window->setMinimumSize(640, 480);
  window->resize(screenWidth * 3 / 5, screenHeight * 3 / 5);

  // center window
  int x = (screenWidth - window->width()) / 2;
  int y = (screenHeight - window->height()) / 2;
  window->move(x, y);


  window->show();

  return app.exec();
}
