#include "PuzzleWidget.h"

#include "Colors.h"

namespace cygnus {

CellWidget::CellWidget(bool isBlack, uint8_t row, uint8_t col, uint32_t num,
                       QWidget *parent)
    : QFrame(parent), row_(row), col_(col), isBlack_(isBlack) {
  QGridLayout *layout = new QGridLayout{};
  setLayout(layout);
  auto pal = palette();
  pal.setColor(QPalette::Background, isBlack_ ? Qt::black : Qt::white);
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

  entryLabel_ = new QLabel{};
  entryLabel_->setContentsMargins(0, 0, 0, 0);
  entryLabel_->setMargin(0);
  entryLabel_->setAlignment(Qt::AlignCenter);
  auto entryFont = entryLabel_->font();
  entryFont.setPointSize(entryFont.pointSize() + 2);
  entryFont.setBold(true);
  entryLabel_->setFont(entryFont);

  layout->addWidget(numLabel, 0, 0, 1, 1);
  layout->addWidget(entryLabel_, 1, 0, 3, 4);
  layout->setContentsMargins(0, 0, 0, 0);

  layout->setSpacing(0);
}

void CellWidget::selectCursor() {
  auto pal = palette();
  pal.setColor(QPalette::Background,
               isBlack_ ? Qt::black : Colors::PRIMARY_HIGHLIGHT);
  setPalette(pal);
}

void CellWidget::select() {
  auto pal = palette();
  pal.setColor(QPalette::Background,
               isBlack_ ? Qt::black : Colors::SECONDARY_HIGHLIGHT);
  setPalette(pal);
}

void CellWidget::deselect() {
  auto pal = palette();
  pal.setColor(QPalette::Background, isBlack_ ? Qt::black : Qt::white);
  setPalette(pal);
}

void CellWidget::setLetter(char letter) {
  entryLabel_->setText(QString("%1").arg(letter != EMPTY ? letter : ' '));
}

void CellWidget::mousePressEvent(QMouseEvent *event) {
  switch (event->button()) {
  case Qt::LeftButton:
    return clicked(row_, col_);
  case Qt::RightButton:
    return rightClicked();
  default:
    return;
  }
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
      auto cell = new CellWidget(puzzle->getGrid()[r][c] == BLACK, r, c,
                                 puzzle->getNums()[r][c]);
      cell->setFixedSize(40, 40);
      cell->setContentsMargins(0, 0, 0, 0);
      cellRow.push_back(cell);
      gridLayout_->addWidget(cell, r, c, 1, 1);

      connect(cell, &CellWidget::clicked, this, &PuzzleWidget::cellClicked);
      connect(cell, &CellWidget::rightClicked, this,
              &PuzzleWidget::cellRightClicked);
    }
    cells_.push_back(cellRow);
  }

  gridLayout_->setSpacing(0);
  gridLayout_->setContentsMargins(0, 0, 0, 0);

  setLayout(gridLayout_);
}

void PuzzleWidget::selectCursorPosition(uint8_t row, uint8_t col) {
  cells_[row][col]->selectCursor();
}

void PuzzleWidget::selectPosition(uint8_t row, uint8_t col) {
  cells_[row][col]->select();
}

void PuzzleWidget::deselectPosition(uint8_t row, uint8_t col) {
  cells_[row][col]->deselect();
}

void PuzzleWidget::setLetter(uint8_t row, uint8_t col, char letter) {
  cells_[row][col]->setLetter(letter);
}

} // namespace cygnus
