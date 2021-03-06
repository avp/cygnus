#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ClueWidget.h"
#include "FilledLabel.h"
#include "Puzzle.h"
#include "PuzzleWidget.h"
#include "TimerWidget.h"

#include <QtWidgets>

namespace cygnus {

struct Cursor {
  uint8_t row;
  uint8_t col;
  Direction dir;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  void showMaximized();

  void setFileName(QString fileName) { fileName_ = fileName; }

  /// Load the file at the internally set fileName_;
  void loadFile();

  /// \return true if the window is displaying a puzzle.
  bool isLoaded() const { return puzzle_ != nullptr; }

public slots:
  /// Show open file dialog.
  void open();

private slots:
  void save();
  void saveAs();

  void revealCurrent();
  void revealClue();
  void revealAll();

  void checkCurrent();
  void checkClue();
  void checkAll();

  void insertMultiple();

  void showNote();
  void about();
  void shortcuts();

  void increaseSize();
  void decreaseSize();
  void toggleDarkMode();

  /// Set the cursor to (row, col) and point the active squares in the direction
  /// of \param dir.
  /// Flips the direction if the row and column don't represent a valid clue in
  /// the given dir, given that every white square must represent a valid clue
  /// in one of the two directions.
  void setCursor(uint8_t row, uint8_t col, Direction dir);

  void puzzleClicked(uint8_t row, uint8_t col);
  void puzzleRightClicked();

  void acrossClueClicked(const QListWidgetItem *item);
  void downClueClicked(const QListWidgetItem *item);

  void tickTimer();
  void toggleTimer();

protected:
  void resizeEvent(QResizeEvent *event) override;

  void closeEvent(QCloseEvent *event) override;

private:
  QString fileName_;
  std::unique_ptr<Puzzle> puzzle_;
  Cursor cursor_;

  struct UndoEntry {
    uint32_t row;
    uint32_t col;
    Puzzle::Markup markup;
    QString text;
    bool pencil;
  };
  std::vector<UndoEntry> undoStack_{};
  std::vector<UndoEntry> redoStack_{};

  void reloadPuzzle();

  QMenu *fileMenu_;
  QAction *openAct_;
  QAction *saveAct_;
  QAction *saveAsAct_;

  QMenu *editMenu_;
  QAction *undoAct_;
  QAction *redoAct_;
  QMenu *revealMenu_;
  QAction *revealCurrentAct_;
  QAction *revealClueAct_;
  QAction *revealAllAct_;
  QMenu *checkMenu_;
  QAction *checkCurrentAct_;
  QAction *checkClueAct_;
  QAction *checkAllAct_;
  QAction *insertMultipleAct_;

  QMenu *viewMenu_;
  QAction *increaseSizeAct_;
  QAction *decreaseSizeAct_;
  QAction *toggleDarkModeAct_;

  QMenu *helpMenu_;
  QAction *aboutAct_;
  QAction *shortcutsAct_;

  QWidget *centralWidget_;

  QPushButton *noteButton_;
  FilledLabel *titleLabel_;
  TimerWidget *timerWidget_;

  QWidget *acrossContainer_;
  QWidget *downContainer_;

  ClueWidget *acrossWidget_;
  ClueWidget *downWidget_;

  QSplitter *puzzleContainer_;
  QVBoxLayout *puzzleContainerLayout_;
  QScrollArea *curClueScroll_;
  FilledLabel *curClueLabel_;
  PuzzleWidget *puzzleWidget_{nullptr};

  QPalette lightPalette_;
  QPalette darkPalette_;

  void createActions();
  void createMenus();

  std::pair<QWidget *, ClueWidget *> createClueWidget(const QString &title);

  void keyPressEvent(QKeyEvent *event) override;
  void keyUp(bool shift = false);
  void keyDown(bool shift = false);
  void keyLeft(bool shift = false);
  void keyRight(bool shift = false);
  void keyTab(bool reverse);

  void setCell(uint8_t row, uint8_t col, QString text, bool pencil);
  void clearLetter(uint8_t row, uint8_t col);

  void reveal(uint8_t row, uint8_t col, bool check = true);
  bool check(uint8_t row, uint8_t col);
  bool checkAndMark(uint8_t row, uint8_t col);

  void undo();
  void redo();

  /// Check whole puzzle and show message if completely correct.
  /// Else, do nothing.
  void checkSuccess();

  void setTimerStatus(bool running);

  /// Prevent tab key from messing with focus in the main window.
  /// It should simply move through crossword entries.
  bool focusNextPrevChild(bool next) override { return false; }
};

} // namespace cygnus

#endif // MAINWINDOW_H
