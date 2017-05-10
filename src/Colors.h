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

const QColor SECONDARY_HIGHLIGHT{220, 220, 235};

const QColor GRAY{150, 150, 150};

} // namespace Colors
} // namespace cygnus

#endif
