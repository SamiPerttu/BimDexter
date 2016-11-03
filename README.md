# BimDexter

Clear and concise DXT1 compressor using a gradient descent algorithm.

Builds as a command line tool that converts between
24-bit uncompressed BMP and DXT1 encoded DDS file formats.
Visual Studio solution is included. The code should be portable anyway.

Run the program with no arguments to display usage.

See LICENSE.txt for licensing information (it is the MIT license).

The DXT1 compression method was designed as a compromise between quality and speed.
In comparison with libsquish, it replaces an exhaustive search for an optimal clustering
with heuristic gradient descent. The meat of the algorithm is in `BimDexter/PixelBlock.cpp`.

## Example

!(https://cdn.rawgit.com/SamiPerttu/BimDexter/master/examples/test-original.png "original image")

!(https://cdn.rawgit.com/SamiPerttu/BimDexter/master/examples/test-BimDexter.png "BimDexter compressed image")
RMS error: 4.87
compression time: 0.593 seconds

!(https://cdn.rawgit.com/SamiPerttu/BimDexter/master/examples/test-nvcompress.png "nvcompress.exe compressed image")
RMS error: 4.74
compression time: 0.836 seconds

!(https://cdn.rawgit.com/SamiPerttu/BimDexter/master/examples/test-nvcompress-fast.png "nvcompress.exe -fast compressed image")
RMS error: 5.05
compression time: 0.040 seconds
