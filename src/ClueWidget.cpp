#include "ClueWidget.h"

namespace cygnus {

ClueWidget::ClueWidget(QWidget *parent) : QListWidget(parent) {
  setMinimumWidth(200);
}

void ClueWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mousePressed_ = true;
  }
  QListWidget::mousePressEvent(event);
}

void ClueWidget::mouseMoveEvent(QMouseEvent *event) {
  if (!mousePressed_) {
    // Disable click and drag.
    QListWidget::mouseMoveEvent(event);
  }
}

void ClueWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    mousePressed_ = false;
  }
  QListWidget::mouseReleaseEvent(event);
}

} // namespace cygnus
