#include "MainWindow.h"

#include "Colors.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QtWidgets>
#include <memory>

namespace cygnus {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  QHBoxLayout *layout = new QHBoxLayout{};

  createActions();
  createMenus();

  // Set layout in QWidget
  QWidget *window = new QWidget(this);
  setCentralWidget(window);

  QSizePolicy puzzleContainerSize{QSizePolicy::Preferred,
                                  QSizePolicy::Preferred};
  puzzleContainerSize.setHorizontalStretch(2);

  acrossWidget = createClueWidget();
  downWidget = createClueWidget();

  puzzleContainer = new QWidget{};
  puzzleContainerLayout = new QVBoxLayout{};
  puzzleContainer->setSizePolicy(puzzleContainerSize);
  puzzleContainer->setLayout(puzzleContainerLayout);

  curClueLabel = new QLabel{};
  puzzleContainerLayout->addWidget(curClueLabel);
  puzzleContainerLayout->addStretch();
  auto clueFont = curClueLabel->font();
  clueFont.setPointSize(clueFont.pointSize() + 3);
  curClueLabel->setFont(clueFont);

  // Set QWidget as the central layout of the main window
  layout->addWidget(acrossWidget);
  layout->addWidget(puzzleContainer);
  layout->addWidget(downWidget);
  window->setLayout(layout);

  cursor.row = 0;
  cursor.col = 0;
  cursor.dir = Direction::ACROSS;
}

void MainWindow::reloadPuzzle() {
  acrossWidget->clear();
  for (const auto &clue : puzzle->getClues(Direction::ACROSS)) {
    acrossWidget->addItem(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }
  downWidget->clear();
  for (const auto &clue : puzzle->getClues(Direction::DOWN)) {
    downWidget->addItem(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }

  if (puzzleWidget) {
    puzzleWidget->deleteLater();
  }
  puzzleWidget = new PuzzleWidget{puzzle};
  puzzleContainerLayout->insertWidget(1, puzzleWidget);
  puzzleContainerLayout->setAlignment(puzzleWidget,
                                      Qt::AlignHCenter | Qt::AlignTop);

  // Set cursor to first non-blank square.
  const auto &across = puzzle->getClues(Direction::ACROSS);
  if (across.size() > 0) {
    setCursor(across[0].row, across[0].col, Direction::ACROSS);
  } else {
    qCritical() << "No across clues in puzzle";
    return;
  }

  // Connect the signals from the puzzle.
  connect(puzzleWidget, &PuzzleWidget::clicked, this,
          &MainWindow::puzzleClicked);
  connect(puzzleWidget, &PuzzleWidget::rightClicked, this,
          &MainWindow::puzzleRightClicked);
}

void MainWindow::setCursor(uint8_t row, uint8_t col, Direction dir) {
  const auto &grid = puzzle->getGrid();

  // Clear current selection.
  if (cursor.dir == Direction::ACROSS) {
    for (uint8_t i = 0; cursor.col + i < puzzle->getWidth() &&
                        grid[cursor.row][cursor.col + i] != BLACK;
         ++i) {
      puzzleWidget->deselectPosition(cursor.row, cursor.col + i);
    }
    for (uint8_t i = 0;
         cursor.col - i >= 0 && grid[cursor.row][cursor.col - i] != BLACK;
         ++i) {
      puzzleWidget->deselectPosition(cursor.row, cursor.col - i);
    }
  } else {
    for (uint8_t i = 0; cursor.row + i < puzzle->getHeight() &&
                        grid[cursor.row + i][cursor.col] != BLACK;
         ++i) {
      puzzleWidget->deselectPosition(cursor.row + i, cursor.col);
    }
    for (uint8_t i = 0;
         cursor.row - i >= 0 && grid[cursor.row - i][cursor.col] != BLACK;
         ++i) {
      puzzleWidget->deselectPosition(cursor.row - i, cursor.col);
    }
  }

  // Select at new cursor position.
  if (dir == Direction::ACROSS) {
    for (uint8_t i = 0;
         col + i < puzzle->getWidth() && grid[row][col + i] != BLACK; ++i) {
      puzzleWidget->selectPosition(row, col + i);
    }
    for (uint8_t i = 0; col - i >= 0 && grid[row][col - i] != BLACK; ++i) {
      puzzleWidget->selectPosition(row, col - i);
    }
  } else {
    for (uint8_t i = 0;
         row + i < puzzle->getHeight() && grid[row + i][col] != BLACK; ++i) {
      puzzleWidget->selectPosition(row + i, col);
    }
    for (uint8_t i = 0; row - i >= 0 && grid[row - i][col] != BLACK; ++i) {
      puzzleWidget->selectPosition(row - i, col);
    }
  }

  uint32_t curNum =
      puzzle->getNumByPosition(cursor.row, cursor.col, cursor.dir);
  uint32_t flipNum =
      puzzle->getNumByPosition(cursor.row, cursor.col, flip(cursor.dir));
  if (cursor.dir == Direction::ACROSS) {
    acrossWidget->item(puzzle->getClueByNum(cursor.dir, curNum))
        ->setBackground(Qt::white);
    downWidget->item(puzzle->getClueByNum(flip(cursor.dir), flipNum))
        ->setBackground(Qt::white);
  } else {
    downWidget->item(puzzle->getClueByNum(cursor.dir, curNum))
        ->setBackground(Qt::white);
    acrossWidget->item(puzzle->getClueByNum(flip(cursor.dir), flipNum))
        ->setBackground(Qt::white);
  }

  curNum = puzzle->getNumByPosition(row, col, dir);
  flipNum = puzzle->getNumByPosition(row, col, flip(dir));

  if (dir == Direction::ACROSS) {
    acrossWidget->item(puzzle->getClueByNum(dir, curNum))
        ->setBackground(Colors::PRIMARY_HIGHLIGHT);
    downWidget->item(puzzle->getClueByNum(flip(dir), flipNum))
        ->setBackground(Colors::SECONDARY_HIGHLIGHT);
  } else {
    downWidget->item(puzzle->getClueByNum(dir, curNum))
        ->setBackground(Colors::PRIMARY_HIGHLIGHT);
    acrossWidget->item(puzzle->getClueByNum(flip(dir), flipNum))
        ->setBackground(Colors::SECONDARY_HIGHLIGHT);
  }

  puzzleWidget->selectCursorPosition(row, col);
  cursor.row = row;
  cursor.col = col;
  cursor.dir = dir;

  uint32_t num = curNum;
  const Clue &clue = puzzle->getClues(dir)[puzzle->getClueByNum(dir, num)];
  curClueLabel->setText(QString{"%1. %2"}.arg(clue.num).arg(clue.clue));
}

QListWidget *MainWindow::createClueWidget() {
  auto result = new QListWidget{};
  QSizePolicy cluesSize{QSizePolicy::Preferred, QSizePolicy::Preferred};
  cluesSize.setHorizontalStretch(1);
  result->setFrameStyle(QFrame::NoFrame);
  result->setSizePolicy(cluesSize);
  result->setWordWrap(true);
  result->setFocusPolicy(Qt::NoFocus);
  return result;
}

void MainWindow::createActions() {
  openAct = new QAction(tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, &QAction::triggered, this, &MainWindow::open);

  saveAct = new QAction(tr("&Save..."), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the current puzzle"));
  connect(saveAct, &QAction::triggered, this, &MainWindow::save);
}

void MainWindow::open() {
  auto fileName =
      QFileDialog::getOpenFileName(this, tr("Open Puzzle"), QDir::homePath(),
                                   tr("Across Lite File (*.puz)"));

  if (!fileName.isEmpty()) {
    qDebug() << "Opening file:" << fileName;
    QFile file{fileName};
    if (!file.open(QIODevice::ReadOnly)) {
      return;
    }
    QByteArray puzFile = file.readAll();
    puzzle.reset(Puzzle::loadFromFile(puzFile));
    if (puzzle) {
      // TODO: Handle null puzzle (failure case).
      reloadPuzzle();
    }
  }
}

void MainWindow::save() {
  if (!puzzle) {
    return;
  }

  QFile file{"/Users/avp/test.puz"};
  if (file.open(QIODevice::WriteOnly)) {
    QByteArray bytes = puzzle->serialize();
    file.write(bytes);
  }
}

void MainWindow::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);
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
  }
}

