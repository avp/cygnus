#include "ClueWidget.h"

#include "Colors.h"

#include <QDebug>

namespace cygnus {

static constexpr int kPixelDelta = 5;

ClueWidget::ClueWidget(QWidget *parent) : QListWidget(parent) {
  setMinimumWidth(200);
}

void ClueWidget::modifySize(int delta) {
  qDebug() << "Changing size by" << delta;
  for (uint32_t i = 0, e = count(); i < e; ++i) {
    QFont font = item(i)->font();
    font.setPointSize(font.pointSize() + delta);
    item(i)->setFont(font);
  }
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

void ClueWidget::setPrimary() {
  setStyleSheet(
      QString("QListView::item:selected { background: %1; foreground: black }")
          .arg(Colors::colorToString(Colors::PRIMARY_HIGHLIGHT)));
}

void ClueWidget::setSecondary() {
  setStyleSheet(
      QString("QListView::item:selected { background: %1; foreground: black }")
          .arg(Colors::colorToString(Colors::SECONDARY_HIGHLIGHT)));
}

} // namespace cygnus
