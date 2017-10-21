#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include <QtWidgets>

namespace cygnus {

class TimerWidget : public QLabel {
  Q_OBJECT
public:
  explicit TimerWidget(QWidget *parent = nullptr);

public slots:
  void setCurrent(uint64_t current);
  void setRunning(bool running);

  void mousePressEvent(QMouseEvent *event);

signals:
  void clicked();

private:
  QLabel *timeLabel_;
};

} // namespace cygnus

#endif
