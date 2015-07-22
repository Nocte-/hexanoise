//---------------------------------------------------------------------------
// hexanoise/generator_slowinterpreter.cpp
//
// Copyright 2014-2015, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------

#include "generator_slowinterpreter.hpp"

#define GLM_FORCE_RADIANS

#include <cmath>
#include <stdexcept>
#include <glm/gtx/rotate_vector.hpp>
#include "node.hpp"

#ifndef INTERPRETER_OCTAVES_LIMIT
#define INTERPRETER_OCTAVES_LIMIT 16
#endif

namespace hexa
{
namespace noise
{

namespace
{

const double pi = 3.14159265358979323846;

#define ONE_F1 (1.0)
#define ZERO_F1 (0.0)

// Ken Perlin's permutation table.
const int P_MASK = 255;
const int P_SIZE = 256;
static const int P[512] = {
    151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,
    225, 140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,  23,  190,
    6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203, 117,
    35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136,
    171, 168, 68,  175, 74,  165, 71,  134, 139, 48,  27,  166, 77,  146, 158,
    231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,  55,  46,
    245, 40,  244, 102, 143, 54,  65,  25,  63,  161, 1,   216, 80,  73,  209,
    76,  132, 187, 208, 89,  18,  169, 200, 196, 135, 130, 116, 188, 159, 86,
    164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250, 124, 123, 5,
    202, 38,  147, 118, 126, 255, 82,  85,  212, 207, 206, 59,  227, 47,  16,
    58,  17,  182, 189, 28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,
    154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,  253,
    19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
    228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,  51,
    145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157, 184,
    84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,
    222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156,
    180, 151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233,
    7,   225, 140, 36,  103, 30,  69,  142, 8,   99,  37,  240, 21,  10,  23,
    190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203,
    117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125,
    136, 171, 168, 68,  175, 74,  165, 71,  134, 139, 48,  27,  166, 77,  146,
    158, 231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,  55,
    46,  245, 40,  244, 102, 143, 54,  65,  25,  63,  161, 1,   216, 80,  73,
    209, 76,  132, 187, 208, 89,  18,  169, 200, 196, 135, 130, 116, 188, 159,
    86,  164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250, 124, 123,
    5,   202, 38,  147, 118, 126, 255, 82,  85,  212, 207, 206, 59,  227, 47,
    16,  58,  17,  182, 189, 28,  42,  223, 183, 170, 213, 119, 248, 152, 2,
    44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,
    253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246,
    97,  228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,
    51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157,
    184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205,
    93,  222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,
    156, 180
};

//////////////////////////////////////////////////////////////////////////

const int G_MASK = 15;
const int G_SIZE = 16;
const int G_VECSIZE = 4;
static const double G[16 * 4]
    = {+ONE_F1,  +ONE_F1,  +ZERO_F1, +ZERO_F1, -ONE_F1,  +ONE_F1,  +ZERO_F1,
       +ZERO_F1, +ONE_F1,  -ONE_F1,  +ZERO_F1, +ZERO_F1, -ONE_F1,  -ONE_F1,
       +ZERO_F1, +ZERO_F1, +ONE_F1,  +ZERO_F1, +ONE_F1,  +ZERO_F1, -ONE_F1,
       +ZERO_F1, +ONE_F1,  +ZERO_F1, +ONE_F1,  +ZERO_F1, -ONE_F1,  +ZERO_F1,
       -ONE_F1,  +ZERO_F1, -ONE_F1,  +ZERO_F1, +ZERO_F1, +ONE_F1,  +ONE_F1,
       +ZERO_F1, +ZERO_F1, -ONE_F1,  +ONE_F1,  +ZERO_F1, +ZERO_F1, +ONE_F1,
       -ONE_F1,  +ZERO_F1, +ZERO_F1, -ONE_F1,  -ONE_F1,  +ZERO_F1, +ONE_F1,
       +ONE_F1,  +ZERO_F1, +ZERO_F1, -ONE_F1,  +ONE_F1,  +ZERO_F1, +ZERO_F1,
       +ZERO_F1, -ONE_F1,  +ONE_F1,  +ZERO_F1, +ZERO_F1, -ONE_F1,  -ONE_F1,
       +ZERO_F1};

inline double clamp(double x, double min, double max)
{
    return std::min(std::max(x, min), max);
}

inline double lerp(double x, double a, double b)
{
    return a + x * (b - a);
}

inline double blend5(const double a)
{
    return a * a * a * (a * (a * 6.0 - 15.0) + 10.0);
}

inline double interp_cubic(double v0, double v1, double v2, double v3,
                           double a)
{
    const double x = v3 - v2 - v0 + v1;
    const double a2 = a * a;
    const double a3 = a2 * a;
    return x * a3 + (v0 - v1 - x) * a2 + (v2 - v0) * a + v1;
}

inline glm::dvec2 lerp2d(const double x, const glm::dvec2& a,
                         const glm::dvec2& b)
{
    return a + x * (b - a);
}

inline glm::dvec4 lerp4d(const double x, const glm::dvec4& a,
                         const glm::dvec4& b)
{
    return a + x * (b - a);
}

inline double dot(const double* p, double x, double y)
{
    return p[0] * x + p[1] * y;
}

inline double dot(const double* p, double x, double y, double z)
{
    return p[0] * x + p[1] * y + p[2] * z;
}

inline uint32_t hash(uint32_t x, uint32_t y)
{
    return ((uint32_t)x * 2120969693) ^ ((uint32_t)y * 915488749) ^ ((uint32_t)(x + 1103515245) * (uint32_t)(y + 1234567));
}

inline uint32_t hash(uint32_t x, uint32_t y, uint32_t z)
{
    return ((uint32_t)x * 2120969693)
            ^ ((uint32_t)y * 915488749)
            ^ ((uint32_t)z * 22695477)
            ^ ((uint32_t)(x + 1103515245) * (uint32_t)(y + 1234567) * (uint32_t)(z + 134775813));
}

inline uint32_t rng(uint32_t last)
{
    return (1103515245 * last + 12345) & 0x7FFFFFFF;
}

//////////////////////////////////////////////////////////////////////////
// Perlin

inline double gradient_noise2d(const glm::dvec2& xy, glm::ivec2 ixy,
                               uint32_t seed)
{
    ixy.x += seed * 1013;
    ixy.y += seed * 1619;
    ixy &= P_MASK;

    int index = (P[ixy.x + P[ixy.y]] & G_MASK) * G_VECSIZE;
    glm::dvec2 g{G[index], G[index + 1]};

    return glm::dot(xy, g);
}

double p_perlin(const glm::dvec2& xy, uint32_t seed)
{
    glm::dvec2 t{glm::floor(xy)};
    glm::ivec2 xy0{(int)t.x, (int)t.y};
    glm::dvec2 xyf{xy - t};

    const glm::ivec2 I01{0, 1};
    const glm::ivec2 I10{1, 0};
    const glm::ivec2 I11{1, 1};

    const glm::dvec2 F01{0.0, 1.0};
    const glm::dvec2 F10{1.0, 0.0};
    const glm::dvec2 F11{1.0, 1.0};

    const double n00 = gradient_noise2d(xyf, xy0, seed);
    const double n10 = gradient_noise2d(xyf - F10, xy0 + I10, seed);
    const double n01 = gradient_noise2d(xyf - F01, xy0 + I01, seed);
    const double n11 = gradient_noise2d(xyf - F11, xy0 + I11, seed);

    const glm::dvec2 n0001{n00, n01};
    const glm::dvec2 n1011{n10, n11};
    const glm::dvec2 n2 = lerp2d(blend5(xyf.x), n0001, n1011);

    return lerp(blend5(xyf.y), n2.x, n2.y) * 1.227;
}

inline double gradient_noise3d(glm::ivec3 ixyz, const glm::dvec3& xyz,
                               uint32_t seed)
{
    ixyz.x += seed * 1013;
    ixyz.y += seed * 1619;
    ixyz.z += seed * 997;
    ixyz &= P_MASK;

    int index = (P[ixyz.x + P[ixyz.y + P[ixyz.z]]] & G_MASK) * G_VECSIZE;
    glm::dvec3 g{G[index], G[index + 1], G[index + 2]};

    return glm::dot(xyz, g);
}

double p_perlin3(const glm::dvec3& xyz, uint32_t seed)
{
    glm::dvec3 t {glm::floor(xyz)};
    glm::ivec3 xyz0 {(int)t.x, (int)t.y, (int)t.z};
    glm::dvec3 xyzf {xyz - t};

    const glm::ivec3 I001 {0, 0, 1};
    const glm::ivec3 I010 {0, 1, 0};
    const glm::ivec3 I011 {0, 1, 1};
    const glm::ivec3 I100 {1, 0, 0};
    const glm::ivec3 I101 {1, 0, 1};
    const glm::ivec3 I110 {1, 1, 0};
    const glm::ivec3 I111 {1, 1, 1};

    const glm::dvec3 F001 {0.0, 0.0, 1.0};
    const glm::dvec3 F010 {0.0, 1.0, 0.0};
    const glm::dvec3 F011 {0.0, 1.0, 1.0};
    const glm::dvec3 F100 {1.0, 0.0, 0.0};
    const glm::dvec3 F101 {1.0, 0.0, 1.0};
    const glm::dvec3 F110 {1.0, 1.0, 0.0};
    const glm::dvec3 F111 {1.0, 1.0, 1.0};

    const double n000 = gradient_noise3d(xyz0       , xyzf       , seed);
    const double n001 = gradient_noise3d(xyz0 + I001, xyzf - F001, seed);
    const double n010 = gradient_noise3d(xyz0 + I010, xyzf - F010, seed);
    const double n011 = gradient_noise3d(xyz0 + I011, xyzf - F011, seed);
    const double n100 = gradient_noise3d(xyz0 + I100, xyzf - F100, seed);
    const double n101 = gradient_noise3d(xyz0 + I101, xyzf - F101, seed);
    const double n110 = gradient_noise3d(xyz0 + I110, xyzf - F110, seed);
    const double n111 = gradient_noise3d(xyz0 + I111, xyzf - F111, seed);

    glm::dvec4 n40 {n000, n001, n010, n011};
    glm::dvec4 n41 {n100, n101, n110, n111};

    auto n4 = lerp4d(blend5(xyzf.x), n40, n41);
    auto n2 = lerp2d(blend5(xyzf.y), {n4.x, n4.y}, {n4.z, n4.w});
    auto n1 = lerp(blend5(xyzf.z), n2.x, n2.y);

    return n1 * 1.216;
}

//////////////////////////////////////////////////////////////////////////
// Simplex

double p_simplex(const glm::dvec2& xy, uint32_t seed)
{
    double n0, n1, n2;

    // Skew the input space to determine which simplex cell we're in
    const double F2 = 0.5 * (std::sqrt(3.0) - 1.0);
    const double G2 = (3.0 - std::sqrt(3.0)) / 6.0;

    double s = (xy.x + xy.y) * F2;
    int i = std::floor(xy.x + s);
    int j = std::floor(xy.y + s);

    // Unskew the cell origin back to (x,y) space
    double t = (i + j) * G2;
    double X0 = i - t;
    double Y0 = j - t;

    // The x,y distances from the cell origin
    double x0 = xy.x - X0;
    double y0 = xy.y - Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner in (i,j) coords
    if (x0 > y0) {
        i1 = 1; // lower triangle, XY order: (0,0)->(1,0)->(1,1)
        j1 = 0;
    } else {
        i1 = 0; // upper triangle, YX order: (0,0)->(0,1)->(1,1)
        j1 = 1;
    }

    double x1 = x0 - i1 + G2;
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2;
    double y2 = y0 - 1.0 + 2.0 * G2;

    int ii = (i + seed * 1063) & 0xFF;
    int jj = j & 0xFF;
    int gi0 = P[ii + P[jj]] & G_MASK;
    int gi1 = P[ii + i1 + P[jj + j1]] & G_MASK;
    int gi2 = P[ii + 1 + P[jj + 1]] & G_MASK;

    double t0 = 0.5 - x0 * x0 - y0 * y0;
    if (t0 < 0) {
        n0 = 0.0;
    } else {
        t0 *= t0;
        n0 = t0 * t0 * dot(&G[gi0 * G_VECSIZE], x0, y0);
    }

    double t1 = 0.5 - x1 * x1 - y1 * y1;
    if (t1 < 0) {
        n1 = 0.0;
    } else {
        t1 *= t1;
        n1 = t1 * t1 * dot(&G[gi1 * G_VECSIZE], x1, y1);
    }

    double t2 = 0.5 - x2 * x2 - y2 * y2;
    if (t2 < 0) {
        n2 = 0.0;
    } else {
        t2 *= t2;
        n2 = t2 * t2 * dot(&G[gi2 * G_VECSIZE], x2, y2);
    }

    return 70.0 * (n0 + n1 + n2);
}

double p_simplex3(const glm::dvec3& p, uint32_t seed)
{
    // Skew the input space to determine which simplex cell we're in
    const double F3 = 1.0 / 3.0;
    double s = (p.x + p.y + p.z) * F3;
    int i = std::floor(p.x + s);
    int j = std::floor(p.y + s);
    int k = std::floor(p.z + s);

    // Unskew the cell origin back to (x,y) space
    const double G3 = 1.0 / 6.0;
    double t = (i + j + k) * G3;
    double X0 = i - t;
    double Y0 = j - t;
    double Z0 = k - t;

    // The x,y distances from the cell origin
    double x0 = p.x - X0;
    double y0 = p.y - Y0;
    double z0 = p.z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner

    if (x0 >= y0) {
        if (y0 >= z0) {
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 1;
            k2 = 0;
        } else if (x0 >= z0) {
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 0;
            k2 = 1;
        } else {
            i1 = 0;
            j1 = 0;
            k1 = 1;
            i2 = 1;
            j2 = 0;
            k2 = 1;
        }
    } else { // x0 < y0
        if (y0 < z0) {
            i1 = 0;
            j1 = 0;
            k1 = 1;
            i2 = 0;
            j2 = 1;
            k2 = 1;
        } else if (x0 < z0) {
            i1 = 0;
            j1 = 1;
            k1 = 0;
            i2 = 0;
            j2 = 1;
            k2 = 1;
        } else {
            i1 = 0;
            j1 = 1;
            k1 = 0;
            i2 = 1;
            j2 = 1;
            k2 = 0;
        }
    }

    double x1 = x0 - i1 + G3;
    double y1 = y0 - j1 + G3;
    double z1 = z0 - k1 + G3;
    double x2 = x0 - i2 + 2.0 * G3;
    double y2 = y0 - j2 + 2.0 * G3;
    double z2 = z0 - k2 + 2.0 * G3;
    double x3 = x0 - 1.0 + 3.0 * G3;
    double y3 = y0 - 1.0 + 3.0 * G3;
    double z3 = z0 - 1.0 + 3.0 * G3;

    int ii = (i + seed * 1063) & 0xFF;
    int jj = j & 0xFF;
    int kk = k & 0xFF;

    int gi0 = P[ii + P[jj + P[kk]]] & G_MASK;
    int gi1 = P[ii + i1 + P[jj + j1 + P[kk + k1]]] & G_MASK;
    int gi2 = P[ii + i2 + P[jj + j2 + P[kk + k2]]] & G_MASK;
    int gi3 = P[ii + 1 + P[jj + 1 + P[kk + 1]]] & G_MASK;

    // Calculate the contribution from the four corners
    double n0, n1, n2, n3;

    double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 < 0) {
        n0 = 0.0;
    } else {
        n0 = std::pow(t0, 4) * dot(&G[gi0 * G_VECSIZE], x0, y0, z0);
    }

