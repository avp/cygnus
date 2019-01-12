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
  timerWidget_ = new TimerWidget{};
  infoLayout->addWidget(timerWidget_);

  hLayout->addWidget(acrossWidget_);
  hLayout->addWidget(puzzleContainer_);
  hLayout->addWidget(downWidget_);

  vLayout->addLayout(infoLayout);
  vLayout->addLayout(hLayout);

  window->setLayout(vLayout);
  window->setWindowTitle(tr("Cygnus Crosswords"));

  cursor_.row = 0;
  cursor_.col = 0;
  cursor_.dir = Direction::ACROSS;

  connect(acrossWidget_, &ClueWidget::itemPressed, this,
          &MainWindow::acrossClueClicked);
  connect(downWidget_, &ClueWidget::itemPressed, this,
          &MainWindow::downClueClicked);

  QTimer *timer = new QTimer(this);
  timer->start(1000);
  connect(timer, &QTimer::timeout, this, &MainWindow::tickTimer);
  connect(timerWidget_, &TimerWidget::clicked, this, &MainWindow::toggleTimer);
}

void MainWindow::showMaximized() {
  QMainWindow::showMaximized();

  // Show the open dialog.
  open();
}

void MainWindow::reloadPuzzle() {
  saveAct_->setEnabled(true);
  saveAsAct_->setEnabled(true);
  puzzleMenu_->setEnabled(true);

  titleLabel_->setText(puzzle_->getTitle());
  authorLabel_->setText(puzzle_->getAuthor());
  copyrightLabel_->setText(puzzle_->getCopyright());
  timerWidget_->setCurrent(puzzle_->getTimer().current);
  timerWidget_->setRunning(puzzle_->getTimer().running);

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
  int curClue = puzzle_->getClueIdxByNum(cursor_.dir, curNum);
  int flipClue = puzzle_->getClueIdxByNum(flip(cursor_.dir), flipNum);
  if (cursor_.dir == Direction::ACROSS) {
    acrossWidget_->item(curClue)->setBackground(Qt::white);
    downWidget_->item(flipClue)->setBackground(Qt::white);
  } else {
    downWidget_->item(curClue)->setBackground(Qt::white);
    acrossWidget_->item(flipClue)->setBackground(Qt::white);
  }

  curNum = puzzle_->getNumByPosition(row, col, dir);
  flipNum = puzzle_->getNumByPosition(row, col, flip(dir));
  curClue = puzzle_->getClueIdxByNum(dir, curNum);
  flipClue = puzzle_->getClueIdxByNum(flip(dir), flipNum);

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
  const Clue &clue = puzzle_->getClues(dir)[puzzle_->getClueIdxByNum(dir, num)];
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

  saveAct_ = new QAction(tr("&Save"), this);
  saveAct_->setShortcuts(QKeySequence::Save);
  saveAct_->setStatusTip(tr("Save the current puzzle"));
  connect(saveAct_, &QAction::triggered, this, &MainWindow::save);

  saveAsAct_ = new QAction(tr("Save &As..."), this);
  saveAsAct_->setShortcuts(QKeySequence::SaveAs);
  saveAsAct_->setStatusTip(tr("Save the current puzzle as"));
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

  checkCurrentAct_ = new QAction(tr("Current Letter"), this);
  checkCurrentAct_->setStatusTip(tr("Check the current letter"));
  connect(checkCurrentAct_, &QAction::triggered, this,
          &MainWindow::checkCurrent);

  checkClueAct_ = new QAction(tr("Current Clue"), this);
  checkClueAct_->setStatusTip(tr("Check the current clue"));
  connect(checkClueAct_, &QAction::triggered, this, &MainWindow::checkClue);

  checkAllAct_ = new QAction(tr("Whole Puzzle"), this);
  checkAllAct_->setStatusTip(tr("Check the whole puzzle"));
  connect(checkAllAct_, &QAction::triggered, this, &MainWindow::checkAll);
}

void MainWindow::open() {
  auto fileName =
      QFileDialog::getOpenFileName(this, tr("Open puzzle"), QDir::homePath(),
                                   tr("Across Lite File (*.puz)"));

  if (!fileName.isEmpty()) {
    qDebug() << "Opening file:" << fileName;
    QFile file{fileName};
    if (!file.open(QIODevice::ReadOnly)) {
      return;
    }
    QByteArray puzFile = file.readAll();
    puzzle_ = std::move(Puzzle::loadFromFile(puzFile));
    if (puzzle_) {
      reloadPuzzle();
    } else {
      QMessageBox::warning(
          this, QString("Corrupted File"),
          QString("The file %1 isn't a valid puzzle file.").arg(fileName));
    }
  }
}

