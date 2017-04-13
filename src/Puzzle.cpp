#include "Puzzle.h"

#include <QDebug>

const static char *MAGIC{"\x41\x43\x52\x4f\x53\x53\x26\x44\x4f\x57\x4e"};

static uint16_t readUInt16LE(const QByteArray &puzFile, uint32_t offset) {
  return puzFile[offset] | (puzFile[offset + 1] << 8);
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

  return nullptr;
}

Puzzle::Puzzle() {}
