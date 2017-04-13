#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Puzzle.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void open();

private:
  Ui::MainWindow *ui;

  static std::unique_ptr<Puzzle> puzzle;

  void createActions();
  void createMenus();

  QMenu *fileMenu;
  QAction *openAct;
};

#endif // MAINWINDOW_H
