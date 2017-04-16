#ifndef PUZZLEWIDGET_H
#define PUZZLEWIDGET_H

#include "Puzzle.h"

#include <QtWidgets>

namespace cygnus {

class CellWidget : public QFrame {
  Q_OBJECT
public:
  explicit CellWidget(bool isBlack, uint8_t row, uint8_t col, uint32_t num,
                      QWidget *parent = nullptr);

  inline uint8_t getRow() const { return row_; }
  inline uint8_t getCol() const { return col_; }

public slots:
  void selectCursor();
  void select();

  void deselect();

signals:
  void clicked(uint8_t row, uint8_t col);
  void rightClicked();

private:
  bool isBlack_;

  uint8_t row_;
  uint8_t col_;

  void mousePressEvent(QMouseEvent *event) override;
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

  void cellClicked(uint8_t row, uint8_t col) { return clicked(row, col); }
  void cellRightClicked() { return rightClicked(); }

signals:
  void clicked(uint8_t row, uint8_t col);
  void rightClicked();

private:
  QGridLayout *gridLayout_;
  Grid<CellWidget *> cells_;
};

} // namespace cygnus

#endif