    double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0) {
        n1 = 0.0;
    } else {
        n1 = std::pow(t1, 4) * dot(&G[gi1 * G_VECSIZE], x1, y1, z1);
    }

    double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0) {
        n2 = 0.0;
    } else {
        n2 = std::pow(t2, 4) * dot(&G[gi2 * G_VECSIZE], x2, y2, z2);
    }

    double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0) {
        n3 = 0.0;
    } else {
        n3 = std::pow(t3, 4) * dot(&G[gi3 * G_VECSIZE], x3, y3, z3);
    }

    return 32.0 * (n0 + n1 + n2 + n3);
}

//////////////////////////////////////////////////////////////////////////
// OpenSimplex

// Gradients for 2D. They approximate the directions to the
// vertices of an octagon from the center.
static const int8_t gradients2D[] = {
    5, 2, 2, 5,
    -5, 2, -2, 5,
    5, -2, 2, -5,
    -5, -2, -2, -5
};

inline double extrapolate2(int xsb, int ysb, const glm::dvec2& d, uint32_t seed)
{
    int index = P[(P[(xsb + seed) & 0xFF] + (ysb + seed * 23)) & 0xFF] & 0x0E;
    return gradients2D[index] * d.x + gradients2D[index + 1] * d.y;
}

inline double attn (const glm::dvec2& p)
{
    return 2.0 - glm::dot(p, p);
}

