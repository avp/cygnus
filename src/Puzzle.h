#ifndef PUZZLE_H
#define PUZZLE_H

#include <QByteArray>
#include <QString>

struct Clue {
  QString clue;
  uint32_t row;
  uint32_t col;
  uint32_t num;
};

class Puzzle {
public:
  static Puzzle *loadFromFile(const QByteArray &puzFile);

private:
  Puzzle();
};

#endif
