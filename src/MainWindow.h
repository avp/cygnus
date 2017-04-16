#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Puzzle.h"
#include "PuzzleWidget.h"

#include <QtWidgets>

namespace cygnus {

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

  void reloadPuzzle();

  QMenu *fileMenu;
  QAction *openAct;

  QListWidget *acrossWidget;
  QListWidget *downWidget;

  QWidget *puzzleContainer;
  QVBoxLayout *puzzleContainerLayout;
  QLabel *curClueLabel;
  PuzzleWidget *puzzleWidget;

  void createActions();
  void createMenus();

  QListWidget *createClueWidget();

  void keyPressEvent(QKeyEvent *event);
  void keyUp();
  void keyDown();
  void keyLeft();
  void keyRight();
};

} // namespace cygnus

#endif // MAINWINDOW_H
