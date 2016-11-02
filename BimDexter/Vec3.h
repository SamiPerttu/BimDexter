// Vec3.h
// A homegrown 3-vector structure designed to simplify notation in DXT1 block compression.

#ifndef VEC3_H
#define VEC3_H

#include <algorithm>

#include "Common.h"

using namespace std;


// Single precision 3-vector structure. Employs component arithmetic.
struct Vec3 {
  
    float x, y, z;

    Vec3() {}
    Vec3(float c) : x(c), y(c), z(c) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Length squared of the vector.
    float length2() const { return x * x + y * y + z * z; }

    // Length of the vector.
    float length() const { return sqrt(length2()); }

    // Component access by indexing. Do not use in performance sensitive loops.
    float& operator[] (int i) { switch(i) { case 0: return x; case 1: return y; default: return z; } }

    // Sum of the components.
    float sum() const { return x + y + z; }

    // Clamps components to the 8-bit range [0, 255].
    void clamp(const Vec3& minimum, const Vec3& maximum) { x = ::clamp(minimum.x, maximum.x, x); y = ::clamp(minimum.y, maximum.y, y); z = ::clamp(minimum.z, maximum.z, z); }

    // Takes the minimum of each component and returns the result.
    static Vec3 minimize(const Vec3& a, const Vec3& b) { return Vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)); }

    // Takes the maximum of each component and returns the result.
    static Vec3 maximize(const Vec3& a, const Vec3& b) { return Vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)); }

    // Dot product.
    static float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

    // Linear interpolation between a and b. Returns a when t = 0 and b when t = 1.
    static Vec3 lerp(const Vec3& a, const Vec3& b, float t);

}; // struct Vec3


inline Vec3& operator+= (Vec3& a, const Vec3& b) { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
inline Vec3& operator-= (Vec3& a, const Vec3& b) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
inline Vec3& operator*= (Vec3& a, const Vec3& b) { a.x *= b.x; a.y *= b.y; a.z *= b.z; return a; }
inline Vec3& operator/= (Vec3& a, const Vec3& b) { a.x /= b.x; a.y /= b.y; a.z /= b.z; return a; }
inline Vec3& operator/= (Vec3& a, const float& b) { a.x /= b; a.y /= b; a.z /= b; return a; }

inline Vec3 operator+ (const Vec3& a, const Vec3& b) { return Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vec3 operator- (const Vec3& a, const Vec3& b) { return Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vec3 operator* (const Vec3& a, const Vec3& b) { return Vec3(a.x * b.x, a.y * b.y, a.z * b.z); }
inline Vec3 operator* (const Vec3& a, const float& b) { return Vec3(a.x * b, a.y * b, a.z * b); }
inline Vec3 operator/ (const Vec3& a, const Vec3& b) { return Vec3(a.x / b.x, a.y / b.y, a.z / b.z); }
inline Vec3 operator/ (const Vec3& a, const float& b) { return Vec3(a.x / b, a.y / b, a.z / b); }

inline Vec3 Vec3::lerp(const Vec3& a, const Vec3& b, float t) { return a * (1.0f - t) + b * t; }


#endif // VEC3_H
