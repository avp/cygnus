#include "PuzzleWidget.h"

namespace cygnus {

CellWidget::CellWidget(bool isBlack, uint32_t num, QWidget *parent)
    : QFrame(parent) {
  QGridLayout *layout = new QGridLayout{};
  setLayout(layout);
  auto pal = palette();
  pal.setColor(QPalette::Background, isBlack ? Qt::black : Qt::white);
  setAutoFillBackground(true);
  setPalette(pal);
  setFrameStyle(QFrame::Box);

  QLabel *numLabel = new QLabel{};
  numLabel->setText(num == 0 ? "" : QString("%1").arg(num));
  numLabel->setContentsMargins(1, 1, 1, 1);
  numLabel->setMargin(0);
  numLabel->setAlignment(Qt::AlignCenter);
  auto numFont = numLabel->font();
  numFont.setPointSize(numFont.pointSize() - 3);
  numLabel->setFont(numFont);

  QLabel *entryLabel = new QLabel{};
  entryLabel->setContentsMargins(0, 0, 0, 0);
  entryLabel->setMargin(0);
  entryLabel->setAlignment(Qt::AlignCenter);
  auto entryFont = entryLabel->font();
  entryFont.setPointSize(entryFont.pointSize() + 2);
  entryFont.setBold(true);
  entryLabel->setFont(entryFont);

  layout->addWidget(numLabel, 0, 0, 1, 1);
  layout->addWidget(entryLabel, 1, 0, 3, 4);
  layout->setContentsMargins(0, 0, 0, 0);

  layout->setSpacing(0);
}

PuzzleWidget::PuzzleWidget(const std::unique_ptr<Puzzle> &puzzle,
                           QWidget *parent)
    : QFrame(parent) {
  setFrameStyle(QFrame::Box);

  gridLayout_ = new QGridLayout{};

  cells_.clear();
  auto &grid = puzzle->getGrid();
  auto &nums = puzzle->getNums();
  for (uint8_t r = 0; r < puzzle->getHeight(); ++r) {
    std::vector<CellWidget *> cellRow{};
    cellRow.reserve(puzzle->getHeight());
    for (uint8_t c = 0; c < puzzle->getWidth(); ++c) {
      auto cell = new CellWidget(puzzle->getGrid()[r][c] == '\0',
                                 puzzle->getNums()[r][c]);
      cell->setFixedSize(40, 40);
      cell->setContentsMargins(0, 0, 0, 0);
      cellRow.push_back(cell);
      gridLayout_->addWidget(cell, r, c, 1, 1);
    }
    cells_.push_back(cellRow);
  }

  gridLayout_->setSpacing(0);
  gridLayout_->setContentsMargins(0, 0, 0, 0);

  setLayout(gridLayout_);
}

} // namespace cygnus
