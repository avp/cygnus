#include "PuzzleWidget.h"

#include "Colors.h"
#include "FilledLabel.h"
#include "Puzzle.h"

namespace cygnus {

CellWidget::CellWidget(bool isBlack, uint8_t row, uint8_t col,
                       const Puzzle::CellData &cellData,
                       const Puzzle::Markup markup, QWidget *parent)
    : QWidget(parent), row_(row), col_(col), isBlack_(isBlack),
      markup_(markup) {
  QGridLayout *layout = new QGridLayout{};
  setLayout(layout);

  auto pal = palette();
  pal.setColor(QPalette::Background, isBlack_ ? Qt::black : Qt::white);
  setAutoFillBackground(true);
  setPalette(pal);

  numLabel_ = new QLabel{this};
  numLabel_->move(0, 0);
  numLabel_->setStyleSheet("QLabel { padding: 2px 0 0 1px; color: black; }");
  numLabel_->show();

  if (cellData.acrossStart) {
    numLabel_->setText(QString("%1").arg(cellData.acrossNum));
  } else if (cellData.downStart) {
    numLabel_->setText(QString("%1").arg(cellData.downNum));
  } else {
    numLabel_->setText("");
  }
  numLabel_->adjustSize();

  entryLabel_ = new FilledLabel{this};
  entryLabel_->setContentsMargins(0, 0, 0, 0);
  entryLabel_->setMargin(0);
  entryLabel_->setAlignment(Qt::AlignCenter);
  entryLabel_->setStyleSheet("QLabel { padding: 0; }");

  layout->addWidget(entryLabel_, 1, 1, 4, 4);
  layout->setContentsMargins(7, 5, 1, 1);

  layout->setSpacing(0);

  setMouseTracking(true);
}

void CellWidget::enterEvent(QEvent *event) {
  if (displayText_.size() > 3)
    QToolTip::showText(this->mapToGlobal(QPoint(0, 0)), displayText_);
}

void CellWidget::leaveEvent(QEvent *event) { QToolTip::hideText(); }

void CellWidget::resizeEvent(QResizeEvent *event) {
  constexpr int kMaxNumSize = 15;
  constexpr int kMinNumSize = 10;
  int h = height();

  auto f = numLabel_->font();
  f.setPixelSize(std::max(kMinNumSize, std::min(kMaxNumSize, h / 5)));
  numLabel_->setFont(f);

  numLabel_->move(0, 0);
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

void CellWidget::setCell(const QString &text) {
  // qDebug() << "Setting cell:" << text;
  if (isBlack_)
    return;

  auto pal = palette();
  if (text == "-" || text.isEmpty()) {
    displayText_ = "";
  } else if (text.at(0).isUpper()) {
    displayText_ = text;
    pal.setColor(QPalette::Foreground, Qt::black);
    isPencil_ = false;
  } else {
    displayText_ = QString("%1").arg(text.toUpper());
    pal.setColor(QPalette::Foreground, Colors::PENCIL);
    isPencil_ = true;
  }

  if (markup_ & Puzzle::IncorrectTag) {
    pal.setColor(QPalette::Foreground, Qt::red);
    isPencil_ = false;
  }

  entryLabel_->setText(displayText_.left(3) +
                       (displayText_.size() > 3 ? "…" : ""));
  entryLabel_->setPalette(pal);
}

void CellWidget::setMarkup(Puzzle::Markup markup) {
  markup_ = markup;
  if (!entryLabel_->text().isEmpty()) {
    setCell(isPencil_ ? displayText_.toLower() : displayText_.toUpper());
  }
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

void CellWidget::paintEvent(QPaintEvent *pe) {
  QWidget::paintEvent(pe);

  QPainter painter(this);
  painter.setPen({Qt::black, 2});
  painter.drawRect(0, 0, width(), height());

  // Draw a circle if necessary.
  if (markup_ & Puzzle::CircledTag) {
    painter.setPen({Colors::CIRCLE, 1});
    painter.setRenderHint(QPainter::Antialiasing);
    auto radius = height() / 2 - 1;
    auto center = rect().center();
    center.setX(center.x() + 1);
    center.setY(center.y() + 1);
    painter.drawEllipse(center, radius, radius);
  }

  if (markup_ & Puzzle::IncorrectTag) {
    auto pal = palette();
    pal.setColor(QPalette::Foreground, Qt::red);
    setPalette(pal);
  }

  if (markup_ & Puzzle::RevealedTag) {
    painter.setPen({Qt::red, 5});
    painter.setRenderHint(QPainter::Antialiasing);
    auto radius = 2;
    auto center = QPoint{};
    center.setX(rect().left() + 8);
    center.setY(rect().bottom() - 8);
    painter.drawEllipse(center, radius, radius);
  } else if (markup_ & Puzzle::PreviousIncorrectTag) {
    painter.setPen({Qt::black, 5});
    painter.setRenderHint(QPainter::Antialiasing);
    auto radius = 2;
    auto center = QPoint{};
    center.setX(rect().left() + 8);
    center.setY(rect().bottom() - 8);
    painter.drawEllipse(center, radius, radius);
  }
}

PuzzleWidget::PuzzleWidget(const std::unique_ptr<Puzzle> &puzzle,
                           QWidget *parent)
    : QWidget(parent) {
  gridLayout_ = new QGridLayout{};
  resizer_ = new PuzzleResizer{this, gridLayout_};
  resizer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  auto *hbox = new QHBoxLayout{};
  hbox->addWidget(resizer_, 1);
  setLayout(hbox);

  cells_.clear();
  auto &grid = puzzle->getGrid();
  auto &markup = puzzle->getMarkup();
  auto &cellData = puzzle->getCellData();
  auto &rebusFill = puzzle->getRebusFill();
  for (uint8_t r = 0; r < puzzle->getHeight(); ++r) {
    std::vector<CellWidget *> cellRow{};
    cellRow.reserve(puzzle->getHeight());
    for (uint8_t c = 0; c < puzzle->getWidth(); ++c) {
      auto cell = new CellWidget(grid[r][c] == BLACK, r, c, cellData[r][c],
                                 markup[r][c]);
      if (!rebusFill[r][c].isEmpty()) {
        cell->setCell(rebusFill[r][c]);
      } else {
        cell->setCell(QString("%1").arg(grid[r][c]));
      }
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

  resizer_->setContentsMargins(0, 0, 0, 0);
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

void PuzzleWidget::setCell(uint8_t row, uint8_t col, const QString &text) {
  cells_[row][col]->setCell(text);
}

void PuzzleWidget::setMarkup(uint8_t row, uint8_t col, Puzzle::Markup markup) {
  cells_[row][col]->setMarkup(markup);
}

} // namespace cygnus
