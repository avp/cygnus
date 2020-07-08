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

  auto size = sizePolicy();
  size.setHeightForWidth(true);
  setSizePolicy(size);

  auto pal = palette();
  pal.setColor(QPalette::ButtonText, Colors::PENCIL);
  if (isBlack_) {
    pal.setColor(QPalette::Base, Qt::black);
    pal.setColor(QPalette::AlternateBase, Qt::black);
    pal.setColor(QPalette::Highlight, Qt::black);
  }
  setPalette(pal);
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);

  numLabel_ = new QLabel{this};
  numLabel_->move(0, 0);
  numLabel_->setContentsMargins(2, 0, 0, 2);
  numLabel_->setForegroundRole(QPalette::Text);
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
  constexpr int kMaxNumSize = 20;
  constexpr int kMinNumSize = 15;
  int h = height();

  auto f = numLabel_->font();
  f.setPixelSize(std::max(kMinNumSize, std::min(kMaxNumSize, h / 5)));
  numLabel_->setFont(f);
  numLabel_->adjustSize();

  numLabel_->move(0, 0);
}

void CellWidget::selectCursor() { setBackgroundRole(QPalette::Highlight); }

void CellWidget::select() { setBackgroundRole(QPalette::AlternateBase); }

void CellWidget::deselect() { setBackgroundRole(QPalette::Base); }

void CellWidget::setCell(const QString &text, bool pencil) {
  qDebug() << "Setting cell:" << text << pencil;
  if (isBlack_)
    return;

  if (text == "-" || text.isEmpty()) {
    displayText_ = " ";
  } else if (pencil) {
    displayText_ = QString("%1").arg(text);
    qDebug() << entryLabel_->palette().buttonText();
    entryLabel_->setForegroundRole(QPalette::ButtonText);
    setForegroundRole(QPalette::ButtonText);
    isPencil_ = true;
  } else {
    displayText_ = text;
    entryLabel_->setForegroundRole(QPalette::Text);
    setForegroundRole(QPalette::Text);
    isPencil_ = false;
  }

  if (markup_ & Puzzle::IncorrectTag) {
    entryLabel_->setForegroundRole(QPalette::BrightText);
    setForegroundRole(QPalette::BrightText);
    isPencil_ = false;
  }

  entryLabel_->setText(displayText_.left(3) +
                       (displayText_.size() > 3 ? "…" : ""));
  qDebug() << entryLabel_->foregroundRole();
  qDebug() << entryLabel_->palette().buttonText()
           << entryLabel_->palette().highlight();
}

void CellWidget::setMarkup(Puzzle::Markup markup) {
  markup_ = markup;
  if (!entryLabel_->text().isEmpty()) {
    setCell(displayText_, isPencil_);
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
    entryLabel_->setForegroundRole(QPalette::BrightText);
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
    painter.setPen({Qt::blue, 5});
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
        cell->setCell(rebusFill[r][c], QChar(grid[r][c]).isLower());
      } else {
        cell->setCell(QString("%1").arg(grid[r][c]).toUpper(),
                      QChar(grid[r][c]).isLower());
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

void PuzzleWidget::setCell(uint8_t row, uint8_t col, const QString &text,
                           bool pencil) {
  cells_[row][col]->setCell(text, pencil);
}

void PuzzleWidget::setMarkup(uint8_t row, uint8_t col, Puzzle::Markup markup) {
  cells_[row][col]->setMarkup(markup);
}

} // namespace cygnus
