#ifndef PUZZLE_H
#define PUZZLE_H

#include <QByteArray>
#include <QString>
#include <vector>

struct Clue {
  QString clue;
  uint32_t row;
  uint32_t col;
  uint32_t num;
};

template <typename T> using Grid = std::vector<std::vector<T>>;

class Puzzle {
public:
  static Puzzle *loadFromFile(const QByteArray &puzFile);

private:
  Puzzle();
};

#endif
