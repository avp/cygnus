#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Puzzle.h"
#include "PuzzleWidget.h"

#include <QtWidgets>

namespace cygnus {

enum class Direction {
  ACROSS,
  DOWN,
};

struct Cursor {
  uint8_t row;
  uint8_t col;
  Direction dir;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void open();
  void setCursor(uint8_t row, uint8_t col, Direction dir);

private:
  std::unique_ptr<Puzzle> puzzle;
  Cursor cursor;

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
