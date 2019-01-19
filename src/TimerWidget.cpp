#include "TimerWidget.h"

namespace cygnus {

TimerWidget::TimerWidget(QWidget *parent) : QLabel(parent) {
  setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

  auto f = font();
  f.setPointSize(12);
  setFont(f);
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
    setStyleSheet(
        "QLabel { background: black; color : #00ff00; padding: 1px }");
  } else {
    setStyleSheet(
        "QLabel { background: black; color : #ff0000; padding: 1px }");
  }
}

} // namespace cygnus
