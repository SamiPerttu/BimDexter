// PixelBlock.cpp

#include <algorithm>
#include <iostream>

#include "PixelBlock.h"
#include "Pixmap.h"
#include "Common.h"
#include "Vec3.h"

using namespace std;


// Weighting factor of second (sic) color for each color in a DXT1 block palette.
float colorWeight[4] { 0.0f, 1.0f, 1.0f/3.0f, 2.0f/3.0f };


Vec3 DxtPalette::colorImportance = Vec3(sqrt(3.0f), sqrt(4.0f), sqrt(2.0f));


void DxtPalette::setColorImportance(Vec3 importance)
{
    colorImportance = Vec3(sqrt(importance.x), sqrt(importance.y), sqrt(importance.z));
}


void DxtPalette::complete()
{
    auto colorMaximum = colorImportance * 255.0f;
    for (int i = 0; i < 2; ++i)
        color[i].clamp(Vec3(0.0f), colorMaximum);
    for (int i = 2; i < 4; ++i)
        color[i] = Vec3::lerp(color[0], color[1], colorWeight[i]);
}


void CodedPixel::encode(Vec3 pixel, DxtPalette const& palette)
{
    error = 1.0e10;

    for (int i = 0; i < DxtPalette::SIZE; ++i) {
        auto g = palette.color[i] - pixel;
        auto e = g.length2();
        if (e < error) {
            error     = e;
            gradient0 = g * (1 - colorWeight[i]);
            gradient1 = g * colorWeight[i];
            color     = i;
        }
    }
}


PixelBlock::PixelBlock() : data_(Vec3(0), N), error_(0)
{
}


void PixelBlock::read(Pixmap const& pixmap, int x, int y)
{
    int i = 0;

    // DXT1 files are encoded upside down so we reverse the Y axis of the block here.
    for (int dy = 3; dy >= 0; --dy) {
        for (int dx = 0; dx < 4; ++dx) {
            auto pixel = pixmap(x + dx, y + dy);
            data_[i++] = Vec3(pixel.r, pixel.g, pixel.b) * DxtPalette::colorImportance;
        }
    }
}


// Converts an 8-bit color value to a 5-bit color value. This inverts the 5-to-8 bit
// conversion (x << 3) + (x << 2). The formula was discovered with genetic programming.
int convert_8to5(float f)
{
    int x = (int)roundf(f);
    int r5 = x - ((x - 124) >> 5);
    return r5 >> 3;
}


// Converts an 8-bit color value to a 6-bit color value. This inverts the 6-to-8 bit
// conversion (x << 2) + (x >> 4). The formula was discovered with genetic programming.
int convert_8to6(float f)
{
    int x = (int)roundf(f) + 2;
    int r3 = x - (x >> 6);
    return r3 >> 2;
}


// Encodes an importance weighted 24-bit RGB triple in the R5G6B5 16-bit format used by DXT1.
uint16_t encode_565(Vec3 const& color)
{
    auto color8 = color / DxtPalette::colorImportance;
    return ((uint16_t)convert_8to5(color8.z) << 11) + ((uint16_t)convert_8to6(color8.y) << 5) + (uint16_t)convert_8to5(color8.x);
}


