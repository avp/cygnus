#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  cygnus::MainWindow w;
  w.showMaximized();
  return a.exec();
}
