#ifndef PUZZLE_H
#define PUZZLE_H

#include <QByteArray>
#include <QString>
#include <utility>
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
public:
  struct CellData {
    uint32_t acrossNum{0};
    uint32_t acrossIdx{0};
    bool acrossStart{false};

    uint32_t downNum{0};
    uint32_t downIdx{0};
    bool downStart{false};
  };

private:
  Puzzle(QByteArray version, uint8_t height, uint8_t width, uint16_t mask1,
         uint16_t mask2, std::vector<Clue> clues[2], Grid<char> solution,
         Grid<char> grid, Grid<CellData> data, QByteArray text);

  QByteArray version_;

  uint8_t height_;
  uint8_t width_;
  uint16_t mask1_;
  uint16_t mask2_;
  std::vector<Clue> clues_[2];
  Grid<char> solution_;
  Grid<char> grid_;
  Grid<CellData> data_;
  QByteArray text_;

public:
  static Puzzle *loadFromFile(const QByteArray &puzFile);

  inline const uint8_t getHeight() const { return height_; }
  inline const uint8_t getWidth() const { return width_; }
  inline const std::vector<Clue> &getClues(Direction dir) const {
    return clues_[static_cast<int>(dir)];
  }
  inline Grid<char> &getGrid() { return grid_; }
  inline const Grid<char> &getSolution() const { return solution_; }
  inline const Grid<CellData> &getCellData() const { return data_; }
  inline const uint16_t getNumClues() const {
    return clues_[0].size() + clues_[1].size();
  }

  const int getClueByNum(Direction dir, uint32_t num) const;
  const uint32_t getNumByPosition(uint8_t row, uint8_t col,
                                  Direction dir) const;

  /// Given the \p idx clue in \p dir, return the {row,col} of the clue.
  const std::pair<uint8_t, uint8_t> getPositionFromClue(Direction dir,
                                                        uint32_t idx) const;

  QByteArray serialize() const;
};

} // namespace cygnus

#endif
