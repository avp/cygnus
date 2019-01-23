#ifndef PUZZLEWIDGET_H
#define PUZZLEWIDGET_H

#include "Puzzle.h"

#include <QtWidgets>

#include <cassert>
#include <memory>

namespace cygnus {

class PuzzleResizer;
class FilledLabel;

/// Represent a cell in the crossword widget.
class CellWidget : public QWidget {
  Q_OBJECT
public:
  static constexpr int kMinimumSize = 33;

  explicit CellWidget(bool isBlack, uint8_t row, uint8_t col,
                      const Puzzle::CellData &cellData,
                      const Puzzle::Markup markup, QWidget *parent = nullptr);

  int heightForWidth(int w) const override { return w; }

  inline uint8_t getRow() const { return row_; }
  inline uint8_t getCol() const { return col_; }

protected:
  void paintEvent(QPaintEvent *pe) override;

  /// Click on the puzzle.
  void mousePressEvent(QMouseEvent *event) override;

  /// Mouseover shows the rebus in a tooltip.
  void enterEvent(QEvent *event) override;

  /// Mouse leave hides the rebus in a tooltip.
  void leaveEvent(QEvent *event) override;

  void resizeEvent(QResizeEvent *event) override;
  void resizeEvent(int height, int width);

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
  QLabel *numLabel_;
};

/// Visual representation of the crossword puzzle.
class PuzzleWidget : public QWidget {
  Q_OBJECT

  friend class PuzzleResizer;

public:
  explicit PuzzleWidget(const std::unique_ptr<Puzzle> &puzzle,
                        QWidget *parent = nullptr);

public slots:
  void selectCursorPosition(uint8_t row, uint8_t col);
  void selectPosition(uint8_t row, uint8_t col);

  void deselectPosition(uint8_t row, uint8_t col);

  void cellClicked(uint8_t row, uint8_t col) {
    setFocus();
    return clicked(row, col);
  }
  void cellRightClicked() {
    setFocus();
    return rightClicked();
  }

  void setCell(uint8_t row, uint8_t col, const QString &text);
  void setMarkup(uint8_t row, uint8_t col, Puzzle::Markup markup);

signals:
  void clicked(uint8_t row, uint8_t col);
  void rightClicked();

private:
  QGridLayout *gridLayout_;
  Grid<CellWidget *> cells_;

  PuzzleResizer *resizer_;
};

/// Resizes the grid owned by a given PuzzleWidget.
/// This is necessary because we cannot increase the size of the grid by a
/// single pixel without increasing the size of every cell widget in both
/// directions. The PuzzleResizer provides the centering and resizing mechanism.
class PuzzleResizer : public QWidget {
  Q_OBJECT
public:
  explicit PuzzleResizer(PuzzleWidget *puzzle, QGridLayout *layout,
                         QWidget *parent = nullptr)
      : QWidget(parent), puzzle_(puzzle), grid_(new QWidget(this)) {
    assert(puzzle && layout && "must provide puzzle and layout");

    auto *hbox = new QHBoxLayout{};
    hbox->setContentsMargins(0, 0, 0, 0);
    setLayout(hbox);

    grid_->setLayout(layout);
    hbox->addWidget(grid_, 0, Qt::AlignTop);
  }

  void resizeEvent(QResizeEvent *event) override {
    int h = event->size().height();
    int w = event->size().width();
    int rows = puzzle_->cells_.size();
    int cols = puzzle_->cells_[0].size();
    int minSize = CellWidget::kMinimumSize;
    int cellSize = std::max(minSize, std::min(h / rows, w / cols));
    grid_->setFixedSize(rows * cellSize, cols * cellSize);
    for (auto &row : puzzle_->cells_) {
      for (auto &cell : row) {
        cell->setFixedSize(cellSize, cellSize);
      }
    }
  }

  QSize minimumSizeHint() const override {
    int rows = puzzle_->cells_.size();
    int cols = puzzle_->cells_[0].size();
    int cellSize = CellWidget::kMinimumSize;
    return QSize(rows * 35, cols * 35);
  }

private:
  PuzzleWidget *puzzle_;
  QWidget *grid_;
};

} // namespace cygnus

#endif
