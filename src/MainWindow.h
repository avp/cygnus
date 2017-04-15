#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Puzzle.h"
#include "PuzzleWidget.h"

#include <QtWidgets>

namespace cygnus {

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void open();

private:
  static std::unique_ptr<Puzzle> puzzle;

  QMenu *fileMenu;
  QAction *openAct;

  QListWidget *acrossWidget;
  QListWidget *downWidget;

  QWidget *puzzleContainer;
  QHBoxLayout *puzzleContainerLayout;
  PuzzleWidget *puzzleWidget;

  void createActions();
  void createMenus();

  QListWidget *createClueWidget();

  void reloadPuzzle();
};

} // namespace cygnus

#endif // MAINWINDOW_H