void MainWindow::keyUp() {
  const auto &grid = puzzle->getGrid();
  if (cursor.dir == Direction::ACROSS) {
    setCursor(cursor.row, cursor.col, Direction::DOWN);
    return;
  }
  uint8_t row = cursor.row;
  do {
    if (row > 0) {
      --row;
    } else {
      return;
    }
  } while (grid[row][cursor.col] == BLACK);
  setCursor(row, cursor.col, Direction::DOWN);
}

void MainWindow::keyDown() {
  const auto &grid = puzzle->getGrid();
  if (cursor.dir == Direction::ACROSS) {
    setCursor(cursor.row, cursor.col, Direction::DOWN);
    return;
  }
  uint8_t row = cursor.row;
  do {
    if (row < puzzle->getHeight() - 1) {
      ++row;
    } else {
      return;
    }
  } while (grid[row][cursor.col] == BLACK);
  setCursor(row, cursor.col, Direction::DOWN);
}

void MainWindow::keyLeft() {
  const auto &grid = puzzle->getGrid();
  if (cursor.dir == Direction::DOWN) {
    setCursor(cursor.row, cursor.col, Direction::ACROSS);
    return;
  }
  uint8_t col = cursor.col;
  do {
    if (col > 0) {
      --col;
    } else {
      return;
    }
  } while (grid[cursor.row][col] == BLACK);
  setCursor(cursor.row, col, Direction::ACROSS);
}

void MainWindow::keyRight() {
  const auto &grid = puzzle->getGrid();
  if (cursor.dir == Direction::DOWN) {
    setCursor(cursor.row, cursor.col, Direction::ACROSS);
    return;
  }
  uint8_t col = cursor.col;
  do {
    if (col < puzzle->getWidth() - 1) {
      ++col;
    } else {
      return;
    }
  } while (grid[cursor.row][col] == BLACK);
  setCursor(cursor.row, col, Direction::ACROSS);
}

void MainWindow::puzzleClicked(uint8_t row, uint8_t col) {
  const auto &grid = puzzle->getGrid();
  if ((0 <= row && row < puzzle->getHeight()) &&
      (0 <= col && col < puzzle->getWidth())) {
    if (grid[row][col] != BLACK) {
      setCursor(row, col, cursor.dir);
    }
  } else {
    qCritical() << "Click event out of bounds:" << row << col;
  }
}

void MainWindow::puzzleRightClicked() {
  setCursor(cursor.row, cursor.col,
            cursor.dir == Direction::ACROSS ? Direction::DOWN
                                            : Direction::ACROSS);
}

} // namespace cygnus
