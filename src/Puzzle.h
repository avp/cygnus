#ifndef PUZZLE_H
#define PUZZLE_H

#include <QByteArray>
#include <QString>
#include <vector>

namespace cygnus {

const char BLACK = '.';
const char EMPTY = '-';

enum class Direction {
  ACROSS,
  DOWN,
};

inline Direction flip(Direction dir) {
  return (dir == Direction::ACROSS) ? Direction::DOWN : Direction::ACROSS;
}

struct Clue {
  QString clue;
  uint32_t row;
  uint32_t col;
  uint32_t num;
};

template <typename T> using Grid = std::vector<std::vector<T>>;

class Puzzle {
private:
  Puzzle(uint8_t height, uint8_t width, std::vector<Clue> clues[2],
         Grid<char> solution, Grid<char> grid, Grid<uint32_t> nums);

  uint8_t height_;
  uint8_t width_;
  std::vector<Clue> clues_[2];
  Grid<char> solution_;
  Grid<char> grid_;
  Grid<uint32_t> nums_;

public:
  static Puzzle *loadFromFile(const QByteArray &puzFile);

  inline const uint8_t getHeight() const { return height_; }
  inline const uint8_t getWidth() const { return width_; }
  inline const std::vector<Clue> &getClues(Direction dir) const {
    return clues_[static_cast<int>(dir)];
  }
  inline Grid<char> &getGrid() { return grid_; }
  inline const Grid<char> &getSolution() const { return solution_; }
  inline const Grid<uint32_t> &getNums() const { return nums_; }

  const int getClueByNum(Direction dir, uint32_t num) const;
  const uint32_t getNumByPosition(uint8_t row, uint8_t col,
                                  Direction dir) const;
};

} // namespace cygnus

#endif
