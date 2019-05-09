#include "FilledLabel.h"
#include <QDebug>

namespace cygnus {

void FilledLabel::resizeText() {
  QFont font = this->font();
  QRect r = QFontMetrics(font).boundingRect(text().isEmpty() ? "A" : text());

  const auto &cRect = contentsRect();

  qreal scaleW = (qreal)cRect.width() / (qreal)r.width();
  qreal scaleH = (qreal)cRect.height() / (qreal)r.height();

  qreal scale = std::min(scaleW, scaleH);
  auto newSize = std::max(1.0, font.pointSizeF() * scale) * 0.9;
  if (newSize > 0) {
    font.setPointSizeF(newSize);
    setFont(font);
  }
}

} // namespace cygnus