// Implementation of the OpenSimplex algorithm by Kurt Spencer.
double p_opensimplex(const glm::dvec2& p, uint32_t seed)
{
    constexpr double STRETCH_CONSTANT_2D = -0.211324865405187; // (1 / sqrt(2 + 1) - 1 ) / 2;
    constexpr double SQUISH_CONSTANT_2D = 0.366025403784439; // (sqrt(2 + 1) -1) / 2;
    constexpr double NORM_CONSTANT_2D = 47.0;

    // Place input coordinates onto grid.
    double stretchOffset = (p.x + p.y) * STRETCH_CONSTANT_2D;
    glm::dvec2 s {p + stretchOffset};

    // Floor to get grid coordinates of rhombus (stretched square) super-cell origin.
    glm::ivec2 sb {glm::floor(s)};

    // Skew out to get actual coordinates of rhombus origin. We'll need these later.
    double squishOffset = (sb.x + sb.y) * SQUISH_CONSTANT_2D;
    glm::dvec2 b {glm::dvec2{sb} + squishOffset};

    // Compute grid coordinates relative to rhombus origin.
    glm::dvec2 ins {s - glm::dvec2{sb}};

    // Sum those together to get a value that determines which region we're in.
    double inSum = ins.x + ins.y;

    // Positions relative to origin point.
    glm::dvec2 d0 {p - b};

    // We'll be defining these inside the next block and using them afterwards.
    glm::dvec2 d_ext;
    glm::ivec2 sv_ext;
    double value = 0;

    // Contribution (1,0)
    glm::dvec2 d1 {(d0 + glm::dvec2{-1,0}) - SQUISH_CONSTANT_2D};
    double attn1 = attn(d1);

    if (attn1 > 0) {
        attn1 *= attn1;
        value += attn1 * attn1 * extrapolate2(sb.x + 1, sb.y + 0, d1, seed);
    }

    // Contribution (0,1)
    glm::dvec2 d2 {(d0 + glm::dvec2(0,-1)) - SQUISH_CONSTANT_2D};
    double attn2 = attn(d2);
    if (attn2 > 0) {
        attn2 *= attn2;
        value += attn2 * attn2 * extrapolate2(sb.x + 0, sb.y + 1, d2, seed);
    }

    if (inSum <= 1) { // We're inside the triangle (2-Simplex) at (0,0)
        double zins = 1 - inSum;
        if (zins > ins.x || zins > ins.y) { // (0,0) is one of the closest two triangular vertices
            if (ins.x > ins.y) {
                sv_ext = sb + glm::ivec2{1, -1};
                d_ext = d0 + glm::dvec2{-1, 1};
            } else {
                sv_ext = sb + glm::ivec2{-1, 1};
                d_ext = d0 + glm::dvec2{1, -1};
            }
        } else { // (1,0) and (0,1) are the closest two vertices.
            sv_ext = sb + glm::ivec2{1, 1};
            d_ext = (d0 + glm::dvec2{-1, -1}) - 2 * SQUISH_CONSTANT_2D;
        }
    } else { // We're inside the triangle (2-Simplex) at (1,1)
        double zins = 2 - inSum;
        if (zins < ins.x || zins < ins.y) { // (0,0) is one of the closest two triangular vertices
            if (ins.x > ins.y) {
                sv_ext = sb + glm::ivec2{2,0};
                d_ext = (d0 + glm::dvec2{-2, 0}) - 2 * SQUISH_CONSTANT_2D;
            } else {
                sv_ext = sb + glm::ivec2{0, 2};
                d_ext = (d0 + glm::dvec2{0, -2}) - 2 * SQUISH_CONSTANT_2D;
            }
        } else { // (1,0) and (0,1) are the closest two vertices.
            d_ext = d0;
            sv_ext = sb;
        }
        sb += 1;
        d0 = d0 - 1.0 - 2 * SQUISH_CONSTANT_2D;
    }

    // Contribution (0,0) or (1,1)
    double attn0 = attn(d0);
    if (attn0 > 0)
        value += std::pow(attn0, 4) * extrapolate2(sb.x, sb.y, d0, seed);

    // Extra Vertex
    double attn_ext = attn(d_ext);
    if (attn_ext > 0)
        value += std::pow(attn_ext, 4) * extrapolate2(sv_ext.x, sv_ext.y, d_ext, seed);

    return value / NORM_CONSTANT_2D;
}


// Gradients for 3D. They approximate the directions to the
// vertices of a rhombicuboctahedron from the center, skewed so
// that the triangular and square facets can be inscribed inside
// circles of the same radius.
static const glm::dvec3 gradients3D[] = {
    glm::dvec3(-11., 4., 4.),   glm::dvec3(-4., 11., 4.),   glm::dvec3(-4., 4., 11.),
    glm::dvec3(11., 4., 4.),    glm::dvec3(4., 11., 4.),    glm::dvec3(4., 4., 11.),
    glm::dvec3(-11., -4., 4.),  glm::dvec3(-4., -11., 4.),  glm::dvec3(-4., -4., 11.),
    glm::dvec3(11., -4., 4.),   glm::dvec3(4., -11., 4.),   glm::dvec3(4., -4., 11.),
    glm::dvec3(-11., 4., -4.),  glm::dvec3(-4., 11., -4.),  glm::dvec3(-4., 4., -11.),
    glm::dvec3(11., 4., -4.),   glm::dvec3(4., 11., -4.),   glm::dvec3(4., 4., -11.),
    glm::dvec3(-11., -4., -4.), glm::dvec3(-4., -11., -4.), glm::dvec3(-4., -4., -11.),
    glm::dvec3(11., -4., -4.),  glm::dvec3(4., -11., -4.),  glm::dvec3(4., -4., -11.)
};

inline double extrapolate3(int xsb, int ysb, int zsb, const glm::dvec3& d, uint32_t seed)
{
    int index = P[(P[(P[(xsb + seed) & 0xFF] + (ysb + seed * 23)) & 0xFF] + (zsb + seed * 27)) & 0xFF] % 24;
    return glm::dot(gradients3D[index], d);
}

inline double attn (const glm::dvec3& p)
{
    return 2.0 - glm::dot(p, p);
}

