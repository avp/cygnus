#ifndef PUZZLEWIDGET_H
#define PUZZLEWIDGET_H

#include "Puzzle.h"

#include <QtWidgets>
#include <memory>

namespace cygnus {

class FilledLabel;

class CellWidget : public QFrame {
  Q_OBJECT
public:
  explicit CellWidget(bool isBlack, uint8_t row, uint8_t col,
                      const Puzzle::CellData &cellData,
                      const Puzzle::Markup markup, QWidget *parent = nullptr);

  inline uint8_t getRow() const { return row_; }
  inline uint8_t getCol() const { return col_; }

protected:
  void paintEvent(QPaintEvent *pe) override;
  void mousePressEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

public slots:
  void selectCursor();
  void select();

  void deselect();

  void setCell(const QString &text);
  void setMarkup(Puzzle::Markup markup);

signals:
  void clicked(uint8_t row, uint8_t col);
  void rightClicked();

private:
  uint8_t row_;
  uint8_t col_;
  bool isBlack_;
  Puzzle::Markup markup_;
  bool isPencil_;

  FilledLabel *entryLabel_;
};

class PuzzleWidget : public QFrame {
  Q_OBJECT
public:
  explicit PuzzleWidget(const std::unique_ptr<Puzzle> &puzzle,
                        QWidget *parent = nullptr);

protected:
  void paintEvent(QPaintEvent *pe) override;

public slots:
  void selectCursorPosition(uint8_t row, uint8_t col);
  void selectPosition(uint8_t row, uint8_t col);

  void deselectPosition(uint8_t row, uint8_t col);

  void cellClicked(uint8_t row, uint8_t col) { return clicked(row, col); }
  void cellRightClicked() { return rightClicked(); }

  void setCell(uint8_t row, uint8_t col, const QString &text);
  void setMarkup(uint8_t row, uint8_t col, Puzzle::Markup markup);

signals:
  void clicked(uint8_t row, uint8_t col);
  void rightClicked();

private:
  QGridLayout *gridLayout_;
  Grid<CellWidget *> cells_;
};

} // namespace cygnus

#endif
