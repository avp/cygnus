#ifndef FILLEDLABEL_H
#define FILLEDLABEL_H

#include <QLabel>
#include <QWidget>

namespace cygnus {

class FilledLabel : public QLabel {
  Q_OBJECT
public:
  explicit FilledLabel(QWidget *parent = nullptr) : QLabel(parent) {}

protected:
  void resizeEvent(QResizeEvent *event) override;
};

} // namespace cygnus

#endif // FILLEDLABEL_H
