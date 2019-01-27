#include "MainWindow.h"

#include "Colors.h"
#include "Version.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QtWidgets>

#include <memory>

namespace cygnus {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  this->setWindowTitle(tr("Cygnus Crosswords"));

  auto *vLayout = new QVBoxLayout{};

  auto *infoLayout = new QHBoxLayout{};
  auto *hLayout = new QHBoxLayout{};

  createActions();
  createMenus();

  // Set layout in QWidget
  QWidget *window = new QWidget(this);
  setCentralWidget(window);

  auto res = createClueWidget("ACROSS");
  QWidget *acrossContainer = res.first;
  acrossWidget_ = res.second;

  res = createClueWidget("DOWN");
  QWidget *downContainer = res.first;
  downWidget_ = res.second;

  puzzleContainer_ = new QFrame{};
  puzzleContainer_->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
  puzzleContainerLayout_ = new QVBoxLayout{};

  auto puzzleContainerSize = puzzleContainer_->sizePolicy();
  puzzleContainerSize.setHorizontalStretch(2);
  puzzleContainerSize.setVerticalPolicy(QSizePolicy::Expanding);
  puzzleContainer_->setSizePolicy(puzzleContainerSize);

  curClueLabel_ = new QLabel{};
  curClueLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  puzzleContainerLayout_->addWidget(curClueLabel_);
  puzzleContainerLayout_->addStretch();
  auto clueFont = curClueLabel_->font();
  clueFont.setPointSize(clueFont.pointSize() * 2);
  curClueLabel_->setFont(clueFont);
  curClueLabel_->setStyleSheet("QLabel {"
                               "background: white;"
                               "padding: 2px;"
                               "border: 1px solid black;"
                               "color: black;"
                               "}");

  titleLabel_ = new FilledLabel{};
  timerWidget_ = new TimerWidget{};

  infoLayout->addWidget(titleLabel_, 3);
  infoLayout->addWidget(timerWidget_, 1);

  hLayout->addWidget(acrossContainer);
  hLayout->addWidget(puzzleContainer_);
  hLayout->addWidget(downContainer);

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

  QTimer *timer = new QTimer(this);
  timer->start(1000);
  connect(timer, &QTimer::timeout, this, &MainWindow::tickTimer);
  connect(timerWidget_, &TimerWidget::clicked, this, &MainWindow::toggleTimer);
}

void MainWindow::showMaximized() { QMainWindow::showMaximized(); }

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (!puzzle_) {
    return event->accept();
  }

  if (!isWindowModified()) {
    return event->accept();
  }

  QMessageBox::StandardButton result = QMessageBox::question(
      this, "Save", tr("You have unsaved changes. Save before closing?"),
      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
      QMessageBox::Cancel);

  switch (result) {
  case QMessageBox::Yes:
    save();
  // Fallthrough to closing the program.
  case QMessageBox::No:
    return event->accept();
  case QMessageBox::Cancel:
    return event->ignore();
  default:
    return;
  }
}

