#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  cygnus::MainWindow w{argc > 1 ? argv[1] : nullptr};
  w.showMaximized();
  QApplication::processEvents();
  return a.exec();
}
