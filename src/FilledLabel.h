#ifndef FILLEDLABEL_H
#define FILLEDLABEL_H

#include <QLabel>
#include <QWidget>

namespace cygnus {

class FilledLabel : public QLabel {
  Q_OBJECT
public:
  explicit FilledLabel(QWidget *parent = nullptr) : QLabel(parent) {}

public slots:
  void setText(const QString &text) {
    QLabel::setText(text);
    resizeText();
  }

protected:
  void resizeEvent(QResizeEvent *event) override {
    QLabel::resizeEvent(event);
    resizeText();
  }

private:
  void resizeText();
};

} // namespace cygnus

#endif // FILLEDLABEL_H
