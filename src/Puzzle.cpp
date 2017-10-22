#include "Puzzle.h"

#include <QDebug>
#include <cassert>
#include <cstring>

namespace cygnus {

const static char MAGIC[] = "ACROSS&DOWN";

const Puzzle::Markup Puzzle::DefaultTag = 0x00;
const Puzzle::Markup Puzzle::PreviousIncorrectTag = 0x10;
const Puzzle::Markup Puzzle::IncorrectTag = 0x20;
const Puzzle::Markup Puzzle::RevealedTag = 0x40;
const Puzzle::Markup Puzzle::CircledTag = 0x80;

/// Reads a Little-Endian 16-bit unsigned int.
static inline uint16_t readUInt16LE(const QByteArray::const_iterator start) {
  uint16_t a = *start & 0xff;
  uint16_t b = *(start + 1) & 0xff;
  return a | (b << 8);
}

/// Writes a Little-Endian 16-bit unsigned int.
static inline void writeUInt16LE(QByteArray::iterator start, uint16_t x) {
  *start++ = x & 0xff;
  *start++ = (x >> 8) & 0xff;
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

/// Writes a Little-Endian 64-bit unsigned int.
static inline void writeUInt64LE(QByteArray::iterator start, uint64_t x) {
  for (uint64_t i = 0; i < 8; ++i) {
    *start++ = x & 0xff;
    x >>= 8;
  }
}

/// Reads a null-terminated string from position \p offset.
/// \param[in,out] start first byte, will be update to point after the NUL byte.
static inline QString readString(QByteArray::const_iterator &start) {
  QString result = QString::fromLatin1(start);
  start += result.size() + 1;
  return result;
}

static inline QByteArray makeUInt16LE(uint16_t x) {
  QByteArray result(2, '\0');
  result[0] = x & 0xff;
  result[1] = (x >> 8) & 0xff;
  return result;
}

template <typename T>
static Grid<T> readGrid(QByteArray::const_iterator &start, const uint8_t height,
                        const uint8_t width) {
  static_assert(sizeof(T) == sizeof(char),
                "readGrid can only read single bytes");
  Grid<T> grid;
  grid.reserve(height);

  for (uint8_t r = 0; r < height; ++r) {
    std::vector<T> row;
    row.reserve(width);
    for (uint8_t c = 0; c < width; ++c) {
      T cell = static_cast<T>(*start++);
      row.push_back(cell);
    }
    grid.push_back(row);
  }

  return grid;
}

/// Writes \p grid to \p start in place.
template <typename T>
static inline void writeGrid(QByteArray::iterator start, Grid<T> grid) {
  static_assert(sizeof(T) == 1, "Can only write grids of chars with this");
  for (const auto &row : grid) {
    for (const auto c : row) {
      *start++ = c;
    }
  }
}

/// Computes the 16-bit checksum of the provided region.
/// \param seed the initial checksum to seed this computation with.
uint16_t Puzzle::checksum(const QByteArray::const_iterator start,
                          const QByteArray::const_iterator end, uint16_t seed) {
  uint16_t result = seed;
  for (auto it = start; it != end; ++it) {
    uint16_t lsb = result & 1;
    result >>= 1;
    // Carry the LSB over.
    result |= lsb << 15;
    result = (result + (*it & 0xff)) & 0xffff;
  }
  return static_cast<uint16_t>(result & 0xffff);
}

/// Computes the 16-bit checksum of the provided grid.
/// \param seed the initial checksum to seed this computation with.
uint16_t Puzzle::checksum(const Grid<char> &grid, uint16_t seed) {
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

inline uint16_t Puzzle::headerChecksum(uint8_t width, uint8_t height,
                                       uint16_t numClues, PuzzleType puzzleType,
                                       SolutionState solutionState,
                                       uint16_t seed) {
  QByteArray header(8, 0);
  header[0] = width;
  header[1] = height;
  header[2] = numClues & 0xff;
  header[3] = numClues >> 8;
  writeUInt16LE(header.begin() + 4, static_cast<uint16_t>(puzzleType));
  writeUInt16LE(header.begin() + 6, static_cast<uint16_t>(solutionState));
  return checksum(header.begin(), header.end(), seed);
}

uint16_t Puzzle::textChecksum(uint16_t numClues, const QByteArray &text,
                              uint16_t seed) {
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

uint64_t Puzzle::magicChecksum(uint8_t width, uint8_t height, uint16_t numClues,
                               PuzzleType puzzleType,
                               SolutionState solutionState,
                               const Grid<char> &solution,
                               const Grid<char> &puzzle, const QByteArray &text,
                               uint16_t seed) {
  const char MASK[]{"ICHEATED"};

  uint64_t checksums[4];
  checksums[3] =
      headerChecksum(width, height, numClues, puzzleType, solutionState);
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

uint16_t Puzzle::globalChecksum(uint8_t width, uint8_t height,
                                uint16_t numClues, PuzzleType puzzleType,
                                SolutionState solutionState,
                                const Grid<char> &solution,
                                const Grid<char> &puzzle,
                                const QByteArray &text, uint16_t seed) {
  uint64_t result{seed};
  result = headerChecksum(width, height, numClues, puzzleType, solutionState,
                          result);
  result = checksum(solution, result);
  result = checksum(puzzle, result);
  result = textChecksum(numClues, text, result);
  return result;
}

bool Puzzle::validatePuzzle(const QByteArray &puzFile) {
  if (puzFile.size() < 0x34) {
    return false;
  }
  uint8_t width = puzFile[0x2c];
  uint8_t height = puzFile[0x2d];
  if (puzFile.size() < 0x34 + 2 * (width * height)) {
    return false;
  }

  uint16_t numClues = readUInt16LE(puzFile.begin() + 0x2e);
  PuzzleType puzzleType = PuzzleType(readUInt16LE(puzFile.begin() + 0x30));
  SolutionState solutionState =
      SolutionState(readUInt16LE(puzFile.begin() + 0x32));

  uint16_t headerChecksumExpected = readUInt16LE(puzFile.begin() + 0xe);
  uint16_t headerChecksumActual =
      headerChecksum(width, height, numClues, puzzleType, solutionState);

  if (headerChecksumExpected != headerChecksumActual) {
    qCritical() << "Header checksum check failed";
    return false;
  }

  auto it = puzFile.begin() + 0x34;
  auto solution = readGrid<char>(it, height, width);
  auto puzzle = readGrid<char>(it, height, width);

  uint64_t magicChecksumExpected = readUInt64LE(puzFile.begin() + 0x10);
  uint64_t magicChecksumActual =
      magicChecksum(width, height, numClues, puzzleType, solutionState,
                    solution, puzzle, puzFile.mid(0x34 + (2 * width * height)));

  // TODO: Re-enable with extensions excluded.
  // if (magicChecksumExpected != magicChecksumActual) {
  //   qCritical() << "Magic checksum check failed";
  //   return false;
  // }

  uint16_t globalChecksumExpected = readUInt16LE(puzFile.begin());
  uint16_t globalChecksumActual = globalChecksum(
      width, height, numClues, puzzleType, solutionState, solution, puzzle,
      puzFile.mid(0x34 + (2 * width * height)));

  // if (globalChecksumExpected != globalChecksumActual) {
  //   qCritical() << "Global checksum check failed";
  //   return false;
  // }

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
  (void)numClues;
  PuzzleType puzzleType = PuzzleType(readUInt16LE(puzFile.begin() + 0x30));
  SolutionState solutionState =
      SolutionState(readUInt16LE(puzFile.begin() + 0x32));

  auto it = puzFile.begin() + 0x34;
  auto solution = readGrid<char>(it, height, width);
  auto grid = readGrid<char>(it, height, width);

  qDebug() << "Text start offset:" << (it - puzFile.begin());
  const auto textStart = it;
  QString title = readString(it);
  QString author = readString(it);
  QString copyright = readString(it);

  uint32_t num = 1;
  std::vector<Clue> across{};
  std::vector<Clue> down{};
  Grid<CellData> data{};
  data.reserve(height);

  uint32_t acrossIdx = 0;
  uint32_t downIdx = 0;

  for (uint8_t r = 0; r < height; ++r) {
    std::vector<CellData> dataRow{};
    dataRow.reserve(width);
    for (uint8_t c = 0; c < width; ++c) {
      if (grid[r][c] == BLACK) {
        dataRow.push_back(CellData{});
        continue;
      }
      bool a = c == 0 || grid[r][c - 1] == BLACK;
      bool d = r == 0 || grid[r - 1][c] == BLACK;
      if (a || d) {
        CellData data{};
        if (a) {
          Clue clue{readString(it), r, c, num, Direction::ACROSS};
          across.push_back(clue);
          data.acrossNum = num;
          data.acrossStart = true;
          data.acrossIdx = acrossIdx++;
        }
        if (d) {
          Clue clue{readString(it), r, c, num, Direction::DOWN};
          down.push_back(clue);
          data.downNum = num;
          data.downStart = true;
          data.downIdx = downIdx++;
        }
        dataRow.push_back(data);
        ++num;
      } else {
        dataRow.push_back(CellData{});
      }
    }
    data.push_back(dataRow);
  }

  qDebug() << "Text end offset:" << (it - puzFile.begin());
  const auto textEnd = it;

  Grid<Markup> markup{};
  markup.resize(height);
  for (uint8_t i = 0; i < height; ++i) {
    markup[i].resize(width);
    std::fill(markup[i].begin(), markup[i].end(), Puzzle::DefaultTag);
  }

  Timer timer{};

  // Try and read extensions.
  while (it < puzFile.end() - 8) {
    if (::strncmp(it, "GEXT", 4) == 0) {
      // Read the markup.
      qDebug() << "Reading extension: Markup";
      it += 4;
      uint16_t len = readUInt16LE(it);
      (void)len;
      it += 4;
      markup = readGrid<Markup>(it, height, width);
      qDebug() << "Read extension:    Markup";
      // Account for the NUL character.
      ++it;
    } else if (::strncmp(it, "LTIM", 4) == 0) {
      qDebug() << "Reading extension: Timer";
      it += 4;
      uint16_t len = readUInt16LE(it);
      it += 2;
      uint16_t cksum = readUInt16LE(it);
      // TODO: Check checksum here.
      (void)cksum;
      it += 2;
      QString timerString = readString(it);
      if (timerString.size() != len) {
        qDebug() << "Invalid length";
        return nullptr;
      }
      auto sepIndex = timerString.indexOf(',');
      if (sepIndex != len - 2) {
        qDebug() << "Invalid comma location";
        return nullptr;
      }
      bool ok;
      timer.current = timerString.leftRef(len - 2).toULongLong(&ok);
      if (!ok) {
        qDebug() << "Invalid time";
        return nullptr;
      }
      timer.running = timerString.at(len - 1) == '1';
      qDebug() << "Read extension:    Timer";
      qDebug() << "Time:" << timer.current << "s";
      qDebug() << "Running:" << timer.running;
    } else {
      ++it;
    }
  }

  // Perform post-processing on the cellData.
  uint32_t curNum = 0;
  uint32_t curIdx = 0;
  for (uint8_t r = 0; r < height; ++r) {
    for (uint8_t c = 0; c < width; ++c) {
      if (data[r][c].acrossStart) {
        curNum = data[r][c].acrossNum;
        curIdx = data[r][c].acrossIdx;
      } else {
        data[r][c].acrossNum = curNum;
        data[r][c].acrossIdx = curIdx;
      }
    }
  }

  for (uint8_t c = 0; c < width; ++c) {
    for (uint8_t r = 0; r < height; ++r) {
      if (data[r][c].downStart) {
        curNum = data[r][c].downNum;
        curIdx = data[r][c].downIdx;
      } else {
        data[r][c].downNum = curNum;
        data[r][c].downIdx = curIdx;
      }
    }
  }

  std::vector<Clue> clues[2]{across, down};
  QByteArray version = puzFile.mid(0x18, 4);
  return new Puzzle{
      version,
      height,
      width,
      puzzleType,
      solutionState,
      clues,
      solution,
      grid,
      data,
      puzFile.mid(textStart - puzFile.begin(), textEnd - textStart),
      markup,
      timer};
}

Puzzle::Puzzle(QByteArray version, uint8_t height, uint8_t width,
               PuzzleType puzzleType, SolutionState solutionState,
               std::vector<Clue> clues[2], Grid<char> solution, Grid<char> grid,
               Grid<CellData> data, QByteArray text, Grid<Markup> markup,
               Timer timer)
    : version_(version), height_(height), width_(width),
      puzzleType_(puzzleType),
      solutionState_(solutionState), clues_{clues[0], clues[1]},
      solution_(solution), grid_(grid), data_(data), text_(text),
      markup_(markup), timer_(timer) {
  QByteArray::const_iterator it = text.begin();
  title_ = readString(it);
  author_ = readString(it);
  copyright_ = readString(it);
}

static bool compareForNum(const Clue &a, const Clue &b) {
  return a.num < b.num;
}

int Puzzle::getClueIdxByNum(Direction dir, uint32_t num) const {
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

const Clue &Puzzle::getClueByNum(Direction dir, uint32_t num) const {
  int idx = getClueIdxByNum(dir, num);
  return clues_[size_t(dir)][idx];
}

uint32_t Puzzle::getNumByPosition(uint8_t row, uint8_t col,
                                  Direction dir) const {
  return dir == Direction::ACROSS ? data_[row][col].acrossNum
                                  : data_[row][col].downNum;
}

const Clue &Puzzle::getClueByIdx(Direction dir, uint32_t idx) const {
  return clues_[size_t(dir)][idx];
}

QByteArray Puzzle::serialize() const {
  QByteArray result(0x34 + (2 * static_cast<uint16_t>(width_) *
                            static_cast<uint16_t>(height_)),
                    '\0');

  writeUInt16LE(result.begin(),
                globalChecksum(width_, height_, getNumClues(), puzzleType_,
                               solutionState_, solution_, grid_, text_));
  std::copy(MAGIC, MAGIC + sizeof(MAGIC), result.begin() + 0x2);
  writeUInt16LE(result.begin() + 0x0e,
                headerChecksum(width_, height_, getNumClues(), puzzleType_,
                               solutionState_));
  writeUInt64LE(result.begin() + 0x10,
                magicChecksum(width_, height_, getNumClues(), puzzleType_,
                              solutionState_, solution_, grid_, text_));
  result.replace(0x18, version_.size(), version_);

  result[0x2c] = width_;
  result[0x2d] = height_;
  writeUInt16LE(result.begin() + 0x2e, getNumClues());
  writeUInt16LE(result.begin() + 0x30, uint16_t(puzzleType_));
  writeUInt16LE(result.begin() + 0x32, uint16_t(solutionState_));

  writeGrid(result.begin() + 0x34, solution_);
  writeGrid(result.begin() + 0x34 + (width_ * height_), grid_);

  result += text_;
  result += '\0';

  // Serialize markup.
  QByteArray markupString{width_ * height_, '\0'};
  writeGrid(markupString.begin(), markup_);

  result += QByteArray("GEXT", 4) + makeUInt16LE(width_ * height_) +
            makeUInt16LE(checksum(markupString.begin(), markupString.end())) +
            markupString + '\0';

  // Serialize timer.
  QByteArray timeString = QString("%1,%2")
                              .arg(timer_.current)
                              .arg(timer_.running ? '1' : '0')
                              .toLocal8Bit();
  uint16_t timerChecksum = checksum(timeString.begin(), timeString.end());
  uint16_t timerLen = timeString.size();
  QByteArray timerExtension = QByteArray("LTIM", 4) + makeUInt16LE(timerLen) +
                              makeUInt16LE(timerChecksum) + timeString + '\0';

  result += timerExtension;

  return result;
}

} // namespace cygnus
