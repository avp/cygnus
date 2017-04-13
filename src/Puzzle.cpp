#include "Puzzle.h"

#include <QDebug>

const static char *MAGIC{"\x41\x43\x52\x4f\x53\x53\x26\x44\x4f\x57\x4e"};

static inline uint16_t readUInt16LE(const QByteArray &puzFile,
                                    const uint32_t offset) {
  return puzFile[offset] | (puzFile[offset + 1] << 8);
}

static inline QString readString(const QByteArray &puzFile,
                                 const uint32_t offset) {
  return QString{puzFile.data() + offset};
}

static inline Grid<char> readGrid(const QByteArray &puzFile,
                                  const uint32_t offset, const uint8_t width,
                                  const uint8_t height) {
  Grid<char> grid;
  grid.reserve(height);

  for (uint8_t r = 0; r < height; ++r) {
    std::vector<char> row;
    row.reserve(width);
    for (uint8_t c = 0; c < width; ++c) {
      char cell = puzFile[offset + (r * width) + c];
      switch (cell) {
      case '.':
        row.push_back('\0');
        break;
      case '-':
        row.push_back(' ');
        break;
      default:
        row.push_back(cell);
        break;
      }
    }
    grid.push_back(row);
  }

  return grid;
}

Puzzle *Puzzle::loadFromFile(const QByteArray &puzFile) {
  QString str{puzFile};
  QStringRef magic{&str, 0x02, 0xb};
  if (magic != MAGIC || str[0x0d] != '\x00') {
    return nullptr;
  }

  uint8_t width = puzFile[0x2c];
  uint8_t height = puzFile[0x2d];
  uint8_t numClues = readUInt16LE(puzFile, 0x2e);

  Grid<char> solution = readGrid(puzFile, 0x34, width, height);
  Grid<char> grid = readGrid(puzFile, 0x34 + (width * height), width, height);

  uint32_t offset = 0x34 + (2 * width * height);
  QString title = readString(puzFile, offset);
  offset += title.size() + 1;
  QString author = readString(puzFile, offset);
  offset += author.size() + 1;
  QString copyright = readString(puzFile, offset);
  offset += copyright.size() + 1;

  uint32_t num = 1;
  std::vector<Clue> across{};
  std::vector<Clue> down{};
  Grid<uint32_t> nums{};
  nums.reserve(height);

  for (uint8_t r = 0; r < height; ++r) {
    std::vector<uint32_t> numRow{};
    for (uint8_t c = 0; c < width; ++c) {
      if (grid[r][c] == '\0') {
        numRow.push_back(0);
        continue;
      }
      bool a = c == 0 || grid[r][c - 1] == '\0';
      bool d = r == 0 || grid[r - 1][c] == '\0';
      if (a || d) {
        numRow.push_back(num);
        ++num;
        if (a) {
          Clue clue{readString(puzFile, offset), r, c, num};
          offset += clue.clue.size() + 1;
          across.push_back(clue);
        }
        if (d) {
          Clue clue{readString(puzFile, offset), r, c, num};
          offset += clue.clue.size() + 1;
          down.push_back(clue);
        }
      } else {
        numRow.push_back(0);
      }
    }
    nums.push_back(numRow);
  }

  return nullptr;
}

Puzzle::Puzzle() {}
