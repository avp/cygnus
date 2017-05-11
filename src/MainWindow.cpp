#include "MainWindow.h"

#include "Colors.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QtWidgets>
#include <memory>

namespace cygnus {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  QVBoxLayout *vLayout = new QVBoxLayout{};

  QHBoxLayout *infoLayout = new QHBoxLayout{};
  QHBoxLayout *hLayout = new QHBoxLayout{};

  createActions();
  createMenus();

  // Set layout in QWidget
  QWidget *window = new QWidget(this);
  setCentralWidget(window);

  QSizePolicy puzzleContainerSize{QSizePolicy::MinimumExpanding,
                                  QSizePolicy::MinimumExpanding};
  puzzleContainerSize.setHorizontalStretch(2);

  acrossWidget_ = createClueWidget();
  downWidget_ = createClueWidget();

  puzzleContainer_ = new QWidget{};
  puzzleContainerLayout_ = new QVBoxLayout{};
  puzzleContainer_->setSizePolicy(puzzleContainerSize);
  puzzleContainer_->setLayout(puzzleContainerLayout_);

  curClueLabel_ = new QLabel{};
  puzzleContainerLayout_->addWidget(curClueLabel_);
  puzzleContainerLayout_->addStretch();
  auto clueFont = curClueLabel_->font();
  clueFont.setPointSize(clueFont.pointSize() + 3);
  curClueLabel_->setFont(clueFont);

  titleLabel_ = new QLabel{};
  infoLayout->addWidget(titleLabel_);
  authorLabel_ = new QLabel{};
  infoLayout->addWidget(authorLabel_);
  copyrightLabel_ = new QLabel{};
  infoLayout->addWidget(copyrightLabel_);

  hLayout->addWidget(acrossWidget_);
  hLayout->addWidget(puzzleContainer_);
  hLayout->addWidget(downWidget_);

  vLayout->addLayout(infoLayout);
  vLayout->addLayout(hLayout);

  window->setLayout(vLayout);

  cursor_.row = 0;
  cursor_.col = 0;
  cursor_.dir = Direction::ACROSS;

  connect(acrossWidget_, &ClueWidget::itemPressed, this,
          &MainWindow::acrossClueClicked);
  connect(downWidget_, &ClueWidget::itemPressed, this,
          &MainWindow::downClueClicked);
}

