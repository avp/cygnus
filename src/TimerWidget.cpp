#include "TimerWidget.h"

namespace cygnus {

TimerWidget::TimerWidget(QWidget *parent) : QLabel(parent) {
  setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

void TimerWidget::mousePressEvent(QMouseEvent *event) { clicked(); }

void TimerWidget::setCurrent(uint64_t current) {
  uint64_t minutes = current / 60ull;
  uint64_t seconds = current % 60ull;
  setText(QString("%1:%2")
              .arg(minutes, 2, 10, QChar('0'))
              .arg(seconds, 2, 10, QChar('0')));
}

void TimerWidget::setRunning(bool running) {
  if (running) {
    setStyleSheet("QLabel { background-color : black; color : #00ff00; }");
  } else {
    setStyleSheet("QLabel { background-color : black; color : #ff0000; }");
  }
}

} // namespace cygnus
