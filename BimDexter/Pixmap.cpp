// Pixmap.cpp

#include <cstdint>
#include <exception>
#include <iostream>

#include "Pixmap.h"
#include "PixelBlock.h"
#include "DxtBlock.h"
#include "Common.h"

using namespace std;


// Skips the specified number of input bytes.
void skip(istream& s, int bytes)
{
    s.seekg(bytes, ios::cur);
}


Pixel Pixel::interpolate(Pixel const& a, int weight_a, Pixel const& b, int weight_b)
{
    auto W = weight_a + weight_b;
    return Pixel(
        ((int)a.r * weight_a + (int)b.r * weight_b) / W,
        ((int)a.g * weight_a + (int)b.g * weight_b) / W,
        ((int)a.b * weight_a + (int)b.b * weight_b) / W);
}


Pixel Pixel::read(istream& s)
{
    union {
        char buf[3];
        unsigned char ubuf[3];
    };
    s.read(buf, 3);
    return Pixel(ubuf[0], ubuf[1], ubuf[2]);
}


void Pixel::write(ostream& s)
{
    char buf[3];
    buf[0] = r;
    buf[1] = g;
    buf[2] = b;
    s.write(buf, 3);
}


void Pixmap::resize(int sizeX, int sizeY)
{
    sizeX_ = sizeX;
    sizeY_ = sizeY;
    data_.resize(sizeX * sizeY);
}


void Pixmap::read_bmp(istream &s, bool verbose)
{
    // Read header.

    auto fileType = read_16_le(s);
    if (fileType != 'MB') throw runtime_error("BMP filetype header not found.");
    skip(s, 8);
    auto bitmapOffset = read_32_le(s);
    auto headerSize = read_32_le(s);
    auto width = (int)read_32_le(s);
    auto height = (int)read_32_le(s);
    if (width & 3) throw runtime_error("BMP image width must be divisible by 4.");
    if (height & 3) throw runtime_error("BMP image height must be divisible by 4.");
    auto planes = read_16_le(s);
    auto bpp = read_16_le(s);
    if (bpp != 24) throw runtime_error("Only 24-bit BMP bitmap format is supported.");
    // If this is BMP version 3 or 4, check that the file is uncompressed.
    if (headerSize > 14) {
        auto compression = read_32_le(s);
        if (compression != 0) throw runtime_error("Only uncompressed BMP files are supported.");
    }

    // Read bitmap data.

    resize(width, height);
    s.seekg(bitmapOffset, ios::beg);
    if (verbose) cerr << "Reading " << width << "x" << height << " BMP image.\n";

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            (*this)(x, y) = Pixel::read(s);
        }
    }
}

void Pixmap::export_bmp(ostream &s, bool verbose)
{
    int bitmapOffset = 54;
    int filesize = bitmapOffset + 3 * sizeX() * sizeY();

    // Write header.

    write_16_le(s, 'MB');
    write_32_le(s, filesize);
    write_32_le(s, 0);
    write_32_le(s, bitmapOffset);
    // Write a BMP version 3.X file.
    write_32_le(s, 40);
    write_32_le(s, sizeX());
    write_32_le(s, sizeY());
    // Planes, BPP, Compression, SizeOfBitmap, HorzResolution, VertResolution, ColorsUsed, ColorsImportant.
    write_16_le(s, 1);
    write_16_le(s, 24);
    write_32_le(s, 0);
    write_32_le(s, 0);
    write_32_le(s, 100);
    write_32_le(s, 100);
    write_32_le(s, 1 << 24);
    write_32_le(s, 0);

    // Write bitmap data.

    for (int y = 0; y < sizeY(); ++y) {
        for (int x = 0; x < sizeX(); ++x) {
            (*this)(x, y).write(s);
        }
    }
}

void Pixmap::read_dxt1(istream &s, bool verbose)
{
    auto filetype = read_32_le(s);
    if (filetype != ' SDD') throw runtime_error("DDS filetype header not found.");
    // Header length.
    int headerLength = read_32_le(s);
    if (headerLength != 124) throw runtime_error("Invalid DDS header length.");
    auto flags = read_32_le(s);
    int height = read_32_le(s);
    int width = read_32_le(s);
    if (width & 3) throw runtime_error("DDS image width must be divisible by 4.");
    if (height & 3) throw runtime_error("DDS image height must be divisible by 4.");
    int linearSize = read_32_le(s);
    int depth = read_32_le(s);
    int mipmapcount = read_32_le(s);
    skip(s, 4 * 11);
    int formatHeaderLength = read_32_le(s);
    int dwFlags = read_32_le(s);
    if (dwFlags != 4) throw runtime_error("Only compressed non-alpha RGB files supported.");
    int fourcc = read_32_le(s);
    if (fourcc != '1TXD') throw runtime_error("Only DXT1 compressed files supported.");
    skip(s, 4 * 5);
    int content = read_32_le(s);
    if (content != 0x1000) throw runtime_error("DDS file content must be texture.");
    skip(s, 4 * 4);
    
    if (verbose) cerr << "Reading " << width << "x" << height << " DDS image.\n";

    // Read pixel block data. Note that they are written upside down.
    resize(width, height);

    DxtBlock dxt;

    for (int y = sizeY() - 4; y >= 0; y -= 4) {
        for (int x = 0; x < sizeX(); x += 4) {
            dxt.read(s);
            dxt.decode(*this, x, y);
        }
    }
}

void Pixmap::export_dxt1(ostream &s, bool verbose)
{
    // Write header.

    write_32_le(s, ' SDD');
    // Header length.
    write_32_le(s, 124);
    // Data flags (CAPS, HEIGHT, WIDTH, PIXELFORMAT, LINEARSIZE).
    write_32_le(s, 0x1 + 0x2 + 0x4 + 0x1000 + 0x80000);
    write_32_le(s, sizeY());
    write_32_le(s, sizeX());
    // PitchOrLinearSize, Depth, MipMapCount, dwReserved[11].
    write_32_le(s, sizeX() / 4 * sizeY() / 4 * 8);
    write_32_le(s, 0);
    write_32_le(s, 0);
    for (int i = 0; i < 11; ++i) write_32_le(s, 0);
    // Pixel format: dwSize, dwFlags, dwFourCC, dwRGBBitCount, dw[R, G, B, A]BitMask.
    write_32_le(s, 32);
    write_32_le(s, 0x4); // DDPF_FOURCC
    write_32_le(s, '1TXD');
    write_32_le(s, 0);
    write_32_le(s, 0xff0000); 
    write_32_le(s, 0x00ff00);
    write_32_le(s, 0x0000ff);
    write_32_le(s, 0);
    // dwCaps, dwCaps[2..4], dwReserved2.
    write_32_le(s, 0x1000); // DDSCAPS_TEXTURE
    write_32_le(s, 0);
    write_32_le(s, 0);
    write_32_le(s, 0);
    write_32_le(s, 0);

    PixelBlock block;
    float error = 0;

    // Export pixel blocks. Note that they are written upside down.
    for (int y = sizeY() - 4; y >= 0; y -= 4) {
        for (int x = 0; x < sizeX(); x += 4) {
            block.read(*this, x, y);
            auto dxt = block.compress_dxt1();
            error += block.error();
            dxt.write(s);
        }
    }

    if (verbose) {
        cerr << "DDS image written. Weighted RMS error per pixel: "
             << sqrt(error / (float)sizeX() / (float)sizeY()) * 100.0f / 256.0f
             << "%.\n";
    }
}