double p_opensimplex3(const glm::dvec3& p, uint32_t seed)
{
    constexpr double STRETCH_CONSTANT_3D = -1.0 / 6.0; // (1 / sqrt(3 + 1) - 1) / 3;
    constexpr double SQUISH_CONSTANT_3D = 1.0 / 3.0; // (sqrt(3+1)-1)/3;
    constexpr double NORM_CONSTANT_3D = 103.0;

    // Place input coordinates on simplectic honeycomb.
    double stretchOffset = (p.x + p.y + p.z) * STRETCH_CONSTANT_3D;
    glm::dvec3 s {p + stretchOffset};

    // Floor to get grid coordinates of rhombohedron (stretched cube) super-cell origin.
    glm::ivec3 sb {glm::floor(s)};

    // Skew out to get actual coordinates of rhombohedron origin. We'll need these later.
    double squishOffset = (sb.x + sb.y + sb.z) * SQUISH_CONSTANT_3D;
    glm::dvec3 b {glm::dvec3{sb} + squishOffset};

    // Compute grid coordinates relative to rhombus origin.
    glm::dvec3 ins {s - glm::dvec3{sb}};

    // Sum those together to get a value that determines which region we're in.
    double inSum = ins.x + ins.y + ins.z;

    // Positions relative to origin point.
    glm::dvec3 d0 = p - b;

    // We'll be defining these inside the next block and using them afterwards.
    glm::dvec3 d_ext0, d_ext1;
    glm::ivec3 sv_ext0, sv_ext1;
    double value = 0;

    if (inSum <= 1) { // We're inside the tetrahedron (3-Simplex) at (0,0,0)
        // Determine which two of (0,0,1), (0,1,0), (1,0,0) are closest.
        uint8_t aPoint = 0x01;
        double aScore = ins.x;
        uint8_t bPoint = 0x02;
        double bScore = ins.y;
        if (aScore >= bScore && ins.z > bScore) {
            bScore = ins.z;
            bPoint = 0x04;
        } else if (aScore < bScore && ins.z > aScore) {
            aScore = ins.z;
            aPoint = 0x04;
        }

        // Now we determine the two lattice points not part of the tetrahedron that may contribute.
        // This depends on the closest two tetrahedral vertices, including (0,0,0)
        double wins = 1 - inSum;
        if (wins > aScore || wins > bScore) { // (0,0,0) is one of the closest two tetrahedral vertices.
            uint8_t c = (bScore > aScore ? bPoint : aPoint); // Our other closest vertex is the closest out of a and b.
            if ((c & 0x01) == 0) {
                sv_ext0.x = sb.x - 1;
                sv_ext1.x = sb.x;
                d_ext0.x = d0.x + 1;
                d_ext1.x = d0.x;
            } else {
                sv_ext0.x = sv_ext1.x = sb.x + 1;
                d_ext0.x = d_ext1.x = d0.x - 1;
            }

            if ((c & 0x02) == 0) {
                sv_ext0.y = sv_ext1.y = sb.y;
                d_ext0.y = d_ext1.y = d0.y;
                if ((c & 0x01) == 0) {
                    sv_ext1.y -= 1;
                    d_ext1.y += 1;
                } else {
                    sv_ext0.y -= 1;
                    d_ext0.y += 1;
                }
            } else {
                sv_ext0.y = sv_ext1.y = sb.y + 1;
                d_ext0.y = d_ext1.y = d0.y - 1;
            }

            if ((c & 0x04) == 0) {
                sv_ext0.z = sb.z;
                sv_ext1.z = sb.z - 1;
                d_ext0.z = d0.z;
                d_ext1.z = d0.z + 1;
            } else {
                sv_ext0.z = sv_ext1.z = sb.z + 1;
                d_ext0.z = d_ext1.z = d0.z - 1;
            }
        } else { // (0,0,0) is not one of the closest two tetrahedral vertices.
            uint8_t c = (aPoint | bPoint); // Our two extra vertices are determined by the closest two.
            if ((c & 0x01) == 0) {
                sv_ext0.x = sb.x;
                sv_ext1.x = sb.x - 1;
                d_ext0.x = d0.x - 2 * SQUISH_CONSTANT_3D;
                d_ext1.x = d0.x + 1 - SQUISH_CONSTANT_3D;
            } else {
                sv_ext0.x = sv_ext1.x = sb.x + 1;
                d_ext0.x = d0.x - 1 - 2 * SQUISH_CONSTANT_3D;
                d_ext1.x = d0.x - 1 - SQUISH_CONSTANT_3D;
            }

            if ((c & 0x02) == 0) {
                sv_ext0.y = sb.y;
                sv_ext1.y = sb.y - 1;
                d_ext0.y = d0.y - 2 * SQUISH_CONSTANT_3D;
                d_ext1.y = d0.y + 1 - SQUISH_CONSTANT_3D;
            } else {
                sv_ext0.y = sv_ext1.y = sb.y + 1;
                d_ext0.y = d0.y - 1 - 2 * SQUISH_CONSTANT_3D;
                d_ext1.y = d0.y - 1 - SQUISH_CONSTANT_3D;
            }

            if ((c & 0x04) == 0) {
                sv_ext0.z = sb.z;
                sv_ext1.z = sb.z - 1;
                d_ext0.z = d0.z - 2 * SQUISH_CONSTANT_3D;
                d_ext1.z = d0.z + 1 - SQUISH_CONSTANT_3D;
            } else {
                sv_ext0.z = sv_ext1.z = sb.z + 1;
                d_ext0.z = d0.z - 1 - 2 * SQUISH_CONSTANT_3D;
                d_ext1.z = d0.z - 1 - SQUISH_CONSTANT_3D;
            }
        }

        // Contribution (0,0,0)
        double attn0 = attn(d0);
        if (attn0 > 0)
            value += std::pow(attn0, 4) * extrapolate3(sb.x + 0, sb.y + 0, sb.z + 0, d0, seed);

        // Contribution (1,0,0)
        glm::dvec3 d1 = (d0 + glm::dvec3{-1,0,0}) - SQUISH_CONSTANT_3D;
        double attn1 = attn(d1);
        if (attn1 > 0)
            value += std::pow(attn1, 4) * extrapolate3(sb.x + 1, sb.y + 0, sb.z + 0, d1, seed);

        // Contribution (0,1,0)
        glm::dvec3 d2  {d0.x - SQUISH_CONSTANT_3D, d0.y - 1 - SQUISH_CONSTANT_3D, d1.z};
        double attn2 = attn(d2);
        if (attn2 > 0)
            value += std::pow(attn2, 4) * extrapolate3(sb.x + 0, sb.y + 1, sb.z + 0, d2, seed);

        // Contribution (0,0,1)
        glm::dvec3 d3 {d2.x, d1.y, d0.z - 1 - SQUISH_CONSTANT_3D};
        double attn3 = attn(d3);
        if (attn3 > 0)
            value += std::pow(attn3, 4) * extrapolate3(sb.x + 0, sb.y + 0, sb.z + 1, d3, seed);

    } else if (inSum >= 2) { // We're inside the tetrahedron (3-Simplex) at (1,1,1)

        // Determine which two tetrahedral vertices are the closest, out of (1,1,0), (1,0,1), (0,1,1) but not (1,1,1).
        uint8_t aPoint = 0x06;
        double aScore = ins.x;
        uint8_t bPoint = 0x05;
        double bScore = ins.y;
        if (aScore <= bScore && ins.z < bScore) {
            bScore = ins.z;
            bPoint = 0x03;
        } else if (aScore > bScore && ins.z < aScore) {
            aScore = ins.z;
            aPoint = 0x03;
        }

        // Now we determine the two lattice points not part of the tetrahedron that may contribute.
        // This depends on the closest two tetrahedral vertices, including (1,1,1)
        double wins = 3 - inSum;
        if (wins < aScore || wins < bScore) { // (1,1,1) is one of the closest two tetrahedral vertices.
            uint8_t c = (bScore < aScore ? bPoint : aPoint); // Our other closest vertex is the closest out of a and b.
            if ((c & 0x01) != 0) {
                sv_ext0.x = sb.x + 2;
                sv_ext1.x = sb.x + 1;
                d_ext0.x = d0.x - 2 - 3 * SQUISH_CONSTANT_3D;
                d_ext1.x = d0.x - 1 - 3 * SQUISH_CONSTANT_3D;
            } else {
                sv_ext0.x = sv_ext1.x = sb.x;
                d_ext0.x = d_ext1.x = d0.x - 3 * SQUISH_CONSTANT_3D;
            }

            if ((c & 0x02) != 0) {
                sv_ext0.y = sv_ext1.y = sb.y + 1;
                d_ext0.y = d_ext1.y = d0.y - 1 - 3 * SQUISH_CONSTANT_3D;
                if ((c & 0x01) != 0) {
                    sv_ext1.y += 1;
                    d_ext1.y -= 1;
                } else {
                    sv_ext0.y += 1;
                    d_ext0.y -= 1;
                }
            } else {
                sv_ext0.y = sv_ext1.y = sb.y;
                d_ext0.y = d_ext1.y = d0.y - 3 * SQUISH_CONSTANT_3D;
            }

            if ((c & 0x04) != 0) {
                sv_ext0.z = sb.z + 1;
                sv_ext1.z = sb.z + 2;
                d_ext0.z = d0.z - 1 - 3 * SQUISH_CONSTANT_3D;
                d_ext1.z = d0.z - 2 - 3 * SQUISH_CONSTANT_3D;
            } else {
                sv_ext0.z = sv_ext1.z = sb.z;
                d_ext0.z = d_ext1.z = d0.z - 3 * SQUISH_CONSTANT_3D;
            }
        } else { // (1,1,1) is not one of the closest two tetrahedral vertices.

            uint8_t c = (aPoint & bPoint); // Our two extra vertices are determined by the closest two.
            if ((c & 0x01) != 0) {
                sv_ext0.x = sb.x + 1;
                sv_ext1.x = sb.x + 2;
                d_ext0.x = d0.x - 1 - SQUISH_CONSTANT_3D;
                d_ext1.x = d0.x - 2 - 2 * SQUISH_CONSTANT_3D;
            } else {
                sv_ext0.x = sv_ext1.x = sb.x;
                d_ext0.x = d0.x - SQUISH_CONSTANT_3D;
                d_ext1.x = d0.x - 2 * SQUISH_CONSTANT_3D;
            }

            if ((c & 0x02) != 0) {
                sv_ext0.y = sb.y + 1;
                sv_ext1.y = sb.y + 2;
                d_ext0.y = d0.y - 1 - SQUISH_CONSTANT_3D;
                d_ext1.y = d0.y - 2 - 2 * SQUISH_CONSTANT_3D;
            } else {
                sv_ext0.y = sv_ext1.y = sb.y;
                d_ext0.y = d0.y - SQUISH_CONSTANT_3D;
                d_ext1.y = d0.y - 2 * SQUISH_CONSTANT_3D;
            }

            if ((c & 0x04) != 0) {
                sv_ext0.z = sb.z + 1;
                sv_ext1.z = sb.z + 2;
                d_ext0.z = d0.z - 1 - SQUISH_CONSTANT_3D;
                d_ext1.z = d0.z - 2 - 2 * SQUISH_CONSTANT_3D;
            } else {
                sv_ext0.z = sv_ext1.z = sb.z;
                d_ext0.z = d0.z - SQUISH_CONSTANT_3D;
                d_ext1.z = d0.z - 2 * SQUISH_CONSTANT_3D;
            }
        }

        // Contribution (1,1,0)
        glm::dvec3 d3 = (d0 + glm::dvec3{-1,-1,0}) - 2 * SQUISH_CONSTANT_3D;
        double attn3 = attn(d3);
        if (attn3 > 0)
            value += std::pow(attn3,4) * extrapolate3(sb.x + 1, sb.y + 1, sb.z + 0, d3, seed);

        // Contribution (1,0,1)
        glm::dvec3 d2 {d3.x, d0.y - 0 - 2 * SQUISH_CONSTANT_3D, d0.z - 1 - 2 * SQUISH_CONSTANT_3D};
        double attn2 = attn(d2);
        if (attn2 > 0)
            value += std::pow(attn2, 4) * extrapolate3(sb.x + 1, sb.y + 0, sb.z + 1, d2, seed);

        // Contribution (0,1,1)
        glm::dvec3 d1 {d0.x - 0 - 2 * SQUISH_CONSTANT_3D, d3.y, d2.z};
        double attn1 = attn(d1);
        if (attn1 > 0)
            value += std::pow(attn1, 4) * extrapolate3(sb.x + 0, sb.y + 1, sb.z + 1, d1, seed);

        // Contribution (1,1,1)
        d0 -= 1 + 3 * SQUISH_CONSTANT_3D;
        double attn0 = attn(d0);
        if (attn0 > 0)
            value += std::pow(attn0, 4) * extrapolate3(sb.x + 1, sb.y + 1, sb.z + 1, d0, seed);

    } else { // We're inside the octahedron (Rectified 3-Simplex) in between.

        double aScore;
        uint8_t aPoint;
        bool aIsFurtherSide;
        double bScore;
        uint8_t bPoint;
        bool bIsFurtherSide;

        // Decide between point (0,0,1) and (1,1,0) as closest
        double p1 = ins.x + ins.y;
        if (p1 > 1) {
            aScore = p1 - 1;
            aPoint = 0x03;
            aIsFurtherSide = true;
        } else {
            aScore = 1 - p1;
            aPoint = 0x04;
            aIsFurtherSide = false;
        }

        // Decide between point (0,1,0) and (1,0,1) as closest
        double p2 = ins.x + ins.z;
        if (p2 > 1) {
            bScore = p2 - 1;
            bPoint = 0x05;
            bIsFurtherSide = true;
        } else {
            bScore = 1 - p2;
            bPoint = 0x02;
            bIsFurtherSide = false;
        }

        // The closest out of the two (1,0,0) and (0,1,1) will replace the furthest out of the two decided above, if closer.
        double p3 = ins.y + ins.z;
        if (p3 > 1) {
            double score = p3 - 1;
            if (aScore <= bScore && aScore < score) {
                aScore = score;
                aPoint = 0x06;
                aIsFurtherSide = true;
            } else if (aScore > bScore && bScore < score) {
                bScore = score;
                bPoint = 0x06;
                bIsFurtherSide = true;
            }
        } else {
            double score = 1 - p3;
            if (aScore <= bScore && aScore < score) {
                aScore = score;
                aPoint = 0x01;
                aIsFurtherSide = false;
            } else if (aScore > bScore && bScore < score) {
                bScore = score;
                bPoint = 0x01;
                bIsFurtherSide = false;
            }
        }

        // Where each of the two closest points are determines how the extra two vertices are calculated.
        if (aIsFurtherSide == bIsFurtherSide) {
            if (aIsFurtherSide) { // Both closest points on (1,1,1) side

                // One of the two extra points is (1,1,1)
                d_ext0 = d0 - 1.0 - 3 * SQUISH_CONSTANT_3D;
                sv_ext0 = sb + 1;

                // Other extra point is based on the shared axis.
                uint8_t c = (aPoint & bPoint);
                if ((c & 0x01) != 0) {
                    d_ext1 = d0 + glm::dvec3{-2,0,0} - 2 * SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + glm::ivec3{2,0,0};
                } else if ((c & 0x02) != 0) {
                    d_ext1 = d0 + glm::dvec3{0,-2,0} - 2 * SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + glm::ivec3{0,2,0};
                } else {
                    d_ext1 = d0 + glm::dvec3{0,0,-2} - 2 * SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + glm::ivec3{0,0,2};
                }
            } else { // Both closest points on (0,0,0) side
                // One of the two extra points is (0,0,0)
                d_ext0 = d0;
                sv_ext0 = sb;

                // Other extra point is based on the omitted axis.
                uint8_t c = (aPoint | bPoint);
                if ((c & 0x01) == 0) {
                    d_ext1 = d0 + glm::dvec3{1,-1,-1} - SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + glm::ivec3{-1,1,1};
                } else if ((c & 0x02) == 0) {
                    d_ext1 = d0 + glm::dvec3{-1,1,-1} - SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + glm::ivec3{1,-1,1};
                } else {
                    d_ext1 = d0 + glm::dvec3{-1,-1,1} - SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + glm::ivec3{1,1,-1};
                }
            }
        } else { // One point on (0,0,0) side, one point on (1,1,1) side
            uint8_t c1, c2;
            if (aIsFurtherSide) {
                c1 = aPoint;
                c2 = bPoint;
            } else {
                c1 = bPoint;
                c2 = aPoint;
            }
            // One contribution is a permutation of (1,1,-1)
            if ((c1 & 0x01) == 0) {
                d_ext0 = d0 + glm::dvec3{1,-1,-1} - SQUISH_CONSTANT_3D;
                sv_ext0 = sb + glm::ivec3{-1,1,1};
            } else if ((c1 & 0x02) == 0) {
                d_ext0 = d0 + glm::dvec3{-1,1,-1} - SQUISH_CONSTANT_3D;
                sv_ext0 = sb + glm::ivec3{1,-1,1};
            } else {
                d_ext0 = d0 + glm::dvec3{-1,-1,1} - SQUISH_CONSTANT_3D;
                sv_ext0 = sb + glm::ivec3{1,1,-1};
            }

            // One contribution is a permutation of (0,0,2)
            d_ext1 = d0 - 2 * SQUISH_CONSTANT_3D;
            sv_ext1 = sb;
            if ((c2 & 0x01) != 0) {
                d_ext1.x -= 2;
                sv_ext1.x += 2;
            } else if ((c2 & 0x02) != 0) {
                d_ext1.y -= 2;
                sv_ext1.y += 2;
            } else {
                d_ext1.z -= 2;
                sv_ext1.z += 2;
            }
        }

        // Contribution (1,0,0)
        glm::dvec3 d1 = (d0 + glm::dvec3{-1,0,0}) - SQUISH_CONSTANT_3D;
        double attn1 = attn(d1);
        if (attn1 > 0)
            value += std::pow(attn1, 4) * extrapolate3(sb.x + 1, sb.y + 0, sb.z + 0, d1, seed);

        // Contribution (0,1,0)
        glm::dvec3 d2 {d0.x - SQUISH_CONSTANT_3D, d0.y - 1 - SQUISH_CONSTANT_3D, d1.z};
        double attn2 = attn(d2);
        if (attn2 > 0)
            value += std::pow(attn2, 4)* extrapolate3(sb.x + 0, sb.y + 1, sb.z + 0, d2, seed);


        // Contribution (0,0,1)
        glm::dvec3 d3 {d2.x, d1.y, d0.z - 1 - SQUISH_CONSTANT_3D};
        double attn3 = attn(d3);
        if (attn3 > 0)
            value += std::pow(attn3, 4)* extrapolate3(sb.x + 0, sb.y + 0, sb.z + 1, d3, seed);

        // Contribution (1,1,0)
        glm::dvec3 d4 = d0 - glm::dvec3{1,1,0} - 2 * SQUISH_CONSTANT_3D;
        double attn4 = attn(d4);
        if (attn4 > 0)
            value += std::pow(attn4, 4)* extrapolate3(sb.x + 1, sb.y + 1, sb.z + 0, d4, seed);

        // Contribution (1,0,1)
        glm::dvec3 d5 {d4.x, d0.y - 2 * SQUISH_CONSTANT_3D, d0.z - 1 - 2 * SQUISH_CONSTANT_3D};
        double attn5 = attn(d5);
        if (attn5 > 0)
            value += std::pow(attn5, 4)* extrapolate3(sb.x + 1, sb.y + 0, sb.z + 1, d5, seed);

        // Contribution (0,1,1)
        glm::dvec3 d6 {d0.x - 2 * SQUISH_CONSTANT_3D, d4.y, d5.z};
        double attn6 = attn(d6);
        if (attn6 > 0)
            value += std::pow(attn6, 4) * extrapolate3(sb.x + 0, sb.y + 1, sb.z + 1, d6, seed);
    }
    // First extra vertex
    double attn_ext0 = attn(d_ext0);
    if (attn_ext0 > 0)
        value += std::pow(attn_ext0, 4) * extrapolate3(sv_ext0.x, sv_ext0.y, sv_ext0.z, d_ext0, seed);

    // Second extra vertex
    double attn_ext1 = attn(d_ext1);
    if (attn_ext1 > 0)
        value += std::pow(attn_ext1, 4) * extrapolate3(sv_ext1.x, sv_ext1.y, sv_ext1.z, d_ext1, seed);

    return value / NORM_CONSTANT_3D;
}

