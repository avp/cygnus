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
private:
  Puzzle(uint8_t height, uint8_t width, std::vector<Clue> across,
         std::vector<Clue> down, Grid<char> solution, Grid<char> grid,
         Grid<uint32_t> nums);

  uint8_t height_;
  uint8_t width_;
  std::vector<Clue> across_;
  std::vector<Clue> down_;
  Grid<char> solution_;
  Grid<char> grid_;
  Grid<uint32_t> nums_;

public:
  static Puzzle *loadFromFile(const QByteArray &puzFile);

  inline const uint8_t getHeight() const { return height_; }
  inline const uint8_t getWidth() const { return width_; }
  inline const std::vector<Clue> &getAcross() const { return across_; }
  inline const std::vector<Clue> &getDown() const { return down_; }
  inline Grid<char> &getGrid() { return grid_; }
  inline const Grid<char> &getSolution() const { return solution_; }
  inline const Grid<uint32_t> &getNums() const { return nums_; }
};

#endif
