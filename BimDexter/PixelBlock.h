// PixelBlock.h
// DXT1 pixel blocks and compression.

#ifndef PIXELBLOCK_H
#define PIXELBLOCK_H

#include <valarray>

#include "Vec3.h"
#include "DxtBlock.h"

using namespace std;


// A DXT1 (non-alpha) block palette.
struct DxtPalette {

    static const int SIZE = 4;

    // Colors 0 and 1 are encoded in the block. Colors 2 and 3 are interpolated from colors 0 and 1.
    Vec3 color[SIZE];

    // Clamps colors 0 and 1 and then interpolates colors 2 and 3 from colors 0 and 1.
    void complete();

    // Sets relative importances of color components with respect to the squared error.
    // This is (3, 4, 2) by default.
    static void setColorImportance(Vec3 importance);

    // (Square roots of) color importances are stored here.
    static Vec3 colorImportance;

}; // struct DxtPalette


// Result of encoding a single pixel.
struct CodedPixel {

    // Error gradient for color 0.
    Vec3 gradient0;
    // Error gradient for color 1.
    Vec3 gradient1;
    // Weighted squared distance error.
    float error;
    // Nearest color in [0, 3].
    int color;

    // Encodes the pixel using the palette and stores the results here.
    void encode(Vec3 pixel, DxtPalette const& palette);

}; // struct CodedPixel


// 4x4 RGB pixel block.
class PixelBlock {

  public:

    // Number of pixels in the block.
    static const int N = 16;

    PixelBlock();

    // Reads this block from the pixmap at the specified position.
    void read(Pixmap const& pixmap, int x, int y);

    // Total squared weighted compression error. Does not include quantization
    // error from the block palette.
    float error() const { return error_; }

    // Compresses the contents of this block. Returns the compressed block
    // and sets the compression error.
    DxtBlock compress_dxt1();

  private:

    // Returns squared compression error using the palette.
    float compute_error(DxtPalette const& palette) const;

    // Runs gradient descent to fine-tune the palette.
    float gradient_descent(int max_iterations, DxtPalette& palette);

    // Pixels are stored in floating point for convenience.
    // Components contain 8 bits per pixel values multiplied by importance.
    valarray<Vec3> data_;
   
    float error_;

}; // class PixelBlock


#endif // PIXELBLOCK_H