void MainWindow::reloadPuzzle() {
  saveAct_->setEnabled(true);
  saveAsAct_->setEnabled(true);

  editMenu_->setEnabled(true);
  viewMenu_->setEnabled(true);
  puzzleMenu_->setEnabled(true);

  undoStack_.clear();

  titleLabel_->setText(QString("<b>%1</b> &nbsp;&nbsp; %2")
                           .arg(puzzle_->getTitle())
                           .arg(puzzle_->getAuthor()));
  timerWidget_->setCurrent(puzzle_->getTimer().current);
  timerWidget_->setRunning(puzzle_->getTimer().running);

  acrossWidget_->clear();
  for (const auto &clue : puzzle_->getClues(Direction::ACROSS)) {
    acrossWidget_->addClue(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }

  downWidget_->clear();
  for (const auto &clue : puzzle_->getClues(Direction::DOWN)) {
    downWidget_->addClue(QString("%1. %2").arg(clue.num).arg(clue.clue));
  }

  if (puzzleWidget_) {
    delete puzzleWidget_;
  }
  puzzleWidget_ = new PuzzleWidget{puzzle_, puzzleContainer_};
  puzzleContainerLayout_->insertWidget(1, puzzleWidget_, 100);
  puzzleContainer_->setLayout(puzzleContainerLayout_);

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

std::pair<QWidget *, ClueWidget *>
MainWindow::createClueWidget(const QString &title) {
  auto *container = new QWidget{this};
  auto *vbox = new QVBoxLayout{};
  container->setLayout(vbox);

  QSizePolicy containerSize(QSizePolicy::MinimumExpanding,
                            QSizePolicy::MinimumExpanding);
  containerSize.setHorizontalStretch(1);
  container->setSizePolicy(containerSize);

  auto *label = new QLabel{};
  label->setText(title);
  label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  label->setAlignment(Qt::AlignCenter);
  label->setMargin(0);
  vbox->addWidget(label);

  auto *clueWidget = new ClueWidget{container};
  clueWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                            QSizePolicy::MinimumExpanding);
  clueWidget->setWordWrap(true);
  clueWidget->setFocusPolicy(Qt::NoFocus);
  vbox->addWidget(clueWidget);

  return {container, clueWidget};
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

  undoAct_ = new QAction(tr("&Undo"), this);
  undoAct_->setShortcuts(QKeySequence::Undo);
  undoAct_->setStatusTip(tr("Undo the last action"));
  connect(undoAct_, &QAction::triggered, this, &MainWindow::undo);

  redoAct_ = new QAction(tr("&Redo"), this);
  redoAct_->setShortcuts(QKeySequence::Redo);
  redoAct_->setStatusTip(tr("Redo the last undone action"));
  connect(redoAct_, &QAction::triggered, this, &MainWindow::redo);

  increaseSizeAct_ = new QAction(tr("&Increase clue size"), this);
  increaseSizeAct_->setShortcuts(QKeySequence::ZoomIn);
  connect(increaseSizeAct_, &QAction::triggered, this,
          &MainWindow::increaseSize);

  decreaseSizeAct_ = new QAction(tr("&Decrease clue size"), this);
  decreaseSizeAct_->setShortcuts(QKeySequence::ZoomOut);
  connect(decreaseSizeAct_, &QAction::triggered, this,
          &MainWindow::decreaseSize);

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

  insertMultipleAct_ = new QAction(tr("Insert multiple letters..."), this);
  insertMultipleAct_->setShortcuts(QKeySequence::InsertLineSeparator);
  connect(insertMultipleAct_, &QAction::triggered, this,
          &MainWindow::insertMultiple);

  aboutAct_ = new QAction(tr("&About"), this);
  connect(aboutAct_, &QAction::triggered, this, &MainWindow::about);

  shortcutsAct_ = new QAction(tr("View &Shortcuts"), this);
  connect(shortcutsAct_, &QAction::triggered, this, &MainWindow::shortcuts);
}

void MainWindow::loadFile() {
  qDebug() << "Opening file:" << fileName_;
  QFile file{fileName_};
  if (!file.open(QIODevice::ReadOnly)) {
    fileName_ = nullptr;
    return;
  }
  QByteArray puzFile = file.readAll();
  puzzle_ = std::move(Puzzle::loadFromFile(puzFile));
  if (puzzle_) {
    QFileInfo info{fileName_};
    this->setWindowTitle(
        QString("[*]%1 - Cygnus Crosswords").arg(info.fileName()));
    reloadPuzzle();
  } else {
    QMessageBox::warning(
        this, QString("Corrupted File"),
        QString("The file %1 isn't a valid puzzle file.").arg(fileName_));
  }
}

void MainWindow::open() {
  const auto fileName =
      QFileDialog::getOpenFileName(this, tr("Open puzzle"), QDir::homePath(),
                                   tr("Across Lite File (*.puz)"));

  if (!fileName.isEmpty()) {
    fileName_ = fileName;
    loadFile();
  }
}

void MainWindow::save() {
  QFile file{fileName_};
  qDebug() << "Saving to:" << file.fileName();
  if (file.open(QIODevice::WriteOnly)) {
    QByteArray bytes = puzzle_->serialize();
    file.write(bytes);
    file.close();
    setWindowModified(false);
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

void MainWindow::reveal(uint8_t row, uint8_t col, bool check) {
  char current = puzzle_->getGrid()[row][col];
  if (current == BLACK) {
    return;
  }
  char solution = puzzle_->getSolution()[row][col];
  if (current == EMPTY || current != solution) {
    setCell(row, col, QString("%1").arg(solution));
    puzzle_->getMarkup()[row][col] |= Puzzle::RevealedTag;
    puzzleWidget_->setMarkup(row, col, puzzle_->getMarkup()[row][col]);
  }
  if (check) {
    checkSuccess();
  }
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
      reveal(r, c, false);
    }
  }
  checkSuccess();
}

bool MainWindow::checkAndMark(uint8_t row, uint8_t col) {
  if (puzzle_->check(row, col)) {
    return true;
  }
  puzzle_->getMarkup()[row][col] |= Puzzle::IncorrectTag;
  puzzleWidget_->setMarkup(row, col, puzzle_->getMarkup()[row][col]);
  return false;
}

void MainWindow::undo() {
  if (undoStack_.empty()) {
    return;
  }
  const auto &entry = undoStack_.back();
  uint32_t row = entry.row;
  uint32_t col = entry.col;

  redoStack_.emplace_back(
      UndoEntry{row, col, puzzle_->getMarkup()[row][col],
                puzzle_->getRebusFill()[row][col].isEmpty()
                    ? QString(QChar(puzzle_->getGrid()[row][col]))
                    : puzzle_->getRebusFill()[row][col]});

  setWindowModified(true);
  puzzle_->getGrid()[row][col] = entry.text.at(0).toLatin1();
  puzzle_->getRebusFill()[row][col] = entry.text;
  puzzleWidget_->setCell(row, col, entry.text);
  setCursor(row, col, cursor_.dir);

  puzzle_->getMarkup()[row][col] = entry.markup;
  puzzleWidget_->setMarkup(row, col, entry.markup);

  undoStack_.pop_back();

  undoAct_->setEnabled(!undoStack_.empty());
  redoAct_->setEnabled(!redoStack_.empty());
}

void MainWindow::redo() {
  if (redoStack_.empty()) {
    return;
  }
  const auto &entry = redoStack_.back();
  uint32_t row = entry.row;
  uint32_t col = entry.col;

  undoStack_.emplace_back(
      UndoEntry{row, col, puzzle_->getMarkup()[row][col],
                puzzle_->getRebusFill()[row][col].isEmpty()
                    ? QString(QChar(puzzle_->getGrid()[row][col]))
                    : puzzle_->getRebusFill()[row][col]});

  setWindowModified(true);
  puzzle_->getGrid()[row][col] = entry.text.at(0).toLatin1();
  puzzle_->getRebusFill()[row][col] = entry.text;
  puzzleWidget_->setCell(row, col, entry.text);

  puzzle_->getMarkup()[row][col] = entry.markup;
  puzzleWidget_->setMarkup(row, col, entry.markup);

  redoStack_.pop_back();

  undoAct_->setEnabled(!undoStack_.empty());
  redoAct_->setEnabled(!redoStack_.empty());
}

void MainWindow::increaseSize() {
  if (puzzle_) {
    acrossWidget_->modifySize(1);
    downWidget_->modifySize(1);
  }
}

void MainWindow::decreaseSize() {
  if (puzzle_) {
    acrossWidget_->modifySize(-1);
    downWidget_->modifySize(-1);
  }
}

void MainWindow::checkSuccess() {
  // See if the puzzle's complete.
  if (!puzzle_->allCorrect()) {
    return;
  }

  // Puzzle is complete.
  setTimerStatus(false);
  QMessageBox::information(this, "Congratulations!",
                           "You completed the puzzle correctly.");
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

void MainWindow::insertMultiple() {
  if (puzzle_) {
    QString rebusInput = QInputDialog::getText(
        this, tr("Enter multiple letters:"), tr("Letters"));
    if (!rebusInput.isEmpty()) {
      setCell(cursor_.row, cursor_.col, rebusInput.toUpper());
      checkSuccess();
    }
  }
}

void MainWindow::about() {
  static QString title(QStringLiteral("About"));
  static QString text(QStringLiteral(
      "<p>Cygnus Crosswords " VER_PRODUCTVERSION_STR "</p>"
      "<p><a href=\"https://github.com/avp/cygnus\">GitHub</a></p>"));

  QWidget *parent = this;

#ifdef Q_OS_MAC
  static QPointer<QMessageBox> oldMsgBox;
  if (oldMsgBox && oldMsgBox->text() == text) {
    oldMsgBox->show();
    oldMsgBox->raise();
    oldMsgBox->activateWindow();
    return;
  }
#endif
  QMessageBox *box =
      new QMessageBox(QMessageBox::Information, title, text, QMessageBox::Ok,
                      parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
  box->setAttribute(Qt::WA_DeleteOnClose);
  box->setTextFormat(Qt::RichText);
  QIcon icon = box->windowIcon();
  QSize size = icon.actualSize(QSize(64, 64));
  box->setIconPixmap(icon.pixmap(size));
// should perhaps be a style hint
#ifdef Q_OS_MAC
  oldMsgBox = box;
  box->show();
#else
  box->exec();
#endif
}

void MainWindow::shortcuts() {
  QDesktopServices::openUrl(
      QStringLiteral("https://github.com/avp/cygnus/wiki/Keyboard-Shortcuts"));
}

void MainWindow::createMenus() {
  fileMenu_ = menuBar()->addMenu(tr("&File"));
  fileMenu_->addAction(openAct_);
  fileMenu_->addAction(saveAct_);
  saveAct_->setEnabled(false);
  fileMenu_->addAction(saveAsAct_);
  saveAsAct_->setEnabled(false);

  editMenu_ = menuBar()->addMenu(tr("&Edit"));
  editMenu_->addAction(undoAct_);
  undoAct_->setEnabled(false);
  editMenu_->addAction(redoAct_);
  redoAct_->setEnabled(false);
  editMenu_->setEnabled(false);

  viewMenu_ = menuBar()->addMenu(tr("&View"));
  viewMenu_->addAction(increaseSizeAct_);
  viewMenu_->addAction(decreaseSizeAct_);
  viewMenu_->setEnabled(false);

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
  puzzleMenu_->addSeparator();
  puzzleMenu_->addAction(insertMultipleAct_);

  helpMenu_ = menuBar()->addMenu(tr("&Help"));
  helpMenu_->addAction(aboutAct_);
  helpMenu_->addAction(shortcutsAct_);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_Up:
    keyUp(event->modifiers() & Qt::ShiftModifier);
    break;
  case Qt::Key_Down:
    keyDown(event->modifiers() & Qt::ShiftModifier);
    break;
  case Qt::Key_Left:
    keyLeft(event->modifiers() & Qt::ShiftModifier);
    break;
  case Qt::Key_Right:
    keyRight(event->modifiers() & Qt::ShiftModifier);
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
  case Qt::Key_Backspace: {
    clearLetter(cursor_.row, cursor_.col);
    const Clue &clue = puzzle_->getClueByNum(
        cursor_.dir,
        puzzle_->getNumByPosition(cursor_.row, cursor_.col, cursor_.dir));
    if (cursor_.dir == Direction::ACROSS) {
      if (cursor_.col != clue.col) {
        keyLeft();
      }
    } else {
      if (cursor_.row != clue.row) {
        keyUp();
      }
    }
    break;
  }
  case Qt::Key_Delete:
    clearLetter(cursor_.row, cursor_.col);
    break;
  case Qt::Key_Insert:
    insertMultiple();
    break;

  case Qt::Key_Home:
    if (puzzle_) {
      const Clue &clue = puzzle_->getClueByNum(
          cursor_.dir,
          puzzle_->getNumByPosition(cursor_.row, cursor_.col, cursor_.dir));
      setCursor(clue.row, clue.col, clue.dir);
    }
    break;
  case Qt::Key_End:
    if (puzzle_) {
      const Clue &clue = puzzle_->getClueByNum(
          cursor_.dir,
          puzzle_->getNumByPosition(cursor_.row, cursor_.col, cursor_.dir));
      auto end = puzzle_->getClueEnd(clue);
      setCursor(end.first, end.second, clue.dir);
    }
    break;
  }

#ifdef Q_OS_MACOS
  // Use the "Meta" modifier on OSX because we don't want the Command key.
  if (event->modifiers() & Qt::MetaModifier) {
#else
  if (event->modifiers() & Qt::ControlModifier) {
#endif
    switch (event->key()) {
    case Qt::Key_J:
      keyDown(event->modifiers() & Qt::ShiftModifier);
      break;
    case Qt::Key_K:
      keyUp(event->modifiers() & Qt::ShiftModifier);
      break;
    case Qt::Key_H:
      keyLeft(event->modifiers() & Qt::ShiftModifier);
      break;
    case Qt::Key_L:
      keyRight(event->modifiers() & Qt::ShiftModifier);
      break;
    }
  }

  if (puzzle_) {
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
          if (cursor_.col + 1 < puzzle_->getWidth() &&
              puzzle_->getGrid()[cursor_.row][cursor_.col + 1] != BLACK) {
            keyRight();
          }
        } else {
          if (cursor_.row + 1 < puzzle_->getHeight() &&
              puzzle_->getGrid()[cursor_.row + 1][cursor_.col] != BLACK) {
            keyDown();
          }
        }
      }
    }
  }
}

