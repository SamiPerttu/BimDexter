// Common.cpp

#include <fstream>
#include <cstdint>

#include "Common.h"

using namespace std;


uint16_t read_16_le(istream& s)
{
    union {
        char buf[2];
        unsigned char ubuf[2];
    };
    s.read(buf, 2);
    return (uint16_t)ubuf[0] + ((uint16_t)ubuf[1] << 8);
}


uint32_t read_32_le(istream& s)
{
    union {
        char buf[4];
        unsigned char ubuf[4];
    };
    s.read(buf, 4);
    return (uint32_t)ubuf[0] + ((uint32_t)ubuf[1] << 8) + ((uint32_t)ubuf[2] << 16) + ((uint32_t)ubuf[3] << 24);
}


void write_16_le(ostream& s, uint16_t x)
{
    char buf[2];
    buf[0] = (char)x;
    buf[1] = (char)(x >> 8);
    s.write(buf, 2);
}


void write_32_le(ostream& s, uint32_t x)
{
    char buf[4];
    buf[0] = (char)x;
    buf[1] = (char)(x >> 8);
    buf[2] = (char)(x >> 16);
    buf[3] = (char)(x >> 24);
    s.write(buf, 4);
}
