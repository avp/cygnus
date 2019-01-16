#ifndef PUZZLEWIDGET_H
#define PUZZLEWIDGET_H

#include "Puzzle.h"

#include <QtWidgets>

#include <cassert>
#include <memory>

namespace cygnus {

class PuzzleResizer;
class FilledLabel;

class CellWidget : public QWidget {
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
  void enterEvent(QEvent *event) override;
  void leaveEvent(QEvent *event) override;

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

  QString displayText_{""};

  FilledLabel *entryLabel_;
};

class PuzzleWidget : public QWidget {
  Q_OBJECT

  friend class PuzzleResizer;

public:
  explicit PuzzleWidget(const std::unique_ptr<Puzzle> &puzzle,
                        QWidget *parent = nullptr);

  CellWidget *getCell() { return cells_[0][0]; };

  PuzzleResizer *resizer_;

protected:
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

class PuzzleResizer : public QWidget {
  Q_OBJECT
public:
  explicit PuzzleResizer(PuzzleWidget *puzzle, QGridLayout *layout,
                         QWidget *parent = nullptr)
      : QWidget(parent), puzzle_(puzzle), grid_(new QWidget(this)) {
    assert(puzzle && layout && "must provide puzzle and layout");

    auto *hbox = new QHBoxLayout{};
    setLayout(hbox);

    grid_->setLayout(layout);
    hbox->addWidget(grid_, 1, Qt::AlignTop);
  }

  void resizeEvent(QResizeEvent *event) override {
    int h = event->size().height();
    int w = event->size().width();
    int rows = puzzle_->cells_.size();
    int cols = puzzle_->cells_[0].size();
    int cellSize = std::max(30, std::min(h / rows, w / cols));
    grid_->setFixedSize(rows * cellSize, cols * cellSize);
  }

  QSize minimumSizeHint() const override {
    int rows = puzzle_->cells_.size();
    int cols = puzzle_->cells_[0].size();
    return QSize(rows * 30, cols * 30);
  }

private:
  PuzzleWidget *puzzle_;
  QWidget *grid_;
};

} // namespace cygnus

#endif
