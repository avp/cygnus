#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>

std::unique_ptr<Puzzle> MainWindow::puzzle;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  createActions();
  createMenus();
}

MainWindow::~MainWindow() { delete ui; }

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
  }
}

void MainWindow::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
}
