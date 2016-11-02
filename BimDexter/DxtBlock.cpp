// DxtBlock.cpp

#include "DxtBlock.h"
#include "Common.h"

using namespace std;


// Retrieves the R component from a R5G6B5 DXT1 color as an 8-bit value.
uint8_t r_565(uint16_t color) { int r = color & 0x1f; return (uint8_t)((r << 3) + (r >> 2)); }


// Retrieves the G component from a R5G6B5 DXT1 color as an 8-bit value.
uint8_t g_565(uint16_t color) { int g = (color >> 5) & 0x3f; return (uint8_t)((g << 2) + (g >> 4)); }


// Retrieves the B component from a R5G6B5 DXT1 color as an 8-bit value.
uint8_t b_565(uint16_t color) { int b = (color >> 11) & 0x1f; return (uint8_t)((b << 3) + (b >> 2)); }


// Converts an R5G6B5 DXT1 color to a Pixel.
Pixel decode_565(uint16_t color)
{
    return Pixel(r_565(color), g_565(color), b_565(color));
}


void DxtBlock::decode(Pixmap& pixmap, int x0, int y0)
{
    Pixel color[4];
    color[0] = decode_565(color0);
    color[1] = decode_565(color1);
    color[2] = Pixel::interpolate(color[0], 2, color[1], 1);
    color[3] = Pixel::interpolate(color[0], 1, color[1], 2);

    uint32_t b = bitmap;

    // Blocks are encoded upside down. We flip them here.
    for(int y = SIZE - 1; y >= 0; --y) {
        for(int x = 0; x < SIZE; ++x) {
            pixmap(x0 + x, y0 + y) = color[b & 3];
            b >>= 2;
        }
    }

}


void DxtBlock::read(istream& s)
{
    color0 = read_16_le(s);
    color1 = read_16_le(s);
    bitmap = read_32_le(s);
}


void DxtBlock::write(ostream& s)
{
    write_16_le(s, color0);
    write_16_le(s, color1);
    write_32_le(s, bitmap);
}