void MainWindow::reloadPuzzle() {
  saveAct_->setEnabled(true);
  saveAsAct_->setEnabled(true);
  puzzleMenu_->setEnabled(true);

  titleLabel_->setText(puzzle_->getTitle());
  authorLabel_->setText(puzzle_->getAuthor());
  copyrightLabel_->setText(puzzle_->getCopyright());

  acrossWidget_->clear();
  for (const auto &clue : puzzle_->getClues(Direction::ACROSS)) {
    acrossWidget_->addItem(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }

  downWidget_->clear();
  for (const auto &clue : puzzle_->getClues(Direction::DOWN)) {
    downWidget_->addItem(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }

  if (puzzleWidget_) {
    delete puzzleWidget_;
  }
  puzzleWidget_ = new PuzzleWidget{puzzle_};
  puzzleContainerLayout_->insertWidget(1, puzzleWidget_);
  puzzleContainerLayout_->setAlignment(puzzleWidget_,
                                       Qt::AlignHCenter | Qt::AlignTop);

  auto puzzleSize = std::min(this->height(), this->width()) - 200;
  puzzleWidget_->setFixedSize(puzzleSize, puzzleSize);

  // Set cursor_ to first non-blank square.
  const auto &across = puzzle_->getClues(Direction::ACROSS);
  if (across.size() > 0) {
    setCursor(across[0].row, across[0].col, Direction::ACROSS);
  } else {
    qCritical() << "No across clues in puzzle_";
    return;
  }

  // Connect the signals from the puzzle_.
  connect(puzzleWidget_, &PuzzleWidget::clicked, this,
          &MainWindow::puzzleClicked);
  connect(puzzleWidget_, &PuzzleWidget::rightClicked, this,
          &MainWindow::puzzleRightClicked);
}

void MainWindow::setCursor(uint8_t row, uint8_t col, Direction dir) {
  const auto &grid = puzzle_->getGrid();

  // Clear current selection.
  if (cursor_.dir == Direction::ACROSS) {
    for (uint8_t i = 0; cursor_.col + i < puzzle_->getWidth() &&
                        grid[cursor_.row][cursor_.col + i] != BLACK;
         ++i) {
      puzzleWidget_->deselectPosition(cursor_.row, cursor_.col + i);
    }
    for (uint8_t i = 0;
         cursor_.col - i >= 0 && grid[cursor_.row][cursor_.col - i] != BLACK;
         ++i) {
      puzzleWidget_->deselectPosition(cursor_.row, cursor_.col - i);
    }
  } else {
    for (uint8_t i = 0; cursor_.row + i < puzzle_->getHeight() &&
                        grid[cursor_.row + i][cursor_.col] != BLACK;
         ++i) {
      puzzleWidget_->deselectPosition(cursor_.row + i, cursor_.col);
    }
    for (uint8_t i = 0;
         cursor_.row - i >= 0 && grid[cursor_.row - i][cursor_.col] != BLACK;
         ++i) {
      puzzleWidget_->deselectPosition(cursor_.row - i, cursor_.col);
    }
  }

  // Select at new cursor_ position.
  if (dir == Direction::ACROSS) {
    for (uint8_t i = 0;
         col + i < puzzle_->getWidth() && grid[row][col + i] != BLACK; ++i) {
      puzzleWidget_->selectPosition(row, col + i);
    }
    for (uint8_t i = 0; col - i >= 0 && grid[row][col - i] != BLACK; ++i) {
      puzzleWidget_->selectPosition(row, col - i);
    }
  } else {
    for (uint8_t i = 0;
         row + i < puzzle_->getHeight() && grid[row + i][col] != BLACK; ++i) {
      puzzleWidget_->selectPosition(row + i, col);
    }
    for (uint8_t i = 0; row - i >= 0 && grid[row - i][col] != BLACK; ++i) {
      puzzleWidget_->selectPosition(row - i, col);
    }
  }

  uint32_t curNum =
      puzzle_->getNumByPosition(cursor_.row, cursor_.col, cursor_.dir);
  uint32_t flipNum =
      puzzle_->getNumByPosition(cursor_.row, cursor_.col, flip(cursor_.dir));
  int curClue = puzzle_->getClueByNum(cursor_.dir, curNum);
  int flipClue = puzzle_->getClueByNum(flip(cursor_.dir), flipNum);
  if (cursor_.dir == Direction::ACROSS) {
    acrossWidget_->item(curClue)->setBackground(Qt::white);
    downWidget_->item(flipClue)->setBackground(Qt::white);
  } else {
    downWidget_->item(curClue)->setBackground(Qt::white);
    acrossWidget_->item(flipClue)->setBackground(Qt::white);
  }

  curNum = puzzle_->getNumByPosition(row, col, dir);
  flipNum = puzzle_->getNumByPosition(row, col, flip(dir));
  curClue = puzzle_->getClueByNum(dir, curNum);
  flipClue = puzzle_->getClueByNum(flip(dir), flipNum);

  QPalette pal;

  if (dir == Direction::ACROSS) {
    acrossWidget_->setCurrentRow(curClue);
    downWidget_->setCurrentRow(flipClue);

    acrossWidget_->setPrimary();
    downWidget_->setSecondary();
  } else {
    downWidget_->setCurrentRow(curClue);
    acrossWidget_->setCurrentRow(flipClue);

    downWidget_->setPrimary();
    acrossWidget_->setSecondary();
  }

  puzzleWidget_->selectCursorPosition(row, col);
  cursor_.row = row;
  cursor_.col = col;
  cursor_.dir = dir;

  uint32_t num = curNum;
  const Clue &clue = puzzle_->getClues(dir)[puzzle_->getClueByNum(dir, num)];
  curClueLabel_->setText(QString{"%1. %2"}.arg(clue.num).arg(clue.clue));
}

ClueWidget *MainWindow::createClueWidget() {
  auto result = new ClueWidget{};
  QSizePolicy cluesSize{QSizePolicy::Preferred, QSizePolicy::Preferred};
  cluesSize.setHorizontalStretch(1);
  result->setSizePolicy(cluesSize);
  result->setWordWrap(true);
  result->setFocusPolicy(Qt::NoFocus);
  return result;
}

void MainWindow::createActions() {
  openAct_ = new QAction(tr("&Open..."), this);
  openAct_->setShortcuts(QKeySequence::Open);
  openAct_->setStatusTip(tr("Open an existing file"));
  connect(openAct_, &QAction::triggered, this, &MainWindow::open);

  saveAct_ = new QAction(tr("&Save..."), this);
  saveAct_->setShortcuts(QKeySequence::Save);
  saveAct_->setStatusTip(tr("Save the current puzzle"));
  connect(saveAct_, &QAction::triggered, this, &MainWindow::save);

  saveAsAct_ = new QAction(tr("Save &As..."), this);
  saveAsAct_->setShortcuts(QKeySequence::SaveAs);
  saveAsAct_->setStatusTip(tr("Save the current puzzle as..."));
  connect(saveAsAct_, &QAction::triggered, this, &MainWindow::saveAs);

  revealCurrentAct_ = new QAction(tr("Current Letter"), this);
  revealCurrentAct_->setStatusTip(tr("Reveal the current letter"));
  connect(revealCurrentAct_, &QAction::triggered, this,
          &MainWindow::revealCurrent);

  revealClueAct_ = new QAction(tr("Current Clue"), this);
  revealClueAct_->setStatusTip(tr("Reveal the current clue"));
  connect(revealClueAct_, &QAction::triggered, this, &MainWindow::revealClue);

  revealAllAct_ = new QAction(tr("Whole Puzzle"), this);
  revealAllAct_->setStatusTip(tr("Reveal the whole puzzle"));
  connect(revealAllAct_, &QAction::triggered, this, &MainWindow::revealAll);
}

void MainWindow::open() {
  auto fileName =
      QFileDialog::getOpenFileName(this, tr("Open puzzle_"), QDir::homePath(),
                                   tr("Across Lite File (*.puz)"));

  if (!fileName.isEmpty()) {
    qDebug() << "Opening file:" << fileName;
    QFile file{fileName};
    if (!file.open(QIODevice::ReadOnly)) {
      return;
    }
    QByteArray puzFile = file.readAll();
    puzzle_.reset(Puzzle::loadFromFile(puzFile));
    if (puzzle_) {
      // TODO: Handle null puzzle_ (failure case).
      reloadPuzzle();
    } else {
      QMessageBox::warning(
          this, QString("Corrupted File"),
          QString("The file %1 isn't a valid puzzle file.").arg(fileName));
    }
  }
}

void MainWindow::save() {
  if (!puzzle_) {
    return;
  }

  QFile file{QDir::home().absoluteFilePath("test.puz")};
  qDebug() << "Saving to:" << file.fileName();
  if (file.open(QIODevice::WriteOnly)) {
    QByteArray bytes = puzzle_->serialize();
    file.write(bytes);
  }
}

void MainWindow::saveAs() {
  if (!puzzle_) {
    return;
  }

  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Save Puzzle"), "",
      tr("Across Lite File (*.puz);;All Files (*)"));

  QFile file{fileName};
  qDebug() << "Saving to:" << file.fileName();
  if (file.open(QIODevice::WriteOnly)) {
    QByteArray bytes = puzzle_->serialize();
    file.write(bytes);
  }
}

void MainWindow::reveal(uint8_t row, uint8_t col) {
  char solution = puzzle_->getSolution()[row][col];
  char current = puzzle_->getGrid()[row][col];
  if (current == EMPTY || current != solution) {
    setLetter(row, col, solution);
  }
}

void MainWindow::revealCurrent() { reveal(cursor_.row, cursor_.col); }

void MainWindow::revealClue() {
  const auto num =
      puzzle_->getNumByPosition(cursor_.row, cursor_.col, cursor_.dir);
  const auto idx = puzzle_->getClueByNum(cursor_.dir, num);
  const auto start = puzzle_->getPositionFromClue(cursor_.dir, idx);
  uint8_t r = start.first;
  uint8_t c = start.second;
  if (cursor_.dir == Direction::ACROSS) {
    while (c < puzzle_->getWidth() &&
           puzzle_->getCellData()[r][c].acrossNum == num) {
      reveal(r, c);
      ++c;
    }
  } else {
    while (r < puzzle_->getHeight() &&
           puzzle_->getCellData()[r][c].downNum == num) {
      reveal(r, c);
      ++r;
    }
  }
}

void MainWindow::revealAll() {
  for (uint8_t r = 0; r < puzzle_->getHeight(); ++r) {
    for (uint8_t c = 0; c < puzzle_->getHeight(); ++c) {
      if (puzzle_->getGrid()[r][c] != BLACK) {
        revealCurrent();
      }
    }
  }
}

void MainWindow::createMenus() {
  fileMenu_ = menuBar()->addMenu(tr("&File"));
  fileMenu_->addAction(openAct_);
  fileMenu_->addAction(saveAct_);
  saveAct_->setEnabled(false);
  fileMenu_->addAction(saveAsAct_);
  saveAsAct_->setEnabled(false);

  puzzleMenu_ = menuBar()->addMenu(tr("&Puzzle"));
  puzzleMenu_->setEnabled(false);
  revealMenu_ = puzzleMenu_->addMenu(tr("&Reveal..."));
  revealMenu_->addAction(revealCurrentAct_);
  revealMenu_->addAction(revealClueAct_);
  revealMenu_->addAction(revealAllAct_);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_Up:
    keyUp();
    break;
  case Qt::Key_Down:
    keyDown();
    break;
  case Qt::Key_Left:
    keyLeft();
    break;
  case Qt::Key_Right:
    keyRight();
    break;
  case Qt::Key_Space:
    setCursor(cursor_.row, cursor_.col, flip(cursor_.dir));
    break;
  case Qt::Key_Backspace:
    clearLetter(cursor_.row, cursor_.col);
    if (cursor_.dir == Direction::ACROSS) {
      keyLeft();
    } else {
      keyUp();
    }
    break;
  case Qt::Key_Delete:
    clearLetter(cursor_.row, cursor_.col);
    break;
  }

  if (Qt::Key_A <= event->key() && event->key() <= Qt::Key_Z) {
    if (event->modifiers() == Qt::NoModifier ||
        event->modifiers() == Qt::ShiftModifier) {
      setLetter(cursor_.row, cursor_.col, static_cast<char>(event->key()));

      if (cursor_.dir == Direction::ACROSS) {
        keyRight();
      } else {
        keyDown();
      }
    }
  }
}

