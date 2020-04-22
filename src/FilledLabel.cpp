#include "FilledLabel.h"

#include <QDebug>
#include <ctgmath>

namespace cygnus {

void FilledLabel::resizeText() {
  QFont font = this->font();
  QRect r = QFontMetrics(font).boundingRect(text().isEmpty() ? "A" : text());

  const auto &cRect = contentsRect();

  qreal scaleW = ((qreal)cRect.width() - 2) / (qreal)r.width();
  qreal scaleH = ((qreal)cRect.height() - 2) / (qreal)r.height();

  qreal scale = std::min(scaleW, scaleH);
  auto newSize = std::round(std::max(1.0, font.pointSizeF() * scale) * 0.9);
  if (newSize > 0) {
    font.setPointSizeF(newSize);
    setFont(font);
  }
}

} // namespace cygnus
