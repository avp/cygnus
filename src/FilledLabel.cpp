#include "FilledLabel.h"
#include <QDebug>

namespace cygnus {

void FilledLabel::resizeEvent(QResizeEvent *event) {
  QLabel::resizeEvent(event);

  const constexpr char *text = "A";
  const auto &cRect = contentsRect();
  uint32_t fontSize = 1;

  QFont font = this->font();
  font.setPixelSize(fontSize);
  QRect r = QFontMetrics(font).boundingRect(text);
  uint32_t factor = cRect.height() / r.height();
  font.setPixelSize(factor + 2);
  setFont(font);
  return;
}

} // namespace cygnus