void MainWindow::keyUp() {
  const auto &grid = puzzle_->getGrid();
  if (cursor_.dir == Direction::ACROSS) {
    setCursor(cursor_.row, cursor_.col, Direction::DOWN);
    return;
  }
  uint8_t row = cursor_.row;
  do {
    if (row > 0) {
      --row;
    } else {
      return;
    }
  } while (grid[row][cursor_.col] == BLACK);
  setCursor(row, cursor_.col, Direction::DOWN);
}

void MainWindow::keyDown() {
  const auto &grid = puzzle_->getGrid();
  if (cursor_.dir == Direction::ACROSS) {
    setCursor(cursor_.row, cursor_.col, Direction::DOWN);
    return;
  }
  uint8_t row = cursor_.row;
  do {
    if (row < puzzle_->getHeight() - 1) {
      ++row;
    } else {
      return;
    }
  } while (grid[row][cursor_.col] == BLACK);
  setCursor(row, cursor_.col, Direction::DOWN);
}

void MainWindow::keyLeft() {
  const auto &grid = puzzle_->getGrid();
  if (cursor_.dir == Direction::DOWN) {
    setCursor(cursor_.row, cursor_.col, Direction::ACROSS);
    return;
  }
  uint8_t col = cursor_.col;
  do {
    if (col > 0) {
      --col;
    } else {
      return;
    }
  } while (grid[cursor_.row][col] == BLACK);
  setCursor(cursor_.row, col, Direction::ACROSS);
}

