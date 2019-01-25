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
  font.setPointSize(std::max(1.0, font.pointSizeF() * scale));
  setFont(font);
}

} // namespace cygnus
