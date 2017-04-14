#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Puzzle.h"

#include <QtWidgets>

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

  QMenu *fileMenu;
  QAction *openAct;

  QListWidget *acrossWidget;
  QListWidget *downWidget;

  QWidget *puzzleWidget;

  void createActions();
  void createMenus();

  QListWidget *createClueWidget();

  void reloadPuzzle();
};

#endif // MAINWINDOW_H
