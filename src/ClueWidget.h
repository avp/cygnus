#ifndef CLUEWIDGET_H
#define CLUEWIDGET_H

#include <QtWidgets>

namespace cygnus {

class ClueWidget : public QListWidget {
  Q_OBJECT
public:
  explicit ClueWidget(QWidget *parent = nullptr);

  void modifySize(int delta);

  /// Add a clue to the end of the widget.
  /// Updates the size of the label to be consistent with the other labels.
  void addClue(const QString &text);

public slots:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

  /// Set the list to be the home of the primary clue (i.e. the one the user is
  /// currently working on).
  void setPrimary();

  /// Set the list to be the home of the secondary clue (i.e. the one in the
  /// other direction of the one the user is currently working on).
  void setSecondary();

private:
  /// Whether the mouse is currently being held down.
  /// Used to disable drag and drop reordering in the list widget.
  bool mousePressed_{false};

  /// If 0, a pointSize_ has not been set yet.
  /// If positive, the pointSize of the font for every clue.
  int pointSize_{0};
};

} // namespace cygnus

#endif
