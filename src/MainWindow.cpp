#include "MainWindow.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QtWidgets>

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
  puzzleContainerLayout = new QHBoxLayout{};
  puzzleContainer->setSizePolicy(puzzleContainerSize);
  puzzleContainer->setLayout(puzzleContainerLayout);

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
  for (const auto &clue : puzzle->getAcross()) {
    acrossWidget->addItem(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }
  downWidget->clear();
  for (const auto &clue : puzzle->getDown()) {
    downWidget->addItem(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }

  if (puzzleWidget) {
    puzzleWidget->deleteLater();
  }
  puzzleWidget = new PuzzleWidget{puzzle};
  puzzleContainerLayout->addWidget(puzzleWidget);
  puzzleContainerLayout->setAlignment(puzzleWidget, Qt::AlignCenter);

  // Set cursor to first non-blank square.
  const auto &across = puzzle->getAcross();
  if (across.size() > 0) {
    setCursor(across[0].row, across[0].col, Direction::ACROSS);
  } else {
    qCritical() << "No across clues in puzzle";
    return;
  }
}

void MainWindow::setCursor(uint8_t row, uint8_t col, Direction dir) {
  const auto &grid = puzzle->getGrid();

  // Clear current selection.
  if (cursor.dir == Direction::ACROSS) {
    for (uint8_t i = 0; cursor.col + i < puzzle->getWidth() &&
                        grid[cursor.row][cursor.col + i] != '\0';
         ++i) {
      puzzleWidget->deselectPosition(cursor.row, cursor.col + i);
    }
    for (uint8_t i = 0;
         cursor.col - i >= 0 && grid[cursor.row][cursor.col - i] != '\0'; --i) {
      puzzleWidget->deselectPosition(cursor.row, cursor.col - i);
    }
  } else {
    for (uint8_t i = 0; cursor.row + i < puzzle->getHeight() &&
                        grid[cursor.row + i][cursor.col] != '\0';
         ++i) {
      puzzleWidget->deselectPosition(row, cursor.col + i);
    }
    for (uint8_t i = 0;
         cursor.row - i >= 0 && grid[cursor.row - i][cursor.col] != '\0'; --i) {
      puzzleWidget->deselectPosition(cursor.row - i, cursor.col);
    }
  }

  // Select at new cursor position.
  if (dir == Direction::ACROSS) {
    for (uint8_t i = 0;
         col + i < puzzle->getWidth() && grid[row][col + i] != '\0'; ++i) {
      puzzleWidget->selectPosition(row, col + i);
    }
    for (uint8_t i = 0; col - i >= 0 && grid[row][col - i] != '\0'; --i) {
      puzzleWidget->selectPosition(row, col - i);
    }
  } else {
    for (uint8_t i = 0;
         row + i < puzzle->getHeight() && grid[row + i][col] != '\0'; ++i) {
      puzzleWidget->selectPosition(row, col + i);
    }
    for (uint8_t i = 0; row - i >= 0 && grid[row - i][col] != '\0'; --i) {
      puzzleWidget->selectPosition(row - i, col);
    }
  }

  puzzleWidget->selectCursorPosition(row, col);
  cursor.row = row;
  cursor.col = col;
  cursor.dir = dir;
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
}

void MainWindow::open() {
  auto fileName =
      QFileDialog::getOpenFileName(this, tr("Open Puzzle"), QDir::homePath(),
                                   tr("Across Lite File (*.puz)"));

  if (!fileName.isEmpty()) {
    qDebug() << fileName;
    QFile file{fileName};
    if (!file.open(QIODevice::ReadOnly)) {
      return;
    }
    QByteArray puzFile = file.readAll();
    puzzle.reset(Puzzle::loadFromFile(puzFile));
    reloadPuzzle();
  }
}

void MainWindow::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
}

} // namespace cygnus