//////////////////////////////////////////////////////////////////////////
// Worley

glm::dvec2 p_worley(const glm::dvec2& xy, uint32_t seed)
{
    glm::dvec2 t{glm::floor(xy)};
    glm::ivec2 xy0{(int)t.x, (int)t.y};
    glm::dvec2 xyf{xy - t};

    double f0 = 99.0;
    double f1 = 99.0;

    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            glm::ivec2 square{xy0 + glm::ivec2{i, j}};
            auto rnglast = rng(hash(square.x + seed, square.y));

            glm::dvec2 rnd_pt;
            rnd_pt.x = i + (double)(rnglast & 0xFFFF) / (double)0x10000;
            rnglast = rng(rnglast);
            rnd_pt.y = j + (double)(rnglast & 0xFFFF) / (double)0x10000;

            auto dist = glm::distance(xyf, rnd_pt);
            if (dist < f0) {
                f1 = f0;
                f0 = dist;
            } else if (dist < f1) {
                f1 = dist;
            }
        }
    }
    return glm::dvec2{f0, f1};
}

glm::dvec2 p_worley3(const glm::dvec3& p, uint32_t seed)
{
    glm::dvec3 t {glm::floor(p)};
    glm::ivec3 p0 {(int)t.x, (int)t.y, (int)t.z};
    glm::dvec3 pf {p - t};

    auto f0 = std::numeric_limits<double>::max();
    auto f1 = std::numeric_limits<double>::max();

    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            for (int k = -1; k < 2; ++k) {
                glm::ivec3 square = p0 + glm::ivec3{i, j, k};
                auto rnglast = rng(hash(square.x + seed, square.y, square.z));

                glm::dvec3 rnd_pt;
                rnd_pt.x = i + (double)(rnglast & 0xFFFF) / (double)0x10000;;
                rnglast = rng(rnglast);
                rnd_pt.y = j + (double)(rnglast & 0xFFFF) / (double)0x10000;;
                rnglast = rng(rnglast);
                rnd_pt.z = k + (double)(rnglast & 0xFFFF) / (double)0x10000;;

                auto dist = glm::distance(pf, rnd_pt);
                if (dist < f0) {
                    f1 = f0;
                    f0 = dist;
                } else if (dist < f1) {
                    f1 = dist;
                }
            }
        }
    }
    return glm::dvec2{f0, f1};
}

