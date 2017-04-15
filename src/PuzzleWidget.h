#ifndef PUZZLEWIDGET_H
#define PUZZLEWIDGET_H

#include "Puzzle.h"

#include <QtWidgets>

class CellWidget : public QFrame {
  Q_OBJECT
public:
  explicit CellWidget(bool isBlack, uint32_t num, QWidget *parent = nullptr);
};

class PuzzleWidget : public QWidget {
  Q_OBJECT
public:
  explicit PuzzleWidget(const std::unique_ptr<Puzzle> &puzzle,
                        QWidget *parent = nullptr);

private:
  QGridLayout *gridLayout_;
  Grid<CellWidget *> cells_;
};

#endif
