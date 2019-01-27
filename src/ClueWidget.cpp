#include "ClueWidget.h"

#include "Colors.h"
#include "Settings.h"

#include <QDebug>

namespace cygnus {

ClueWidget::ClueWidget(QWidget *parent) : QListWidget(parent) {
  setMinimumWidth(200);
  QSettings settings;
  pointSize_ = settings.value(Settings::clueSize).toInt();
}

void ClueWidget::modifySize(int delta) {
  qDebug() << "Changing size by" << delta;
  pointSize_ += delta;
  for (uint32_t i = 0, e = count(); i < e; ++i) {
    QFont font = item(i)->font();
    font.setPointSize(pointSize_);
    item(i)->setFont(font);
  }
  QSettings settings;
  settings.setValue(Settings::clueSize, pointSize_);
}

void ClueWidget::addClue(const QString &text) {
  this->addItem(text);
  if (pointSize_ == 0) {
    pointSize_ = item(count() - 1)->font().pointSize();
    QSettings settings;
    settings.setValue(Settings::clueSize, pointSize_);
  } else {
    QFont font = item(count() - 1)->font();
    font.setPointSize(pointSize_);
    item(count() - 1)->setFont(font);
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
      QString("QListView::item:selected { background: %1; color: black }")
          .arg(Colors::colorToString(Colors::PRIMARY_HIGHLIGHT)));
}

void ClueWidget::setSecondary() {
  setStyleSheet(
      QString("QListView::item:selected { background: %1; color: black }")
          .arg(Colors::colorToString(Colors::SECONDARY_HIGHLIGHT)));
}

} // namespace cygnus