//////////////////////////////////////////////////////////////////////////
// Voronoi

glm::dvec3 p_voronoi(const glm::dvec2& xy, uint32_t seed)
{
    glm::dvec2 t{glm::floor(xy)};
    glm::ivec2 xy0{(int)t.x, (int)t.y};
    glm::dvec2 xyf{xy - t};
    glm::dvec2 result;

    auto f0 = std::numeric_limits<double>::max();

    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            glm::ivec2 square = xy0 + glm::ivec2{i, j};
            auto rnglast = rng(hash(square.x + seed, square.y));

            glm::dvec2 rnd_pt;
            rnd_pt.x = i + (double)(rnglast & 0xFFFF) / (double)0x10000;;
            rnglast = rng(rnglast);
            rnd_pt.y = j + (double)(rnglast & 0xFFFF) / (double)0x10000;;

            auto dist = glm::distance(xyf, rnd_pt);
            if (dist < f0) {
                f0 = dist;
                result = rnd_pt;
            }
        }
    }
    t += result;
    return glm::dvec3{t.x, t.y, 0.0};
}

//////////////////////////////////////////////////////////////////////////

double curve_linear(double x, const std::vector<node::control_point>& curve)
{
    auto i = curve.begin();
    if (x < i->in)
        return i->out;

    for (; i != curve.end(); ++i) {
        if (x < i->in) {
            --i;
            double deltax = (i + 1)->in - i->in;
            return lerp((x - i->in) / deltax, i->out, (i + 1)->out);
        }
    }
    return std::prev(i)->out;
}

double curve_spline(double x, const std::vector<node::control_point>& curve)
{
    int index = 0;
    for (; index < (int)curve.size(); ++index) {
        if (x < curve[index].in)
            break;
    }

    const int lim = curve.size() - 1;
    const int index0 = clamp(index - 2, 0, lim);
    const int index1 = clamp(index - 1, 0, lim);
    const int index2 = clamp(index, 0, lim);
    const int index3 = clamp(index + 1, 0, lim);

    if (index1 == index2)
        return curve[index1].out;

    const double in0 = curve[index1].in;
    const double in1 = curve[index2].in;
    const double a = (x - in0) / (in1 - in0);

    const double out0 = curve[index0].out;
    const double out1 = curve[index1].out;
    const double out2 = curve[index2].out;
    const double out3 = curve[index3].out;

    return interp_cubic(out0, out1, out2, out3, a);
}

