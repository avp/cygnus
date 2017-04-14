#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QtWidgets>

/// Currently loaded puzzle.
std::unique_ptr<Puzzle> MainWindow::puzzle;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  QHBoxLayout *layout = new QHBoxLayout{};

  createActions();
  createMenus();

  // Set layout in QWidget
  QWidget *window = new QWidget();
  setCentralWidget(window);

  QSizePolicy puzzleSize{QSizePolicy::Preferred, QSizePolicy::Preferred};
  puzzleSize.setHorizontalStretch(2);

  acrossWidget = createClueWidget();
  downWidget = createClueWidget();

  puzzleWidget = new QWidget{};
  puzzleWidget->setSizePolicy(puzzleSize);

  // Set QWidget as the central layout of the main window
  layout->addWidget(acrossWidget);
  layout->addWidget(puzzleWidget);
  layout->addWidget(downWidget);
  window->setLayout(layout);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::reloadPuzzle() {
  acrossWidget->clear();
  for (const auto &clue : puzzle->getAcross()) {
    acrossWidget->addItem(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }
  downWidget->clear();
  for (const auto &clue : puzzle->getDown()) {
    downWidget->addItem(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }
}

QListWidget *MainWindow::createClueWidget() {
  auto result = new QListWidget{};
  QSizePolicy cluesSize{QSizePolicy::Preferred, QSizePolicy::Preferred};
  cluesSize.setHorizontalStretch(1);
  result->setFrameStyle(QFrame::NoFrame);
  result->setSizePolicy(cluesSize);
  result->setWordWrap(true);
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
