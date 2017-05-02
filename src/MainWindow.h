#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ClueWidget.h"
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

  void acrossClueClicked(const QListWidgetItem *item);
  void downClueClicked(const QListWidgetItem *item);

private:
  std::unique_ptr<Puzzle> puzzle;
  Cursor cursor;

  void reloadPuzzle();

  QMenu *fileMenu;
  QAction *openAct;
  QAction *saveAct;

  ClueWidget *acrossWidget;
  ClueWidget *downWidget;

  QWidget *puzzleContainer;
  QVBoxLayout *puzzleContainerLayout;
  QLabel *curClueLabel;
  PuzzleWidget *puzzleWidget;

  void createActions();
  void createMenus();

  ClueWidget *createClueWidget();

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