double png(const glm::dvec2& p, const generator_context::image& img)
{
    glm::dvec2 fl{glm::floor(p)};
    glm::dvec2 fr{p - fl};
    glm::ivec2 i{fr.x * img.width, fr.y * img.height};

    return ((double)img.buffer[i.y * img.width + i.x] - 127.5) / 127.5;
}

} // anonymous namespace

//---------------------------------------------------------------------------

generator_slowinterpreter::generator_slowinterpreter(
    const generator_context& context, const node& n)
    : generator_i(context)
    , n_(n)
    , seed_(static_cast<uint32_t>(
          boost::get<double>(context.get_global("seed"))))
{
}

std::vector<double> generator_slowinterpreter::run(const glm::dvec2& corner,
                                                   const glm::dvec2& step,
                                                   const glm::ivec2& count)
{
    std::vector<double> result(count.x * count.y);
    size_t i = 0;
    for (int y = 0; y < count.y; ++y)
        for (int x = 0; x < count.x; ++x)
            result[i++] = eval(corner + glm::dvec2{x, y} * step, n_);

    return result;
}

std::vector<int16_t> generator_slowinterpreter::run_int16(
    const glm::dvec2& corner, const glm::dvec2& step, const glm::ivec2& count)
{
    std::vector<int16_t> result(count.x * count.y);
    size_t i = 0;
    for (int y = 0; y < count.y; ++y) {
        for (int x = 0; x < count.x; ++x) {
            result[i++] = static_cast<int16_t>(std::floor(0.5 + 
                eval(corner + glm::dvec2{x, y} * step, n_)));
        }
    }
    return result;
}

std::vector<double> generator_slowinterpreter::run(const glm::dvec3& corner,
                                                   const glm::dvec3& step,
                                                   const glm::ivec3& count)
{
    std::vector<double> result(count.x * count.y * count.z);
    size_t i = 0;
    for (int z = 0; z < count.z; ++z) {
        for (int y = 0; y < count.y; ++y) {
            for (int x = 0; x < count.x; ++x) {
                result[i++] = eval(corner + glm::dvec3{x, y, z} * step, n_);
            }
        }
    }
    return result;
}

std::vector<int16_t> generator_slowinterpreter::run_int16(
    const glm::dvec3& corner, const glm::dvec3& step, const glm::ivec3& count)
{
    std::vector<int16_t> result(count.x * count.y * count.z);
    size_t i = 0;
    for (int z = 0; z < count.z; ++z) {
        for (int y = 0; y < count.y; ++y) {
            for (int x = 0; x < count.x; ++x) {
                result[i++] = static_cast<int16_t>(
                    eval(corner + glm::dvec3{x, y, z} * step, n_));
            }
        }
    }
    return result;
}

double generator_slowinterpreter::eval(const glm::dvec2& p, const node& n)
{
    p_.x = p.x;
    p_.y = p.y;
    p_.z = 0.0;
    return eval_v(n);
}

double generator_slowinterpreter::eval(const glm::dvec3& p, const node& n)
{
    p_ = p;
    return eval_v(n);
}

double generator_slowinterpreter::eval_v(const node& n)
{
    if (n.type == node::const_var)
        return n.aux_var;

    auto& in = n.input[0];

    switch (n.type) {
    case node::angle: {
        auto p = eval_xy(in);
        return std::atan2(p.y, p.x) / pi;
    }

    case node::chebyshev: {
        auto p = eval_xy(in);
        return std::max(std::abs(p.x), std::abs(p.y));
    }

    case node::chebyshev3: {
        auto p = eval_xyz(in);
        return std::max(std::max(std::abs(p.x), std::abs(p.y)), std::abs(p.z));
    }

    case node::checkerboard: {
        auto p = eval_xy(in);
        auto fl = glm::floor(p);
        auto fr = p - fl;
        return (fr.x < 0.5) ^ (fr.y < 0.5) ? 1 : -1;
    }

    case node::checkerboard3: {
        auto p = eval_xyz(in);
        auto fl = glm::floor(p);
        auto fr = p - fl;
        return (fr.x < 0.5) ^ (fr.y < 0.5) ^ (fr.z < 0.5) ? 1 : -1;
    }
    case node::distance:
        return glm::length(eval_xy(in));

    case node::distance3:
        return glm::length(eval_xyz(in));

    case node::perlin: {
        auto p = eval_xy(in);
        auto seed = eval_v(n.input[1]);
        return p_perlin(p, seed);
    }

    case node::perlin3: {
        auto p = eval_xyz(in);
        auto seed = eval_v(n.input[1]);
        return p_perlin3(p, seed);
    }

    case node::simplex: {
        auto p = eval_xy(in);
        auto seed = eval_v(n.input[1]);
        return p_simplex(p, seed_ + seed);
    }

    case node::opensimplex: {
        auto p = eval_xy(in);
        auto seed = eval_v(n.input[1]);
        return p_opensimplex(p, seed_ + seed);
    }

    case node::simplex3: {
        auto p = eval_xyz(in);
        auto seed = eval_v(n.input[1]);
        return p_simplex3(p, seed_ + seed);
    }

    case node::opensimplex3: {
        auto p = eval_xyz(in);
        auto seed = eval_v(n.input[1]);
        return p_opensimplex3(p, seed_ + seed);
    }

    case node::worley: {
        auto p = eval_xy(in);
        auto seed = eval_v(n.input[2]);
        auto tmp = p_;
        p_ = glm::dvec3(p_worley(p, seed_ + seed), 0.0);
        auto result = eval_v(n.input[1]);
        p_ = tmp;
        return result;
    }

    case node::worley3: {
        auto p = eval_xyz(in);
        auto seed = eval_v(n.input[2]);
        auto tmp = p_;
        p_ = glm::dvec3(p_worley3(p, seed_ + seed), 0.0);
        auto result = eval_v(n.input[1]);
        p_ = tmp;
        return result;
    }

    case node::voronoi: {
        auto p(eval_xy(in));
        auto seed(eval_v(n.input[2]));

        auto tmp = p_;
        p_ = p_voronoi(p, seed_ + seed);
        auto result(eval_v(n.input[1]));
        p_ = tmp;
        return result;
    }

    case node::external_: 
        return call_lambda(cntx_.get_script(n.aux_string), in);

    case node::lambda_: 
        return call_lambda(n.input[1], in);

    case node::manhattan: {
        auto p = eval_xy(in);
        return std::abs(p.x) + std::abs(p.y);
    }

    case node::manhattan3: {
        auto p = eval_xyz(in);
        return std::abs(p.x) + std::abs(p.y) + std::abs(p.z);
    }

    case node::x:
        return eval_xy(in).x;

    case node::y:
        return eval_xy(in).y;

    case node::z:
        return eval_xyz(in).z;

    case node::fractal: {
        auto tmp = p_;
        p_ = glm::dvec3{eval_xy(n.input[0]), 0.0};

        auto& f = n.input[1];
        int octaves = eval_v(n.input[2]);

        octaves = std::min(octaves, INTERPRETER_OCTAVES_LIMIT);

        double lacunarity = eval_v(n.input[3]);
        double persistence = eval_v(n.input[4]);

        double div = 0.0, mul = 1.0, result = 0.0;
        for (int i = 0; i < octaves; ++i) {
            result += eval_v(f) * mul;
            div += mul;
            mul *= persistence;
            p_ *= lacunarity;
            p_.x += 12345;
        }
        p_ = tmp;
        return result / div;
    }

    case node::fractal3: {
        auto tmp = p_;
        p_ = eval_xyz(n.input[0]);

        auto& f = n.input[1];
        int octaves = eval_v(n.input[2]);

        octaves = std::min(octaves, INTERPRETER_OCTAVES_LIMIT);

        double lacunarity = eval_v(n.input[3]);
        double persistence = eval_v(n.input[4]);

        double div = 0.0, mul = 1.0, result = 0.0;
        for (int i = 0; i < octaves; ++i) {
            result += eval_v(f) * mul;
            div += mul;
            mul *= persistence;
            p_ *= lacunarity;
            p_.x += 12345;
        }
        p_ = tmp;
        return result / div;
    }

    case node::abs:
        return std::abs(eval_v(in));

    case node::add:
        return eval_v(in) + eval_v(n.input[1]);

    case node::blend: {
        double l = (eval_v(in) + 1.0) / 2.0;
        double a = eval_v(n.input[1]);
        double b = eval_v(n.input[2]);
        return a + l * (b - a);
    }

    case node::cos:
        return std::cos(eval_v(in) * pi);

    case node::div:
        return eval_v(in) / eval_v(n.input[1]);

    case node::max:
        return std::max(eval_v(in), eval_v(n.input[1]));

    case node::min:
        return std::min(eval_v(in), eval_v(n.input[1]));

    case node::mul:
        return eval_v(in) * eval_v(n.input[1]);

    case node::neg:
        return -eval_v(in);

    case node::pow:
        return std::pow(eval_v(in), eval_v(n.input[1]));

    case node::round:
        return std::round(eval_v(in));

    case node::saw: {
        auto v = eval_v(in);
        return v - std::floor(v);
    }

    case node::sin:
        return std::sin(eval_v(in) * pi);

    case node::sqrt:
        return std::sqrt(eval_v(in));

    case node::sub:
        return eval_v(in) - eval_v(n.input[1]);

    case node::tan:
        return std::tan(eval_v(in) * pi);

    case node::then_else:
        return (eval_bool(n.input[0])) ? eval_v(n.input[1])
                                       : eval_v(n.input[2]);

    case node::curve_linear:
        return curve_linear(eval_v(in), n.curve);

    case node::curve_spline:
        return curve_spline(eval_v(in), n.curve);

    case node::png_lookup:
        return png(eval_xy(in), cntx_.get_image(n.input[1].aux_string));

    default:
        throw std::runtime_error("type mismatch");
    }
}