void MainWindow::save() {
  QFile file{QDir::home().absoluteFilePath("test.puz")};
  qDebug() << "Saving to:" << file.fileName();
  if (file.open(QIODevice::WriteOnly)) {
    QByteArray bytes = puzzle_->serialize();
    file.write(bytes);
  }
}

void MainWindow::saveAs() {
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
  if (current == BLACK) {
    return;
  }
  if (current == EMPTY || current != solution) {
    setCell(row, col, QString("%1").arg(solution));
  }
  checkSuccess();
}

void MainWindow::revealCurrent() { reveal(cursor_.row, cursor_.col); }

void MainWindow::revealClue() {
  auto num = puzzle_->getNumByPosition(cursor_.row, cursor_.col, cursor_.dir);
  const Clue &clue = puzzle_->getClueByNum(cursor_.dir, num);
  uint8_t r = clue.row;
  uint8_t c = clue.col;
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
    for (uint8_t c = 0; c < puzzle_->getWidth(); ++c) {
      reveal(r, c);
    }
  }
}

bool MainWindow::check(uint8_t row, uint8_t col) {
  qDebug() << "Checking" << row << ',' << col;
  const char solution = puzzle_->getSolution()[row][col];
  const char current = puzzle_->getGrid()[row][col];
  qDebug() << "Current" << current;
  qDebug() << "Solution" << solution;
  if (current == BLACK || current == EMPTY) {
    // Black or empty squares are always considered correct.
    qDebug() << "Result: black or empty";
    return true;
  }
  if ((current == solution) || (current == (solution | 32))) {
    qDebug() << "Result: correct";
    return true;
  }
  // Otherwise, it's incorrect. Mark it as such.
  qDebug() << "Result: incorrect";
  return false;
}

bool MainWindow::checkAndMark(uint8_t row, uint8_t col) {
  if (check(row, col)) {
    return true;
  }
  puzzle_->getMarkup()[row][col] |= Puzzle::IncorrectTag;
  puzzleWidget_->setMarkup(row, col, puzzle_->getMarkup()[row][col]);
  return false;
}

void MainWindow::checkSuccess() {
  // See if the puzzle's complete.
  for (uint8_t r = 0; r < puzzle_->getHeight(); ++r) {
    for (uint8_t c = 0; c < puzzle_->getWidth(); ++c) {
      if (puzzle_->getGrid()[r][c] == EMPTY) {
        return;
      } else if (!check(r, c)) {
        return;
      }
    }
  }

  // Puzzle is complete.
  setTimerStatus(false);
  QMessageBox::information(this, "Congratulations!",
                           "You completeed the puzzle correctly.");
}

void MainWindow::checkCurrent() { checkAndMark(cursor_.row, cursor_.col); }

void MainWindow::checkClue() {
  auto num = puzzle_->getNumByPosition(cursor_.row, cursor_.col, cursor_.dir);
  const Clue &clue = puzzle_->getClueByNum(cursor_.dir, num);
  uint8_t r = clue.row;
  uint8_t c = clue.col;
  if (cursor_.dir == Direction::ACROSS) {
    while (c < puzzle_->getWidth() &&
           puzzle_->getCellData()[r][c].acrossNum == num) {
      checkAndMark(r, c);
      ++c;
    }
  } else {
    while (r < puzzle_->getHeight() &&
           puzzle_->getCellData()[r][c].downNum == num) {
      checkAndMark(r, c);
      ++r;
    }
  }
}

