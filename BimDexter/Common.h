// Common.h
// Some common definitions.

#ifndef COMMON_H
#define COMMON_H

#include <fstream>
#include <cstdint>

using namespace std;


// Clamps x to the range [mini, maxi].
template <class T> T const& clamp(T const& mini, T const& maxi, T const& x)
{
    if (x < mini) return mini;
    if (x > maxi) return maxi;
    return x;
}

// Reads 2 little-endian bytes.
uint16_t read_16_le(istream&);

// Reads 4 little-endian bytes.
uint32_t read_32_le(istream&);

// Writes 2 little-endian bytes.
void write_16_le(ostream&, uint16_t x);

// Writes 4 little-endian bytes.
void write_32_le(ostream&, uint32_t x);


#endif // COMMON_H