glm::dvec2 generator_slowinterpreter::eval_xy(const node& n)
{
    switch (n.type) {
    case node::entry_point:
        return glm::dvec2{p_.x, p_.y};

    case node::rotate: {
        auto p = eval_xy(n.input[0]);
        auto t = eval_v(n.input[1]) * pi;
        auto ct = std::cos(t);
        auto st = std::sin(t);
        return glm::dvec2{p.x * ct - p.y * st, p.x * st + p.y * ct};
    }

    case node::scale: {
        auto p = eval_xy(n.input[0]);
        auto s = eval_v(n.input[1]);
        return glm::dvec2{p.x / s, p.y / s};
    }

    case node::shift: {
        auto p = eval_xy(n.input[0]);
        auto sx = eval_v(n.input[1]);
        auto sy = eval_v(n.input[2]);
        return glm::dvec2{p.x + sx, p.y + sy};
    }

    case node::map: {
        auto tmp(p_);
        p_ = glm::dvec3{eval_xy(n.input[0]), 0.0};
        auto x = eval_v(n.input[1]);
        auto y = eval_v(n.input[2]);
        p_ = tmp;
        return glm::dvec2{x, y};
    }

    case node::turbulence: {
        auto tmp(p_);
        p_ = glm::dvec3{eval_xy(n.input[0]), 0.0};
        auto x = eval_v(n.input[1]);
        auto y = eval_v(n.input[2]);
        p_ = tmp;
        return glm::dvec2{p_.x + x, p_.y + y};
    }

    case node::swap: {
        auto p = eval_xy(n.input[0]);
        return glm::dvec2{p.y, p.x};
    }

    case node::xy: {
        auto p = eval_xyz(n.input[0]);
        return glm::dvec2{p.x, p.y};
    }

    default:
        throw std::runtime_error("type mismatch");
    }
}

glm::dvec3 generator_slowinterpreter::eval_xyz(const node& n)
{
    switch (n.type) {
    case node::entry_point:
        return p_;

    case node::xplane: {
        auto p = eval_xy(n.input[0]);
        auto x = eval_v(n.input[1]);
        return glm::dvec3{x, p.y, p.x};
    }

    case node::yplane: {
        auto p = eval_xy(n.input[0]);
        auto y = eval_v(n.input[1]);
        return glm::dvec3{p.x, y, p.y};
    }

    case node::zplane: {
        auto p = eval_xy(n.input[0]);
        auto z = eval_v(n.input[1]);
        return glm::dvec3{p.x, p.y, z};
    }

    case node::rotate3: {
        auto p = eval_xyz(n.input[0]);
        auto ax = eval_v(n.input[1]);
        auto ay = eval_v(n.input[2]);
        auto az = eval_v(n.input[3]);
        auto angle = eval_v(n.input[4]) * pi;

        return glm::rotate(p, angle, glm::dvec3(ax, ay, az));
    }

    case node::scale3: {
        auto p = eval_xyz(n.input[0]);
        auto s = eval_v(n.input[1]);
        return p / s;
    }

    case node::shift3: {
        auto p = eval_xyz(n.input[0]);
        auto q = input_vec3(n, 1);
        return p + q;
    }

    case node::map3: {
        auto tmp = p_;
        p_ = eval_xyz(n.input[0]);
        auto q = input_vec3(n, 1);
        p_ = tmp;
        return q;
    }

    case node::turbulence3: {
        auto tmp = p_;
        p_ = eval_xyz(n.input[0]);
        auto q = input_vec3(n, 1);
        p_ = tmp;
        return p_ + q;
    }

    default:
        throw std::runtime_error("type mismatch");
    }
}

bool generator_slowinterpreter::eval_bool(const node& n)
{
    switch (n.type) {
    case node::const_bool:
        return n.aux_bool;

    case node::is_equal:
        return eval_v(n.input[0]) == eval_v(n.input[1]);

    case node::is_greaterthan:
        return eval_v(n.input[0]) > eval_v(n.input[1]);

    case node::is_gte:
        return eval_v(n.input[0]) >= eval_v(n.input[1]);

    case node::is_lessthan:
        return eval_v(n.input[0]) < eval_v(n.input[1]);

    case node::is_lte:
        return eval_v(n.input[0]) <= eval_v(n.input[1]);

    case node::bnot:
        return !eval_bool(n.input[0]);

    case node::band:
        return eval_bool(n.input[0]) && eval_bool(n.input[1]);

    case node::bor:
        return eval_bool(n.input[0]) || eval_bool(n.input[1]);

    case node::bxor:
        return eval_bool(n.input[0]) ^ eval_bool(n.input[1]);

    case node::is_in_circle: {
        auto p = eval_xy(n.input[0]);
        return std::sqrt(p.x * p.x + p.y * p.y) <= eval_v(n.input[1]);
    }

    case node::is_in_rectangle: {
        auto p = eval_xy(n.input[0]);

        return p.x >= eval_v(n.input[1]) && p.y >= eval_v(n.input[2])
               && p.x <= eval_v(n.input[3]) && p.y <= eval_v(n.input[4]);
    }

    default:
        throw std::runtime_error("type mismatch");
    }
}

glm::dvec3 generator_slowinterpreter::input_vec3(const node& n, int i)
{
    return glm::dvec3{eval_v(n.input[i]), eval_v(n.input[i + 1]),
                      eval_v(n.input[i + 2])};
}

double generator_slowinterpreter::call_lambda(const node &func, const node& in)
{
    auto type = func.input_type();
    auto tmp = p_;
    
    if (type == var_t::xyz) 
        p_ = eval_xyz(in);
    else if (type == var_t::xy) 
        p_ = glm::dvec3{eval_xy(in), 0.0};
    else
        throw std::runtime_error("lambda must take a coordinate type");
    
    auto result = eval_v(func);
    p_ = tmp;
    
    return result;
}

} // namespace noise
} // namespace hexa
