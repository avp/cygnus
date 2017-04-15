#ifndef PUZZLEWIDGET_H
#define PUZZLEWIDGET_H

#include "Puzzle.h"

#include <QtWidgets>

class CellWidget : public QFrame {
  Q_OBJECT
public:
  explicit CellWidget(QWidget *parent, bool isBlack, uint32_t num);
};

class PuzzleWidget : public QWidget {
  Q_OBJECT
public:
  explicit PuzzleWidget(QWidget *parent, const std::unique_ptr<Puzzle> &puzzle);

private:
  QGridLayout *gridLayout_;
  Grid<CellWidget *> cells_;
};

#endif
