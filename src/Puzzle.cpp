#include "Puzzle.h"

#include <QDebug>

namespace cygnus {

const static char *MAGIC{"ACROSS&DOWN"};

/// Reads a Little-Endian 16-bit unsigned int.
static inline uint16_t readUInt16LE(const QByteArray::const_iterator start) {
  uint16_t a = *start & 0xff;
  uint16_t b = *(start + 1) & 0xff;
  return a | (b << 8);
}

/// Reads a Little-Endian 64-bit unsigned int.
static inline uint64_t readUInt64LE(const QByteArray::const_iterator start) {
  uint64_t result = 0;
  for (uint64_t i = 0; i < 8; ++i) {
    uint64_t b = *(start + i) & 0xff;
    result |= b << (8 * i);
  }
  return result;
}

/// Reads a null-terminated string from position \p offset.
/// \param[in,out] start first byte, will be update to point after the NUL byte.
static inline QString readString(QByteArray::const_iterator &start) {
  QString result{start};
  start += result.size() + 1;
  return result;
}

static Grid<char> readGrid(QByteArray::const_iterator &start,
                           const uint8_t height, const uint8_t width) {
  Grid<char> grid;
  grid.reserve(height);

  for (uint8_t r = 0; r < height; ++r) {
    std::vector<char> row;
    row.reserve(width);
    for (uint8_t c = 0; c < width; ++c) {
      char cell = *start++;
      row.push_back(cell);
      // switch (cell) {
      // case '.':
      //   row.push_back('\0');
      //   break;
      // case '-':
      //   row.push_back(' ');
      //   break;
      // default:
      //   row.push_back(cell);
      //   break;
      // }
    }
    grid.push_back(row);
  }

  return grid;
}

/// Computes the 16-bit checksum of the provided region.
/// \param seed the initial checksum to seed this computation with.
static inline uint16_t checksum(const QByteArray::const_iterator start,
                                const QByteArray::const_iterator end,
                                const uint16_t seed = 0) {
  uint16_t result = seed;
  for (auto it = start; it != end; ++it) {
    uint16_t lsb = result & 1;
    result >>= 1;
    // Carry the LSB over.
    result |= lsb << 15;
    result = (result + (*it & 0xff)) & 0xffff;
  }
  return result & 0xffff;
}

/// Computes the 16-bit checksum of the provided grid.
/// \param seed the initial checksum to seed this computation with.
static inline uint16_t checksum(const Grid<char> &grid,
                                const uint16_t seed = 0) {
  uint16_t result = seed;
  for (const auto &row : grid) {
    for (const auto c : row) {
      uint16_t lsb = result & 1;
      result >>= 1;
      // Carry the LSB over.
      result |= lsb << 15;
      result = (result + (c & 0xff)) & 0xffff;
    }
  }
  return result & 0xffff;
}

static inline uint16_t headerChecksum(uint8_t width, uint8_t height,
                                      uint16_t numClues, uint16_t mask1,
                                      uint16_t mask2, uint16_t seed = 0) {
  QByteArray header(8, 0);
  header[0] = width;
  header[1] = height;
  header[2] = numClues & 0xff;
  header[3] = numClues >> 8;
  header[4] = mask1 & 0xff;
  header[5] = mask1 >> 8;
  header[6] = mask2 & 0xff;
  header[7] = mask2 >> 8;
  return checksum(header.begin(), header.end(), seed);
}

static uint16_t textChecksum(const uint16_t numClues, const QByteArray &text,
                             uint16_t seed = 0) {
  uint16_t result{seed};
  auto it = text.begin();
  auto titleStart = it;
  auto title = readString(it);
  auto authorStart = it;
  auto author = readString(it);
  auto copyrightStart = it;
  auto copyright = readString(it);
  auto cluesStart = it;

  if (!title.isEmpty()) {
    result = checksum(titleStart, authorStart, result);
  }
  if (!author.isEmpty()) {
    result = checksum(authorStart, copyrightStart, result);
  }
  if (!copyright.isEmpty()) {
    result = checksum(copyrightStart, cluesStart, result);
  }

  for (uint16_t i = 0; i < numClues; ++i) {
    auto start = it;
    auto clue = readString(it);
    auto end = it - 1;
    result = checksum(start, end, result);
  }

  return result;
}

static uint64_t magicChecksum(uint8_t width, uint8_t height, uint16_t numClues,
                              uint16_t mask1, uint16_t mask2,
                              const Grid<char> &solution,
                              const Grid<char> &puzzle, const QByteArray &text,
                              uint16_t seed = 0) {
  const char MASK[]{"ICHEATED"};

  uint64_t checksums[4];
  checksums[3] = headerChecksum(width, height, numClues, mask1, mask2);
  checksums[2] = checksum(solution);
  checksums[1] = checksum(puzzle);
  checksums[0] = textChecksum(numClues, text);

  uint64_t result{seed};
  for (uint64_t i = 0; i < 4; ++i) {
    result <<= 8;
    result |= MASK[4 - i - 1] ^ (checksums[i] & 0xff);
    result |= (MASK[4 - i - 1 + 4] ^ (checksums[i] >> 8)) << 32;
  }

  return result;
}

static uint16_t globalChecksum(uint8_t width, uint8_t height, uint16_t numClues,
                               uint16_t mask1, uint16_t mask2,
                               const Grid<char> &solution,
                               const Grid<char> &puzzle, const QByteArray &text,
                               uint16_t seed = 0) {
  uint64_t result{seed};
  result = headerChecksum(width, height, numClues, mask1, mask2, result);
  result = checksum(solution, result);
  result = checksum(puzzle, result);
  result = textChecksum(numClues, text, result);
  return result;
}

static bool validatePuzzle(const QByteArray &puzFile) {
  if (puzFile.size() < 0x34) {
    return false;
  }
  uint8_t width = puzFile[0x2c];
  uint8_t height = puzFile[0x2d];
  if (puzFile.size() < 0x34 + 2 * (width * height)) {
    return false;
  }

  uint16_t numClues = readUInt16LE(puzFile.begin() + 0x2e);
  uint16_t mask1 = readUInt16LE(puzFile.begin() + 0x30);
  uint16_t mask2 = readUInt16LE(puzFile.begin() + 0x32);

  uint16_t headerChecksumExpected = readUInt16LE(puzFile.begin() + 0xe);
  qDebug("H Expected: 0x%04x", headerChecksumExpected);
  uint16_t headerChecksumActual =
      headerChecksum(width, height, numClues, mask1, mask2);
  qDebug("H Actual:   0x%04x", headerChecksumActual);

  if (headerChecksumExpected != headerChecksumActual) {
    qCritical() << "Header checksum check failed";
    return false;
  }

  auto it = puzFile.begin() + 0x34;
  Grid<char> solution = readGrid(it, height, width);
  Grid<char> puzzle = readGrid(it, height, width);

  uint64_t magicChecksumExpected = readUInt64LE(puzFile.begin() + 0x10);
  qDebug("M Expected: 0x%16llx", magicChecksumExpected);
  uint64_t magicChecksumActual =
      magicChecksum(width, height, numClues, mask1, mask2, solution, puzzle,
                    puzFile.mid(0x34 + (2 * width * height)));
  qDebug("M Actual:   0x%16llx", magicChecksumActual);

  if (magicChecksumExpected != magicChecksumActual) {
    qCritical() << "Magic checksum check failed";
    return false;
  }

  uint16_t globalChecksumExpected = readUInt16LE(puzFile.begin());
  qDebug("G Expected: 0x%04x", globalChecksumExpected);
  uint16_t globalChecksumActual =
      globalChecksum(width, height, numClues, mask1, mask2, solution, puzzle,
                     puzFile.mid(0x34 + (2 * width * height)));
  qDebug("G Actual:   0x%04x", globalChecksumActual);

  if (globalChecksumExpected != globalChecksumActual) {
    qCritical() << "Global checksum check failed";
    return false;
  }

  QByteArray magic = puzFile.mid(0x2, 0xb);
  if (magic != MAGIC || puzFile[0x0d] != '\x00') {
    qCritical() << "Magic number check failed";
    return false;
  }

  return true;
}

Puzzle *Puzzle::loadFromFile(const QByteArray &puzFile) {
  if (!validatePuzzle(puzFile)) {
    qCritical() << "Failed to validate puzzle";
    return nullptr;
  }

  uint8_t width = puzFile[0x2c];
  uint8_t height = puzFile[0x2d];
  uint16_t numClues = readUInt16LE(puzFile.begin() + 0x2e);

  auto it = puzFile.begin() + 0x34;
  Grid<char> solution = readGrid(it, height, width);
  Grid<char> grid = readGrid(it, height, width);

  QString title = readString(it);
  QString author = readString(it);
  QString copyright = readString(it);

  uint32_t num = 1;
  std::vector<Clue> across{};
  std::vector<Clue> down{};
  Grid<uint32_t> nums{};
  nums.reserve(height);

  for (uint8_t r = 0; r < height; ++r) {
    std::vector<uint32_t> numRow{};
    for (uint8_t c = 0; c < width; ++c) {
      if (grid[r][c] == BLACK) {
        numRow.push_back(0);
        continue;
      }
      bool a = c == 0 || grid[r][c - 1] == BLACK;
      bool d = r == 0 || grid[r - 1][c] == BLACK;
      if (a || d) {
        numRow.push_back(num);
        if (a) {
          Clue clue{readString(it), r, c, num};
          across.push_back(clue);
        }
        if (d) {
          Clue clue{readString(it), r, c, num};
          down.push_back(clue);
        }
        ++num;
      } else {
        numRow.push_back(0);
      }
    }
    nums.push_back(numRow);
  }

  std::vector<Clue> clues[2]{across, down};
  return new Puzzle{height, width, clues, solution, grid, nums};
}

Puzzle::Puzzle(uint8_t height, uint8_t width, std::vector<Clue> clues[2],
               Grid<char> solution, Grid<char> grid, Grid<uint32_t> nums)
    : height_(height), width_(width), clues_{clues[0], clues[1]},
      solution_(solution), grid_(grid), nums_(nums) {}

static bool compareForNum(const Clue &a, const Clue &b) {
  return a.num < b.num;
}

const int Puzzle::getClueByNum(Direction dir, uint32_t num) const {
  Clue clue{};
  clue.num = num;
  auto clues = clues_[static_cast<int>(dir)];
  auto result =
      std::lower_bound(clues.begin(), clues.end(), clue, compareForNum);
  if (result == clues.end()) {
    return -1;
  }
  return result - clues.begin();
}

const uint32_t Puzzle::getNumByPosition(uint8_t row, uint8_t col,
                                        Direction dir) const {
  if (dir == Direction::ACROSS) {
    do {
      if (col == 0 || grid_[row][col - 1] == BLACK) {
        return nums_[row][col];
      }
      --col;
    } while (col >= 0);
  } else {
    do {
      if (row == 0 || grid_[row - 1][col] == BLACK) {
        return nums_[row][col];
      }
      --row;
    } while (row >= 0);
  }
  return 0;
}

} // namespace cygnus
