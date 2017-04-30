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
  void save();
  void setCursor(uint8_t row, uint8_t col, Direction dir);
  void puzzleClicked(uint8_t row, uint8_t col);
  void puzzleRightClicked();

private:
  std::unique_ptr<Puzzle> puzzle;
  Cursor cursor;

  void reloadPuzzle();

  QMenu *fileMenu;
  QAction *openAct;
  QAction *saveAct;

  QListWidget *acrossWidget;
  QListWidget *downWidget;

  QWidget *puzzleContainer;
  QVBoxLayout *puzzleContainerLayout;
  QLabel *curClueLabel;
  PuzzleWidget *puzzleWidget;

  void createActions();
  void createMenus();

  QListWidget *createClueWidget();

  void keyPressEvent(QKeyEvent *event) override;
  void keyUp();
  void keyDown();
  void keyLeft();
  void keyRight();

  void setLetter(char letter);
  void clearLetter();
};

} // namespace cygnus

#endif // MAINWINDOW_H