DxtBlock PixelBlock::compress_dxt1()
{
    // Here we are explicitly attempting to minimize total squared error
    // with respect to importance weighted components. This makes the most sense
    // if the image data is gamma corrected, as then the Euclidean metric
    // can work reasonably well in measuring perceptual error.
    
    // Note. Importance weighting is just an unequal scaling factor; we could
    // actually apply any affine transform, for example a transform to YUV, then
    // optimize the palette there, and transform back when finished.
    // Working in YUV would allow us to assign an independent importance weight
    // to luminance (this is not implemented).
    error_ = 0;

    DxtBlock block;

    // Compute the covariance matrix for the color components. The matrix is symmetric
    // so we can regard covX, covY and covZ as either rows or columns.

    auto mean = data_.sum() / (float)N;
    auto covX = Vec3(0.0f);
    auto covY = Vec3(0.0f);
    auto covZ = Vec3(0.0f);

    for(int i = 0; i < N; ++i) {
        auto d = data_[i] - mean;
        covX += d * d.x;
        covY += d * d.y;
        covZ += d * d.z;
    }

    // Check here if we have a constant color block. Include some numerical tolerance in the test.
    if (covX.x + covY.y + covZ.z < 0.1f) {
        block.color0 = encode_565(data_[0]);
        block.color1 = 0;
        block.bitmap = 0;
        return block;
    }

    // Note that the normalization factor of an unbiased estimator is 1 / (N - 1) but 1 / N
    // yields the best fit to the data.
    covX /= (float)N;
    covY /= (float)N;
    covZ /= (float)N;

    // Now we approximate the principal eigenpair using power iteration. The largest amount of
    // variance is concentrated in the direction of the principal eigenvector, and that is what
    // we want to account for with our color choices.
    // As a starting point, we pick a diagonal of the bounding box of the colors in the block.
    // Power iteration converges slowly if there is more than one prominent eigenvalue of similar
    // magnitude; the initial guess at least provides good contrast then.
    // We could also derive the eigenpairs from the solutions of a cubic equation but that
    // is not likely to be significantly faster, and would require higher numerical precision.

    auto mini = data_[0];
    auto maxi = data_[0];

    for(int i = 1; i < N; ++i) {
        mini = Vec3::minimize(mini, data_[i]);
        maxi = Vec3::maximize(maxi, data_[i]);
    }

    // The eigenvector estimate is stored here.
    auto b = maxi - mini;

    // The eigenvalue estimate is stored here.
    auto v = 0.0f;

    // Do a fixed number of iterations.
    for(int iteration = 0; iteration < 12; ++iteration) {
        b  = Vec3(Vec3::dot(b, covX), Vec3::dot(b, covY), Vec3::dot(b, covZ));
        v  = b.length();
        b /= v;
    }

    // Now estimate the two colors from sample mean and the principal eigenpair.
    // (The sample mean is the single point that minimizes squared error.)
    // The other two colors are interpolated from them. We run gradient descent
    // for a small amount of steps with three different starting points, keep
    // the best result, and refine it some more.

    DxtPalette palette;
    auto error = 1.0e10f;

    for (float factor = 0.5f; factor <= 2.0f; factor *= 2.0f) {
        auto stdev = sqrt(factor * v);

        DxtPalette candidate_palette;
        candidate_palette.color[0] = mean + stdev * b;
        candidate_palette.color[1] = mean - stdev * b;
        candidate_palette.complete();

        auto candidate_error = gradient_descent(8, candidate_palette);

        if (candidate_error < error) {
            palette = candidate_palette;
            error = candidate_error;
        }
    }

    error = gradient_descent(64, palette);

    // Encode the block.

    block.color0 = encode_565(palette.color[0]);
    block.color1 = encode_565(palette.color[1]);
    block.bitmap = 0;

    // DXT1 specifies that color0 > color1 for the block to be interpreted as
    // a non-alpha encoding.
    if (block.color0 < block.color1) {
        swap(block.color0, block.color1);
        swap(palette.color[0], palette.color[1]);
        swap(palette.color[2], palette.color[3]);
    }

    CodedPixel coded;

    for (int i = 0; i < N; ++i) {
        coded.encode(data_[i], palette);
        error_ += coded.error;
        block.bitmap |= coded.color << (i * 2);
    }

    // If color0 = color1, the block is logically encoded with alpha but
    // we use the first color only.
    if (block.color0 == block.color1) block.bitmap = 0;

    error_ /= DxtPalette::colorImportance.length2();

    return block;
}


float PixelBlock::gradient_descent(int max_iterations, DxtPalette& palette)
{
    // Start with an empirically chosen step size.
    float step_size = 8.0f / (float)N;

    // We stop gradient descent when the step size falls below this value.
    // The divisor (1 << N) translates to N rejected steps.
    float minimum_step_size = step_size / (1 << 4);

    CodedPixel coded;
    auto error = 0.0f;
    auto gradient0 = Vec3(0.0f);
    auto gradient1 = Vec3(0.0f);

    for (int i = 0; i < N; ++i) {
        coded.encode(data_[i], palette);
        error += coded.error;
        gradient0 += coded.gradient0;
        gradient1 += coded.gradient1;
    }

    DxtPalette new_palette;

    for (int iteration = 0; iteration < max_iterations && step_size > minimum_step_size; ++iteration) {

        // Take a step in the gradient directions.
        for (int i = 0; i < 2; ++i)
            new_palette.color[i] = palette.color[i] - Vec3::lerp(gradient0, gradient1, colorWeight[i]) * step_size;
        new_palette.complete();

        auto new_error = 0.0f;
        auto new_gradient0 = Vec3(0.0f);
        auto new_gradient1 = Vec3(0.0f);

        for (int i = 0; i < N; ++i) {
            coded.encode(data_[i], new_palette);
            new_error += coded.error;
            new_gradient0 += coded.gradient0;
            new_gradient1 += coded.gradient1;
        }

        if (new_error < error) {
            // Accept the step and increase step size.
            palette   = new_palette;
            error     = new_error;
            gradient0 = new_gradient0;
            gradient1 = new_gradient1;
            step_size *= 1.2;
        } else {
            // Error was not reduced. Try a smaller step size.
            step_size *= 0.5;
        }
    }

    return error;
}
