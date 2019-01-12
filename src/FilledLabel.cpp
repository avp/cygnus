#include "FilledLabel.h"
#include <QDebug>

namespace cygnus {

void FilledLabel::resizeEvent(QResizeEvent *event) {
  QLabel::resizeEvent(event);

  const constexpr char *text = "A";
  const auto &cRect = contentsRect();
  uint32_t fontSize = 1;

  QFont font = this->font();
  while (1) {
    font.setPixelSize(fontSize);
    QRect r = QFontMetrics(font).boundingRect(text);
    if (r.height() < cRect.height() && r.width() < cRect.width()) {
      fontSize++;
    } else {
      font.setPixelSize(fontSize - 1);
      setFont(font);
      return;
    }
  }
}

} // namespace cygnus
