#ifndef PUZZLE_H
#define PUZZLE_H

#include <QByteArray>
#include <QDebug>
#include <QString>

#include <memory>
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
  Direction dir;
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

  enum class PuzzleType : uint16_t {
    NORMAL = 0x0001,
    DIAGRAMLESS = 0x0401,
  };

  enum class SolutionState : uint16_t {
    UNLOCKED = 0x0000,
    LOCKED = 0x0004,
  };

  using Markup = uint8_t;
  static const Markup DefaultTag;
  static const Markup PreviousIncorrectTag;
  static const Markup IncorrectTag;
  static const Markup RevealedTag;
  static const Markup CircledTag;

  struct Timer {
    uint64_t current{0};
    bool running{false};
  };

private:
  Puzzle(QByteArray version, uint8_t height, uint8_t width,
         PuzzleType puzzleType, SolutionState solutionState,
         std::vector<Clue> clues[2], Grid<char> solution, Grid<char> grid,
         Grid<CellData> data, QByteArray text, Grid<Markup> markup, Timer timer,
         Grid<QString> rebusFill);
  QByteArray version_;

  uint8_t height_;
  uint8_t width_;
  PuzzleType puzzleType_;
  SolutionState solutionState_;
  std::vector<Clue> clues_[2];
  Grid<char> solution_;
  Grid<char> grid_;
  Grid<CellData> data_;
  QByteArray text_;
  Grid<Markup> markup_;
  Timer timer_;
  Grid<QString> rebusFill_{};

  QString title_;
  QString author_;
  QString copyright_;

public:
  static std::unique_ptr<Puzzle> loadFromFile(const QByteArray &puzFile);

  inline uint8_t getHeight() const { return height_; }
  inline uint8_t getWidth() const { return width_; }
  inline const std::vector<Clue> &getClues(Direction dir) const {
    return clues_[static_cast<int>(dir)];
  }
  inline Grid<char> &getGrid() { return grid_; }
  inline const Grid<char> &getGrid() const { return grid_; }
  inline Grid<Markup> &getMarkup() { return markup_; }
  inline const Grid<Markup> &getMarkup() const { return markup_; }
  inline const Grid<char> &getSolution() const { return solution_; }
  inline const Grid<CellData> &getCellData() const { return data_; }
  inline uint16_t getNumClues() const {
    return static_cast<uint16_t>(clues_[0].size() + clues_[1].size());
  }
  inline const QString &getTitle() const { return title_; }
  inline const QString &getAuthor() const { return author_; }
  inline const QString &getCopyright() const { return copyright_; }
  inline Timer &getTimer() { return timer_; }
  inline Grid<QString> &getRebusFill() { return rebusFill_; }
  inline const Grid<QString> &getRebusFill() const { return rebusFill_; }

  /// \return the clue index of clue number \p num in direction \p dir.
  int getClueIdxByNum(Direction dir, uint32_t num) const;

  /// \return the clue in direction \p dir with number \p num.
  const Clue &getClueByNum(Direction dir, uint32_t num) const {
    int idx = getClueIdxByNum(dir, num);
    return clues_[size_t(dir)][idx];
  }

  /// \return the clue in direction \p dir at index \p idx.
  inline const Clue &getClueByIdx(Direction dir, uint32_t idx) const {
    return clues_[size_t(dir)][idx];
  }

  /// \return the coordinate of the last cell in \p clue.
  std::pair<uint32_t, uint32_t> getClueEnd(const Clue &clue) const;

  /// \return the first blank space in \p clue, and return the start of the
  /// clue if there are none.
  std::pair<uint32_t, uint32_t> getFirstBlank(const Clue &clue) const;

  /// \return true if the grid entry at (row,col) is correct.
  inline bool check(uint8_t row, uint8_t col) const {
    qDebug() << "Checking" << row << ',' << col;
    const char solution = getSolution()[row][col];
    const char current = getGrid()[row][col];
    if (current == BLACK || current == EMPTY) {
      // Black or empty squares are always considered correct.
      return true;
    }
    if (current == solution || current == (solution | 32)) {
      return true;
    }
    // Otherwise, it's incorrect. Mark it as such.
    return false;
  }

  /// \return true if every cell has an entry in it.
  inline bool completelyFilled() const {
    for (const auto &row : grid_) {
      for (const auto &c : row) {
        if (c == EMPTY) {
          return false;
        }
      }
    }
    return true;
  }

  /// \return true if every cell is filled and correct.
  inline bool allCorrect() const {
    for (uint8_t r = 0; r < getHeight(); ++r) {
      for (uint8_t c = 0; c < getWidth(); ++c) {
        if (getGrid()[r][c] == EMPTY || !check(r, c)) {
          return false;
        }
      }
    }
    return true;
  }

  /// \return the number of the clue at position (\p row, \p col),
  /// in direction \p dir.
  inline uint32_t getNumByPosition(uint8_t row, uint8_t col,
                                   Direction dir) const {
    return dir == Direction::ACROSS ? data_[row][col].acrossNum
                                    : data_[row][col].downNum;
  }

  QByteArray serialize() const;

  inline QDebug dumpGrid(QDebug &stream) const {
    for (uint8_t r = 0; r < height_; ++r) {
      QString row;
      for (uint8_t c = 0; c < width_; ++c) {
        row += grid_[r][c];
      }
      stream << row << "\n";
    }
    return stream;
  }

private:
  static bool validatePuzzle(const QByteArray &puzFile);

  static uint16_t checksum(const QByteArray::const_iterator start,
                           const QByteArray::const_iterator end,
                           uint16_t seed = 0);

  static uint16_t checksum(const Grid<char> &grid, const uint16_t seed = 0);

  static uint16_t headerChecksum(uint8_t width, uint8_t height,
                                 uint16_t numClues, PuzzleType puzzleType,
                                 SolutionState solutionState,
                                 uint16_t seed = 0);

  static uint16_t textChecksum(uint16_t numClues, const QByteArray &text,
                               uint16_t seed = 0);

  static uint64_t magicChecksum(uint8_t width, uint8_t height,
                                uint16_t numClues, PuzzleType puzzleType,
                                SolutionState solutionState,
                                const Grid<char> &solution,
                                const Grid<char> &puzzle,
                                const QByteArray &text, uint16_t seed = 0);

  static uint16_t globalChecksum(uint8_t width, uint8_t height,
                                 uint16_t numClues, PuzzleType puzzleType,
                                 SolutionState solutionState,
                                 const Grid<char> &solution,
                                 const Grid<char> &puzzle,
                                 const QByteArray &text, uint16_t seed = 0);
};

} // namespace cygnus

#endif
