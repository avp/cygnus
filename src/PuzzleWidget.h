#ifndef PUZZLEWIDGET_H
#define PUZZLEWIDGET_H

#include "Puzzle.h"

#include <QtWidgets>

namespace cygnus {

class CellWidget : public QFrame {
  Q_OBJECT
public:
  explicit CellWidget(bool isBlack, uint32_t num, QWidget *parent = nullptr);

public slots:
  void selectCursor();
  void select();

  void deselect();

private:
  bool isBlack_;
};

class PuzzleWidget : public QFrame {
  Q_OBJECT
public:
  explicit PuzzleWidget(const std::unique_ptr<Puzzle> &puzzle,
                        QWidget *parent = nullptr);

public slots:
  void selectCursorPosition(uint8_t row, uint8_t col);
  void selectPosition(uint8_t row, uint8_t col);

  void deselectPosition(uint8_t row, uint8_t col);

private:
  QGridLayout *gridLayout_;
  Grid<CellWidget *> cells_;
};

} // namespace cygnus

#endif