void MainWindow::keyRight() {
  const auto &grid = puzzle_->getGrid();
  if (cursor_.dir == Direction::DOWN) {
    setCursor(cursor_.row, cursor_.col, Direction::ACROSS);
    return;
  }
  uint8_t col = cursor_.col;
  do {
    if (col < puzzle_->getWidth() - 1) {
      ++col;
    } else {
      return;
    }
  } while (grid[cursor_.row][col] == BLACK);
  setCursor(cursor_.row, col, Direction::ACROSS);
}

void MainWindow::setLetter(uint8_t row, uint8_t col, char letter) {
  puzzle_->getGrid()[row][col] = letter;
  puzzleWidget_->setLetter(row, col, letter);
  puzzle_->dumpGrid(qDebug());
}

void MainWindow::clearLetter(uint8_t row, uint8_t col) {
  setLetter(row, col, EMPTY);
}

void MainWindow::puzzleClicked(uint8_t row, uint8_t col) {
  const auto &grid = puzzle_->getGrid();
  if ((0 <= row && row < puzzle_->getHeight()) &&
      (0 <= col && col < puzzle_->getWidth())) {
    if (grid[row][col] != BLACK) {
      setCursor(row, col, cursor_.dir);
    }
  } else {
    qCritical() << "Click event out of bounds:" << row << col;
  }
}

void MainWindow::puzzleRightClicked() {
  setCursor(cursor_.row, cursor_.col,
            cursor_.dir == Direction::ACROSS ? Direction::DOWN
                                             : Direction::ACROSS);
}

void MainWindow::acrossClueClicked(const QListWidgetItem *item) {
  auto idx = acrossWidget_->row(item);
  auto pos = puzzle_->getPositionFromClue(Direction::ACROSS, idx);
  setCursor(pos.first, pos.second, Direction::ACROSS);
}

void MainWindow::downClueClicked(const QListWidgetItem *item) {
  auto idx = downWidget_->row(item);
  auto pos = puzzle_->getPositionFromClue(Direction::DOWN, idx);
  setCursor(pos.first, pos.second, Direction::DOWN);
}

} // namespace cygnus
