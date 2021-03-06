#ifndef COLORS_H
#define COLORS_H

#include <QColor>

namespace cygnus {
namespace Colors {

inline QString colorToString(const QColor &color) {
  return QString("rgb(%1, %2, %3)")
      .arg(color.red())
      .arg(color.green())
      .arg(color.blue());
}

const QColor PRIMARY_HIGHLIGHT{120, 210, 250};
const QColor PRIMARY_HIGHLIGHT_DARK{80, 150, 200};

const QColor SECONDARY_HIGHLIGHT{220, 220, 235};

const QColor CIRCLE{100, 100, 100};

const QColor PENCIL{128, 128, 128};
const QColor PENCIL_DARK{170, 170, 170};

} // namespace Colors
} // namespace cygnus

#endif
