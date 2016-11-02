// Pixmap.h
// Simple Pixmap class with reading and writing facilities.

#ifndef PIXMAP_H
#define PIXMAP_H

#include <vector>
#include <fstream>
#include <cstdint>

using namespace std;


// 24-bit RGB pixel.
struct Pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;

  Pixel() {}
  Pixel(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

  // Interpolates between two Pixels using integer arithmetic.
  static Pixel interpolate(Pixel const& a, int weight_a, Pixel const& b, int weight_b);

  // Reads the pixel from the stream in little-endian 24-bit RGB format.
  static Pixel read(istream& s);

  // Writes the pixel to the stream in little-endian 24-bit RGB format.
  void write(ostream& s);

}; // struct Pixel


// 24-bit RGB pixmap stored in row-major order.
class Pixmap {

  private:

    int sizeX_;
    int sizeY_;
    vector<Pixel> data_;

    int offset(int x, int y) const { return y * sizeX_ + x; }

  public:

    Pixmap() { }

    // Prohibit copy construction.
    Pixmap(Pixmap const&) = delete;

    // Prohibit assignment.
    void operator= (Pixmap const&) = delete;

    int sizeX() const { return sizeX_; }
    int sizeY() const { return sizeY_; }

    // Pixel accessor.
    Pixel& operator() (int x, int y) { return data_[offset(x, y)]; }

    // Pixel accessor.
    Pixel const& operator() (int x, int y) const { return data_[offset(x, y)]; }

    // Resizes the pixmap. Contents are undefined.
    void resize(int sizeX, int sizeY);

    // Reads a 24-bit uncompressed BMP stream. Throws runtime_error if something goes wrong.
    void read_bmp(istream&, bool verbose);

    // Writes a 24-bit uncompressed BMP stream.
    void export_bmp(ostream&, bool verbose);

    // Reads a DXT1 DDS stream. Throws runtime_error if something goes wrong.
    void read_dxt1(istream&, bool verbose);

    // Writes a DXT1 DDS stream.
    void export_dxt1(ostream&, bool verbose);

}; // class Pixmap


#endif // PIXMAP_H