void MainWindow::checkAll() {
  for (uint8_t r = 0; r < puzzle_->getHeight(); ++r) {
    for (uint8_t c = 0; c < puzzle_->getWidth(); ++c) {
      checkAndMark(r, c);
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
  checkMenu_ = puzzleMenu_->addMenu(tr("&Check..."));
  checkMenu_->addAction(checkCurrentAct_);
  checkMenu_->addAction(checkClueAct_);
  checkMenu_->addAction(checkAllAct_);
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
  case Qt::Key_Tab:
    keyTab(false);
    break;
  case Qt::Key_Backtab:
    keyTab(true);
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
  case Qt::Key_Plus:
    if (event->modifiers() & Qt::ControlModifier) {
      acrossWidget_->modifySize(1);
      downWidget_->modifySize(1);
    }
    break;
  case Qt::Key_Minus:
    if (event->modifiers() & Qt::ControlModifier) {
      acrossWidget_->modifySize(-1);
      downWidget_->modifySize(-1);
    }
    break;
  case Qt::Key_Insert:
    QString rebusInput =
        QInputDialog::getText(this, tr("Enter rebus input:"), tr("Letters"));
    if (!rebusInput.isEmpty()) {
      setCell(cursor_.row, cursor_.col, rebusInput.toUpper());
      checkSuccess();
    }
    break;
  }

  if (Qt::Key_A <= event->key() && event->key() <= Qt::Key_Z) {
    if (event->modifiers() == Qt::NoModifier ||
        event->modifiers() == Qt::ShiftModifier) {
      // Convert to lowercase if shift is being held.
      QChar letter = event->key();
      setCell(cursor_.row, cursor_.col,
              QString("%1").arg(event->modifiers() == Qt::ShiftModifier
                                    ? letter.toLower()
                                    : letter.toUpper()));
      checkSuccess();

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

void MainWindow::keyTab(bool reverse) {
  const Direction dir = cursor_.dir;
  auto curNum = puzzle_->getNumByPosition(cursor_.row, cursor_.col, dir);
  auto curIdx = puzzle_->getClueIdxByNum(dir, curNum);
  uint32_t newIdx;
  if (reverse) {
    newIdx = curIdx == 0 ? puzzle_->getClues(dir).size() - 1 : curIdx - 1;
  } else {
    newIdx = (curIdx + (reverse ? -1 : +1)) % puzzle_->getClues(dir).size();
  }
  const Clue &newClue = puzzle_->getClueByIdx(dir, newIdx);
  setCursor(newClue.row, newClue.col, dir);
}

void MainWindow::setCell(uint8_t row, uint8_t col, QString text) {
  if (puzzle_->getGrid()[row][col] == text.at(0).toLatin1()) {
    return;
  }
  puzzle_->getGrid()[row][col] = text.at(0).toLatin1();
  puzzle_->getRebusFill()[row][col] = text;
  puzzleWidget_->setCell(row, col, text);
  Puzzle::Markup &markup = puzzle_->getMarkup()[row][col];
  if (markup & Puzzle::IncorrectTag) {
    markup &= ~Puzzle::IncorrectTag;
    markup |= Puzzle::PreviousIncorrectTag;
  }
  puzzleWidget_->setMarkup(row, col, markup);
}

void MainWindow::clearLetter(uint8_t row, uint8_t col) {
  setCell(row, col, "");
}

void MainWindow::puzzleClicked(uint8_t row, uint8_t col) {
  const auto &grid = puzzle_->getGrid();
  if ((row < puzzle_->getHeight()) && (col < puzzle_->getWidth())) {
    if (grid[row][col] != BLACK) {
      setCursor(row, col, cursor_.dir);
    }
  } else {
    qCritical() << "Click event out of bounds:" << row << col;
  }
}

void MainWindow::puzzleRightClicked() {
  setCursor(cursor_.row, cursor_.col, cursor_.dir == Direction::ACROSS
                                          ? Direction::DOWN
                                          : Direction::ACROSS);
}

void MainWindow::acrossClueClicked(const QListWidgetItem *item) {
  auto idx = acrossWidget_->row(item);
  const Clue &clue = puzzle_->getClueByIdx(Direction::ACROSS, idx);
  setCursor(clue.row, clue.col, Direction::ACROSS);
}

void MainWindow::downClueClicked(const QListWidgetItem *item) {
  auto idx = downWidget_->row(item);
  const Clue &clue = puzzle_->getClueByIdx(Direction::DOWN, idx);
  setCursor(clue.row, clue.col, Direction::DOWN);
}

void MainWindow::tickTimer() {
  if (!puzzle_) {
    return;
  }

  if (puzzle_->getTimer().running) {
    puzzle_->getTimer().current += 1;
    timerWidget_->setCurrent(puzzle_->getTimer().current);
  }
}

void MainWindow::setTimerStatus(bool running) {
  if (!puzzle_) {
    return;
  }

  puzzle_->getTimer().running = running;
  timerWidget_->setRunning(running);
}

void MainWindow::toggleTimer() { setTimerStatus(!puzzle_->getTimer().running); }

} // namespace cygnus
