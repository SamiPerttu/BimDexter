// DxtBlock.h
// A DXT1 encoded pixel block.

#ifndef DXTBLOCK_H
#define DXTBLOCK_H

#include <cstdint>
#include <fstream>

#include "Pixmap.h"

using namespace std;


/// A 4x4 DXT1 encoded pixel block.
struct DxtBlock {

  static const int SIZE = 4;

  // Block colors.
  uint16_t color0;
  uint16_t color1;
  // 4x4 pixels in row major order, 2 bits per pixel, pixel (0, 0) in LSB.
  uint32_t bitmap;

  // Decodes this block and places it into the pixmap with the upper left corner at the given coordinates.
  void decode(Pixmap& pixmap, int x0, int y0);

  // Reads this block from the stream.
  void read(istream& s);

  // Writes this block to the stream.
  void write(ostream& s);

}; // struct DxtBlock


#endif // DXTBLOCK_H
