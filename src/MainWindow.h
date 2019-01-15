#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ClueWidget.h"
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
  explicit MainWindow(const char *filename, QWidget *parent = nullptr);
  void showMaximized();

private slots:
  void open();
  void save();
  void saveAs();

  void revealCurrent();
  void revealClue();
  void revealAll();

  void checkCurrent();
  void checkClue();
  void checkAll();

  void setCursor(uint8_t row, uint8_t col, Direction dir);
  void puzzleClicked(uint8_t row, uint8_t col);
  void puzzleRightClicked();

  void acrossClueClicked(const QListWidgetItem *item);
  void downClueClicked(const QListWidgetItem *item);

  void tickTimer();
  void toggleTimer();

  void loadFile(const QString &fileName);

protected:
  void resizeEvent(QResizeEvent *event) override;

private:
  QString fileName_;
  std::unique_ptr<Puzzle> puzzle_;
  Cursor cursor_;

  void reloadPuzzle();

  QMenu *fileMenu_;
  QAction *openAct_;
  QAction *saveAct_;
  QAction *saveAsAct_;

  QMenu *puzzleMenu_;
  QMenu *revealMenu_;
  QAction *revealCurrentAct_;
  QAction *revealClueAct_;
  QAction *revealAllAct_;
  QMenu *checkMenu_;
  QAction *checkCurrentAct_;
  QAction *checkClueAct_;
  QAction *checkAllAct_;

  QLabel *titleLabel_;
  QLabel *authorLabel_;
  QLabel *copyrightLabel_;
  TimerWidget *timerWidget_;

  ClueWidget *acrossWidget_;
  ClueWidget *downWidget_;

  QWidget *puzzleContainer_;
  QVBoxLayout *puzzleContainerLayout_;
  QLabel *curClueLabel_;
  PuzzleWidget *puzzleWidget_{nullptr};

  void createActions();
  void createMenus();

  std::pair<QWidget *, ClueWidget *> createClueWidget(const QString &title);

  void keyPressEvent(QKeyEvent *event) override;
  void keyUp(bool shift = false);
  void keyDown(bool shift = false);
  void keyLeft(bool shift = false);
  void keyRight(bool shift = false);
  void keyTab(bool reverse);

  void setCell(uint8_t row, uint8_t col, QString letter);
  void clearLetter(uint8_t row, uint8_t col);

  void reveal(uint8_t row, uint8_t col, bool check = true);
  bool check(uint8_t row, uint8_t col);
  bool checkAndMark(uint8_t row, uint8_t col);

  /// Check whole puzzle and show message if completely correct.
  /// Else, do nothing.
  void checkSuccess();

  void setTimerStatus(bool running);
};

} // namespace cygnus

#endif // MAINWINDOW_H