void MainWindow::keyUp(bool shift) {
  const auto &grid = puzzle_->getGrid();
  if (!shift && cursor_.dir == Direction::ACROSS) {
    setCursor(cursor_.row, cursor_.col, Direction::DOWN);
    if (grid[cursor_.row][cursor_.col] != EMPTY) {
      keyUp();
    }
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
  setCursor(row, cursor_.col, shift ? cursor_.dir : Direction::DOWN);
}

void MainWindow::keyDown(bool shift) {
  const auto &grid = puzzle_->getGrid();
  if (!shift && cursor_.dir == Direction::ACROSS) {
    setCursor(cursor_.row, cursor_.col, Direction::DOWN);
    if (grid[cursor_.row][cursor_.col] != EMPTY) {
      keyDown();
    }
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
  setCursor(row, cursor_.col, shift ? cursor_.dir : Direction::DOWN);
}

void MainWindow::keyLeft(bool shift) {
  const auto &grid = puzzle_->getGrid();
  if (!shift && cursor_.dir == Direction::DOWN) {
    setCursor(cursor_.row, cursor_.col, Direction::ACROSS);
    if (grid[cursor_.row][cursor_.col] != EMPTY) {
      keyLeft();
    }
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
  setCursor(cursor_.row, col, shift ? cursor_.dir : Direction::ACROSS);
}

void MainWindow::keyRight(bool shift) {
  const auto &grid = puzzle_->getGrid();
  if (!shift && cursor_.dir == Direction::DOWN) {
    setCursor(cursor_.row, cursor_.col, Direction::ACROSS);
    if (grid[cursor_.row][cursor_.col] != EMPTY) {
      keyRight();
    }
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
  setCursor(cursor_.row, col, shift ? cursor_.dir : Direction::ACROSS);
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
  auto newPos = puzzle_->getFirstBlank(newClue);
  setCursor(newPos.first, newPos.second, dir);
}

void MainWindow::setCell(uint8_t row, uint8_t col, QString text) {
  if (puzzle_->getMarkup()[row][col] & Puzzle::RevealedTag) {
    // If the letter was revealed, don't allow editing it.
    return;
  }

  undoStack_.emplace_back(
      UndoEntry{row, col, puzzle_->getMarkup()[row][col],
                puzzle_->getRebusFill()[row][col].isEmpty()
                    ? QString(QChar(puzzle_->getGrid()[row][col]))
                    : puzzle_->getRebusFill()[row][col]});
  redoStack_.clear();

  setWindowModified(true);
  puzzle_->getGrid()[row][col] = text.at(0).toLatin1();
  puzzle_->getRebusFill()[row][col] = text;
  puzzleWidget_->setCell(row, col, text);
  Puzzle::Markup &markup = puzzle_->getMarkup()[row][col];
  if (markup & Puzzle::IncorrectTag) {
    markup &= ~Puzzle::IncorrectTag;
    markup |= Puzzle::PreviousIncorrectTag;
  }
  puzzleWidget_->setMarkup(row, col, markup);

  undoAct_->setEnabled(!undoStack_.empty());
  redoAct_->setEnabled(!redoStack_.empty());
}

void MainWindow::clearLetter(uint8_t row, uint8_t col) {
  setCell(row, col, QString(EMPTY));
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
