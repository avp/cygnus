#ifndef CLUEWIDGET_H
#define CLUEWIDGET_H

#include <QtWidgets>

namespace cygnus {

class ClueWidget : public QListWidget {
  Q_OBJECT
public:
  explicit ClueWidget(QWidget *parent = nullptr);

  void modifySize(int delta);

public slots:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

  void setPrimary();
  void setSecondary();

private:
  bool mousePressed_{false};
};

} // namespace cygnus

#endif
