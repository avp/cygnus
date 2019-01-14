#include "FilledLabel.h"
#include <QDebug>

namespace cygnus {

void FilledLabel::resizeEvent(QResizeEvent *event) {
  QLabel::resizeEvent(event);

  const auto &cRect = contentsRect();
  uint32_t fontSize = 1;

  QFont font = this->font();
  while (1) {
    font.setPixelSize(fontSize);
    QRect r = QFontMetrics(font).boundingRect(text().isEmpty() ? "A" : text());
    if (r.height() < cRect.height() && r.width() < cRect.width()) {
      fontSize++;
    } else {
      uint32_t newSize = std::max(
          text().size() > 1 ? fontSize / 2 : fontSize - 1, (uint32_t)1);
      font.setPixelSize(newSize);
      setFont(font);
      return;
    }
  }
}

} // namespace cygnus
