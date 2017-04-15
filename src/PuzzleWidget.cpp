#include "PuzzleWidget.h"

CellWidget::CellWidget(QWidget *parent, bool isBlack, uint32_t num)
    : QFrame(parent) {
  QHBoxLayout *layout = new QHBoxLayout{};
  setLayout(layout);
  auto pal = palette();
  pal.setColor(QPalette::Background, isBlack ? Qt::black : Qt::white);
  setAutoFillBackground(true);
  setPalette(pal);
  setFrameStyle(QFrame::Box | QFrame::Plain);
  show();
}

PuzzleWidget::PuzzleWidget(QWidget *parent,
                           const std::unique_ptr<Puzzle> &puzzle)
    : QWidget(parent) {

  gridLayout_ = new QGridLayout{};

  cells_.clear();
  auto &grid = puzzle->getGrid();
  auto &nums = puzzle->getNums();
  for (uint8_t r = 0; r < puzzle->getHeight(); ++r) {
    std::vector<CellWidget *> cellRow{};
    cellRow.reserve(puzzle->getHeight());
    for (uint8_t c = 0; c < puzzle->getWidth(); ++c) {
      auto cell = new CellWidget(this, puzzle->getGrid()[r][c] == '\0',
                                 puzzle->getNums()[r][c]);
      cell->setGeometry(0, 0, 30, 30);
      cell->setContentsMargins(0, 0, 0, 0);
      cellRow.push_back(cell);
      gridLayout_->addWidget(cell, r, c, 1, 1);
    }
    cells_.push_back(cellRow);
  }

  gridLayout_->setSpacing(0);
  setLayout(gridLayout_);
  show();
}
