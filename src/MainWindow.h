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
  void saveAs();

  void reveal(uint8_t row, uint8_t col);
  void revealCurrent();
  void revealClue();
  void revealAll();

  void setCursor(uint8_t row, uint8_t col, Direction dir);
  void puzzleClicked(uint8_t row, uint8_t col);
  void puzzleRightClicked();

  void acrossClueClicked(const QListWidgetItem *item);
  void downClueClicked(const QListWidgetItem *item);

private:
  std::unique_ptr<Puzzle> puzzle_;
  Cursor cursor_;

  void reloadPuzzle();

  QMenu *fileMenu_;
  QAction *openAct_;
  QAction *saveAct_;
  QAction *saveAsAct_;

  QMenu *puzzleMenu_;
  QMenu *revealMenu_;
  QAction *revealCurrentAct_;
  QAction *revealClueAct_;
  QAction *revealAllAct_;

  QLabel *titleLabel_;
  QLabel *authorLabel_;
  QLabel *copyrightLabel_;

  ClueWidget *acrossWidget_;
  ClueWidget *downWidget_;

  QWidget *puzzleContainer_;
  QVBoxLayout *puzzleContainerLayout_;
  QLabel *curClueLabel_;
  PuzzleWidget *puzzleWidget_{nullptr};

  void createActions();
  void createMenus();

  ClueWidget *createClueWidget();

  void keyPressEvent(QKeyEvent *event) override;
  void keyUp();
  void keyDown();
  void keyLeft();
  void keyRight();

  void setLetter(uint8_t row, uint8_t col, char letter);
  void clearLetter(uint8_t row, uint8_t col);
};

} // namespace cygnus

#endif // MAINWINDOW_H
