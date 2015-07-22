//---------------------------------------------------------------------------
// hexanoise/opencl_prelude.hpp
//
// Copyright 2014-2015, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------

namespace hexa {
namespace noise {

const char* opencl_prelude = R"xxxxx(


#ifdef cl_khr_fp64
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#elif defined(cl_amd_fp64)
    #pragma OPENCL EXTENSION cl_amd_fp64 : enable
#else
    #error "Double precision floating point not supported by OpenCL implementation."
#endif

#define ONE_F1                 (1.0f)
#define ZERO_F1                (0.0f)

__constant int P_MASK = 255;
__constant int P_SIZE = 256;
__constant int P[512] = {151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  };

//////////////////////////////////////////////////////////////////////////

__constant int G_MASK = 15;
__constant int G_SIZE = 16;
__constant int G_VECSIZE = 4;
__constant float G[16*4] = {
      +ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1,
      -ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1,
      +ONE_F1,  -ONE_F1, +ZERO_F1, +ZERO_F1,
      -ONE_F1,  -ONE_F1, +ZERO_F1, +ZERO_F1,
      +ONE_F1, +ZERO_F1,  +ONE_F1, +ZERO_F1,
      -ONE_F1, +ZERO_F1,  +ONE_F1, +ZERO_F1,
      +ONE_F1, +ZERO_F1,  -ONE_F1, +ZERO_F1,
      -ONE_F1, +ZERO_F1,  -ONE_F1, +ZERO_F1,
     +ZERO_F1,  +ONE_F1,  +ONE_F1, +ZERO_F1,
     +ZERO_F1,  -ONE_F1,  +ONE_F1, +ZERO_F1,
     +ZERO_F1,  +ONE_F1,  -ONE_F1, +ZERO_F1,
     +ZERO_F1,  -ONE_F1,  -ONE_F1, +ZERO_F1,
      +ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1,
      -ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1,
     +ZERO_F1,  -ONE_F1,  +ONE_F1, +ZERO_F1,
     +ZERO_F1,  -ONE_F1,  -ONE_F1, +ZERO_F1
};

__constant uint OFFSET_BASIS = 2166136261;
__constant uint FNV_PRIME = 16777619;

inline double lerp (double x, double a, double b)
{
    return mad(x, b - a, a);
}

inline double2 lerp2d (const double x, const double2 a, const double2 b)
{
    return mad(x, b - a, a);
}

inline double4 lerp4d (const double x, const double4 a, const double4 b)
{
    return mad(x, b - a, a);
}

inline double blend3 (const double a)
{
    return a * a * (3.0 - 2.0 * a);
}

inline double blend5 (const double a)
{
    return a * a * a * (a * (a * 6.0 - 15.0) + 10.0);
}


inline uint hash (int x, int y)
{
    return ((uint)x * 2120969693) ^ ((uint)y * 915488749) ^ ((uint)(x + 1103515245) * (uint)(y + 1234567));
}

inline uint hash3 (int x, int y, int z)
{
    return ((uint)x * 2120969693)
            ^ ((uint)y * 915488749)
            ^ ((uint)z * 22695477)
            ^ ((uint)(x + 1103515245) * (uint)(y + 1234567) * (uint)(z + 134775813));
}

inline uint rng (uint last)
{
    return (1103515245 * last + 12345) & 0x7FFFFFFF;
}

//////////////////////////////////////////////////////////////////////////

inline double gradient_noise2d (double2 xy, int2 ixy, uint seed)
{
    ixy.x += seed * 1013;
    ixy.y += seed * 1619;
    ixy &= P_MASK;

    int index = (P[ixy.x+P[ixy.y]] & G_MASK) * G_VECSIZE;
    double2 g = (double2)(G[index], G[index+1]);

    return dot(xy, g);
}

double p_perlin (double2 xy, uint seed)
{
    double2 t = floor(xy);
    int2 xy0 = (int2)((int)t.x, (int)t.y);
    double2 xyf = xy - t;

    const int2 I01 = (int2)(0, 1);
    const int2 I10 = (int2)(1, 0);
    const int2 I11 = (int2)(1, 1);

    const double2 F01 = (double2)(0.0, 1.0);
    const double2 F10 = (double2)(1.0, 0.0);
    const double2 F11 = (double2)(1.0, 1.0);

    const double n00 = gradient_noise2d(xyf      , xy0, seed);
    const double n10 = gradient_noise2d(xyf - F10, xy0 + I10, seed);
    const double n01 = gradient_noise2d(xyf - F01, xy0 + I01, seed);
    const double n11 = gradient_noise2d(xyf - F11, xy0 + I11, seed);

    const double2 n0001 = (double2)(n00, n01);
    const double2 n1011 = (double2)(n10, n11);
    const double2 n2 = lerp2d(blend5(xyf.x), n0001, n1011);

    return lerp(blend5(xyf.y), n2.x, n2.y) * 1.227;
}

inline double gradient_noise3d (int3 ixyz, double3 xyz, uint seed)
{
    ixyz.x += seed * 1013;
    ixyz.y += seed * 1619;
    ixyz.z += seed * 997;
    ixyz &= P_MASK;

    int index = (P[ixyz.x+P[ixyz.y+P[ixyz.z]]] & G_MASK) * G_VECSIZE;
    double3 g = (double3)(G[index], G[index+1], G[index+2]);

    return dot(xyz, g);
}

double p_perlin3 (double3 xyz, uint seed)
{
    double3 t = floor(xyz);
    int3 xyz0 = (int3)((int)t.x, (int)t.y, (int)t.z);
    double3 xyzf = xyz - t;

    const int3 I001 = (int3)(0, 0, 1);
    const int3 I010 = (int3)(0, 1, 0);
    const int3 I011 = (int3)(0, 1, 1);
    const int3 I100 = (int3)(1, 0, 0);
    const int3 I101 = (int3)(1, 0, 1);
    const int3 I110 = (int3)(1, 1, 0);
    const int3 I111 = (int3)(1, 1, 1);

    const double3 F001 = (double3)(0.0, 0.0, 1.0);
    const double3 F010 = (double3)(0.0, 1.0, 0.0);
    const double3 F011 = (double3)(0.0, 1.0, 1.0);
    const double3 F100 = (double3)(1.0, 0.0, 0.0);
    const double3 F101 = (double3)(1.0, 0.0, 1.0);
    const double3 F110 = (double3)(1.0, 1.0, 0.0);
    const double3 F111 = (double3)(1.0, 1.0, 1.0);

    const double n000 = gradient_noise3d(xyz0       , xyzf       , seed);
    const double n001 = gradient_noise3d(xyz0 + I001, xyzf - F001, seed);
    const double n010 = gradient_noise3d(xyz0 + I010, xyzf - F010, seed);
    const double n011 = gradient_noise3d(xyz0 + I011, xyzf - F011, seed);
    const double n100 = gradient_noise3d(xyz0 + I100, xyzf - F100, seed);
    const double n101 = gradient_noise3d(xyz0 + I101, xyzf - F101, seed);
    const double n110 = gradient_noise3d(xyz0 + I110, xyzf - F110, seed);
    const double n111 = gradient_noise3d(xyz0 + I111, xyzf - F111, seed);

    double4 n40 = (double4)(n000, n001, n010, n011);
    double4 n41 = (double4)(n100, n101, n110, n111);

    double4 n4 = lerp4d(blend5(xyzf.x), n40, n41);
    double2 n2 = lerp2d(blend5(xyzf.y), n4.xy, n4.zw);
    double n = lerp(blend5(xyzf.z), n2.x, n2.y);

    return n * 1.216;
}

//////////////////////////////////////////////////////////////////////////

__constant double F2 = 0.366025404; // 0.5 * (sqrt(3.0) - 1.0)
__constant double G2 = 0.211324865; // (3.0 - sqrt(3.0)) / 6.0

double p_simplex (double2 xy, uint seed)
{
    double n0, n1, n2;

    // Skew the input space to determine which simplex cell we're in
    double s = (xy.x + xy.y) * F2;
    int i = floor(xy.x + s);
    int j = floor(xy.y + s);

    // Unskew the cell origin back to (x,y) space
    double t = (i + j) * G2;
    double2 o = (double2)(i - t, j - t);

    // The x,y distances from the cell origin
    double2 d0 = xy - o;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if (d0.x > d0.y)
    {
        i1=1; // lower triangle, XY order: (0,0)->(1,0)->(1,1)
        j1=0;
    }
    else
    {
        i1=0; // upper triangle, YX order: (0,0)->(0,1)->(1,1)
        j1=1;
    }

    double2 d1 = (double2)(d0.x - i1 + G2, d0.y - j1 + G2);
    double2 d2 = (double2)(d0.x - 1.0 + 2.0 * G2, d0.y - 1.0 + 2.0 * G2);

    int ii = (i + seed * 1063) & 0xFF;
    int jj = j & 0xFF;
    int gi0 = (P[ii+P[jj]] & G_MASK) * G_VECSIZE;
    int gi1 = (P[ii+i1+P[jj+j1]] & G_MASK) * G_VECSIZE;
    int gi2 = (P[ii+1+P[jj+1]] & G_MASK) * G_VECSIZE;

    double t0 = 0.5 - dot(d0,d0);
    if (t0 < 0)
    {
        n0 = 0.0;
    }
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * dot((double2)(G[gi0],G[gi0+1]), d0);
    }

    double t1 = 0.5 - dot(d1,d1);
    if(t1 < 0)
    {
        n1 = 0.0;
    }
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * dot((double2)(G[gi1],G[gi1+1]), d1);
    }

    double t2 = 0.5 - dot(d2,d2);
    if(t2 < 0)
    {
        n2 = 0.0;
    }
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * dot((double2)(G[gi2],G[gi2+1]), d2);
    }

    return 70.0 * (n0 + n1 + n2);
}

__constant double G3 = 0.16666666666666666; // 1.0 / 6.0

double p_simplex3 (double3 p, uint seed)
{
    // Skew the input space to determine which simplex cell we're in
    double s = (p.x + p.y + p.z) / 3.0;
    int i = floor(p.x + s);
    int j = floor(p.y + s);
    int k = floor(p.z + s);

    // Unskew the cell origin back to (x,y,z) space
    double t = (i + j + k) / 6.0;
    double3 o = (double3)(i - t, j - t, k - t);

    // The x,y,z distances from the cell origin
    double3 d0 = p - o;

    // For the 3D case, the simplex shape is an irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second (middle) corner of simplex
    int i2, j2, k2; // Offsets for third corner of simplex

    if (d0.x >= d0.y) {
        if (d0.y >= d0.z) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
        } else if (d0.x >= d0.z) {
            i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1;
        } else {
            i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1;
        }
    } else {
        if (d0.y < d0.z) {
            i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1;
        } else if (d0.x < d0.z) {
            i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1;
        } else {
            i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
        }
    }

    double3 d1 = (double3)(d0.x - i1, d0.y - j1, d0.z - k1) + G3;
    double3 d2 = (double3)(d0.x - i2, d0.y - j2, d0.z - k2) + 2.0 * G3;
    double3 d3 = d0 - 1.0 + 3.0 * G3;

    int ii = (i + seed * 1063) & 0xFF;
    int jj = j & 0xFF;
    int kk = k & 0xFF;
    int gi0 = (P[ii+P[jj+P[kk]]] & G_MASK) * G_VECSIZE;
    int gi1 = (P[ii+i1+P[jj+j1+P[kk+k1]]] & G_MASK) * G_VECSIZE;
    int gi2 = (P[ii+i2+P[jj+j2+P[kk+k2]]] & G_MASK) * G_VECSIZE;
    int gi3 = (P[ii+1+P[jj+1+P[kk+1]]] & G_MASK) * G_VECSIZE;

    double n0, n1, n2, n3;
    double t0 = 0.6 - dot(d0, d0);
    if (t0 < 0) {
        n0 = 0.0;
    } else {
        n0 = pow(t0, 4) * dot((double3)(G[gi0],G[gi0+1],G[gi0+2]), d0);
    }

    double t1 = 0.6 - dot(d1, d1);
    if (t1 < 0) {
        n1 = 0.0;
    } else {
        n1 = pow(t1, 4) * dot((double3)(G[gi1],G[gi1+1],G[gi1+2]), d1);
    }

    double t2 = 0.6 - dot(d2, d2);
    if (t2 < 0) {
        n2 = 0.0;
    } else {
        n2 = pow(t2, 4) * dot((double3)(G[gi2],G[gi2+1],G[gi2+2]), d2);
    }

    double t3 = 0.6 - dot(d3, d3);
    if (t3 < 0) {
        n3 = 0.0;
    } else {
        n3 = pow(t3, 4) * dot((double3)(G[gi3],G[gi3+1],G[gi3+2]), d3);
    }

    return 32.0 * (n0 + n1 + n2 + n3);
}

)xxxxx"  /* Split in half so MSVC can handle it */
R"xxxxy(

//////////////////////////////////////////////////////////////////////////

// Gradients for 2D. They approximate the directions to the
// vertices of an octagon from the center.
__constant int gradients2D[16] = {
    5, 2, 2, 5,
    -5, 2, -2, 5,
    5, -2, 2, -5,
    -5, -2, -2, -5
};

// Gradients for 3D. They approximate the directions to the
// vertices of a rhombicuboctahedron from the center, skewed so
// that the triangular and square facets can be inscribed inside
// circles of the same radius.
__constant int gradients3D[] = {
    -11,  4,  4,  -4,  11,  4,  -4,  4,  11,
     11,  4,  4,   4,  11,  4,   4,  4,  11,
    -11, -4,  4,  -4, -11,  4,  -4, -4,  11,
     11, -4,  4,   4, -11,  4,   4, -4,  11,
    -11,  4, -4,  -4,  11, -4,  -4,  4, -11,
     11,  4, -4,   4,  11, -4,   4,  4, -11,
    -11, -4, -4,  -4, -11, -4,  -4, -4, -11,
     11, -4, -4,   4, -11, -4,   4, -4, -11
};

inline double extrapolate2(int x, int y, double2 d, uint seed)
{
    int index = P[(P[(x + seed) & 0xFF] + (y + seed * 23)) & 0xFF] & 0x0E;
    return gradients2D[index] * d.x + gradients2D[index + 1] * d.y;
}

inline double extrapolate3(int x, int y, int z, double3 d, uint seed)
{
    int index = (P[(P[(P[(x + seed) & 0xFF] + (y + seed * 23)) & 0xFF] + (z + seed * 27)) & 0xFF] % 24) * 3;
    return gradients3D[index] * d.x + gradients3D[index + 1] * d.y + gradients3D[index + 2] * d.z;
}

// Implementation of the OpenSimplex algorithm by Kurt Spencer.

double p_opensimplex (double2 p, uint seed)
{
    const double STRETCH_CONSTANT_2D = -0.211324865405187; // (1 / sqrt(2 + 1) - 1 ) / 2;
    const double SQUISH_CONSTANT_2D = 0.366025403784439; // (sqrt(2 + 1) -1) / 2;
    const double NORM_CONSTANT_2D = 47.0;

    // Place input coordinates onto grid.
    double stretchOffset = (p.x + p.y) * STRETCH_CONSTANT_2D;
    double2 s = p + stretchOffset;

    // Floor to get grid coordinates of rhombus (stretched square) super-cell origin.
    int2 sb = (int2)(floor(s.x), floor(s.y));

    // Skew out to get actual coordinates of rhombus origin. We'll need these later.
    double squishOffset = (sb.x + sb.y) * SQUISH_CONSTANT_2D;
    double2 b = (double2)(sb.x, sb.y) + squishOffset;

    // Compute grid coordinates relative to rhombus origin.
    double2 ins = s - (double2)(sb.x, sb.y);

    // Sum those together to get a value that determines which region we're in.
    double inSum = ins.x + ins.y;

    // Positions relative to origin point.
    double2 d0 = p - b;

    // We'll be defining these inside the next block and using them afterwards.
    double2 d_ext;
    int2 sv_ext;
    double value = 0;

    // Contribution (1,0)
    double2 d1 = d0 + (double2)(-1,0) - SQUISH_CONSTANT_2D;
    double attn1 = 2.0 - dot(d1, d1);
    if (attn1 > 0)
        value += pow(attn1, 4) * extrapolate2(sb.x + 1, sb.y + 0, d1, seed);

    // Contribution (0,1)
    double2 d2 = d0 + (double2)(0,-1) - SQUISH_CONSTANT_2D;
    double attn2 = 2.0 - dot(d2, d2);
    if (attn2 > 0)
        value += pow(attn2, 4) * extrapolate2(sb.x + 0, sb.y + 1, d2, seed);

    if (inSum <= 1) { // We're inside the triangle (2-Simplex) at (0,0)
        double zins = 1 - inSum;
        if (zins > ins.x || zins > ins.y) { // (0,0) is one of the closest two triangular vertices
            if (ins.x > ins.y) {
                sv_ext = sb + (int2)(1, -1);
                d_ext = d0 + (double2)(-1, 1);
            } else {
                sv_ext = sb + (int2)(-1, 1);
                d_ext = d0 + (double2)(1, -1);
            }
        } else { // (1,0) and (0,1) are the closest two vertices.
            sv_ext = sb + (int2)(1, 1);
            d_ext = d0 + (double2)(-1, -1) - 2 * SQUISH_CONSTANT_2D;
        }
    } else { // We're inside the triangle (2-Simplex) at (1,1)
        double zins = 2 - inSum;
        if (zins < ins.x || zins < ins.y) { // (0,0) is one of the closest two triangular vertices
            if (ins.x > ins.y) {
                sv_ext = sb + (int2)(2,0);
                d_ext = d0 + (double2)(-2, 0) - 2 * SQUISH_CONSTANT_2D;
            } else {
                sv_ext = sb + (int2)(0, 2);
                d_ext = d0 + (double2)(0, -2) - 2 * SQUISH_CONSTANT_2D;
            }
        } else { // (1,0) and (0,1) are the closest two vertices.
            d_ext = d0;
            sv_ext = sb;
        }
        sb += 1;
        d0 = d0 - 1.0 - 2 * SQUISH_CONSTANT_2D;
    }

    // Contribution (0,0) or (1,1)
    double attn0 = 2.0 - dot(d0, d0);
    if (attn0 > 0)
        value += pow(attn0, 4) * extrapolate2(sb.x, sb.y, d0, seed);

    // Extra Vertex
    double attn_ext = 2.0 - dot(d_ext, d_ext);
    if (attn_ext > 0)
        value += pow(attn_ext, 4) * extrapolate2(sv_ext.x, sv_ext.y, d_ext, seed);

    return (value / NORM_CONSTANT_2D) * 1.152;
}

double p_opensimplex3 (double3 p, uint seed)
{
    const double STRETCH_CONSTANT_3D = -1.0 / 6.0; // (1 / sqrt(3 + 1) - 1) / 3;
    const double SQUISH_CONSTANT_3D = 1.0 / 3.0; // (sqrt(3+1)-1)/3;
    const double NORM_CONSTANT_3D = 103.0;

    // Place input coordinates on simplectic honeycomb.
    double stretchOffset = (p.x + p.y + p.z) * STRETCH_CONSTANT_3D;
    double3 s = p + stretchOffset;

    // Floor to get grid coordinates of rhombohedron (stretched cube) super-cell origin.
    int3 sb = (int3)(floor(s.x), floor(s.y), floor(s.z));

    // Skew out to get actual coordinates of rhombohedron origin. We'll need these later.
    double3 dsb = (double3)(sb.x, sb.y, sb.z);
    double squishOffset = (dsb.x + dsb.y + dsb.z) * SQUISH_CONSTANT_3D;
    double3 b = dsb + squishOffset;

    // Compute grid coordinates relative to rhombus origin.
    double3 ins = s - dsb;

    // Sum those together to get a value that determines which region we're in.
    double inSum = ins.x + ins.y + ins.z;

    // Positions relative to origin point.
    double3 d0 = p - b;

    // We'll be defining these inside the next block and using them afterwards.
    double3 d_ext0, d_ext1;
    int3 sv_ext0, sv_ext1;
    double value = 0.0;

    if (inSum <= 1) { // We're inside the tetrahedron (3-Simplex) at (0,0,0)
        // Determine which two of (0,0,1), (0,1,0), (1,0,0) are closest.
        uchar aPoint = 0x01;
        double aScore = ins.x;
        uchar bPoint = 0x02;
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
            uchar c = (bScore > aScore ? bPoint : aPoint); // Our other closest vertex is the closest out of a and b.
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
            uchar c = (aPoint | bPoint); // Our two extra vertices are determined by the closest two.
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
        double attn0 = 2.0 - dot(d0, d0);
        if (attn0 > 0)
            value += pow(attn0, 4) * extrapolate3(sb.x + 0, sb.y + 0, sb.z + 0, d0, seed);

        // Contribution (1,0,0)
        double3 d1 = (d0 + (double3)(-1,0,0)) - SQUISH_CONSTANT_3D;
        double attn1 = 2.0 - dot(d1, d1);
        if (attn1 > 0)
            value += pow(attn1, 4) * extrapolate3(sb.x + 1, sb.y + 0, sb.z + 0, d1, seed);

        // Contribution (0,1,0)
        double3 d2 = (double3)(d0.x - SQUISH_CONSTANT_3D, d0.y - 1 - SQUISH_CONSTANT_3D, d1.z);
        double attn2 = 2.0 - dot(d2, d2);
        if (attn2 > 0)
            value += pow(attn2, 4) * extrapolate3(sb.x + 0, sb.y + 1, sb.z + 0, d2, seed);

        // Contribution (0,0,1)
        double3 d3 = (double3)(d2.x, d1.y, d0.z - 1 - SQUISH_CONSTANT_3D);
        double attn3 = 2.0 - dot(d3, d3);
        if (attn3 > 0)
            value += pow(attn3, 4) * extrapolate3(sb.x + 0, sb.y + 0, sb.z + 1, d3, seed);

    } else if (inSum >= 2) { // We're inside the tetrahedron (3-Simplex) at (1,1,1)

        // Determine which two tetrahedral vertices are the closest, out of (1,1,0), (1,0,1), (0,1,1) but not (1,1,1).
        uchar aPoint = 0x06;
        double aScore = ins.x;
        uchar bPoint = 0x05;
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
            uchar c = (bScore < aScore ? bPoint : aPoint); // Our other closest vertex is the closest out of a and b.
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

            uchar c = (aPoint & bPoint); // Our two extra vertices are determined by the closest two.
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
        double3 d3 = (double3)(d0 + (double3)(-1,-1,0)) - 2 * SQUISH_CONSTANT_3D;
        double attn3 = 2.0 - dot(d3, d3);
        if (attn3 > 0)
            value += pow(attn3,4) * extrapolate3(sb.x + 1, sb.y + 1, sb.z + 0, d3, seed);

        // Contribution (1,0,1)
        double3 d2 = (double3)(d3.x, d0.y - 0 - 2 * SQUISH_CONSTANT_3D, d0.z - 1 - 2 * SQUISH_CONSTANT_3D);
        double attn2 = 2.0 - dot(d2, d2);
        if (attn2 > 0)
            value += pow(attn2, 4) * extrapolate3(sb.x + 1, sb.y + 0, sb.z + 1, d2, seed);

        // Contribution (0,1,1)
        double3 d1 = (double3)(d0.x - 0 - 2 * SQUISH_CONSTANT_3D, d3.y, d2.z);
        double attn1 = 2.0 - dot(d1, d1);
        if (attn1 > 0)
            value += pow(attn1, 4) * extrapolate3(sb.x + 0, sb.y + 1, sb.z + 1, d1, seed);

        // Contribution (1,1,1)
        d0 -= 1 + 3 * SQUISH_CONSTANT_3D;
        double attn0 = 2.0 - dot(d0, d0);
        if (attn0 > 0)
            value += pow(attn0, 4) * extrapolate3(sb.x + 1, sb.y + 1, sb.z + 1, d0, seed);

    } else { // We're inside the octahedron (Rectified 3-Simplex) in between.

        double aScore;
        uchar aPoint;
        bool aIsFurtherSide;
        double bScore;
        uchar bPoint;
        bool bIsFurtherSide;
)xxxxy"  /* Split in half so MSVC can handle it */
R"xxxxz(
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
                uchar c = (aPoint & bPoint);
                if ((c & 0x01) != 0) {
                    d_ext1 = d0 + (double3)(-2,0,0) - 2 * SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + (int3)(2,0,0);
                } else if ((c & 0x02) != 0) {
                    d_ext1 = d0 + (double3)(0,-2,0) - 2 * SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + (int3)(0,2,0);
                } else {
                    d_ext1 = d0 + (double3)(0,0,-2) - 2 * SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + (int3)(0,0,2);
                }
            } else { // Both closest points on (0,0,0) side
                // One of the two extra points is (0,0,0)
                d_ext0 = d0;
                sv_ext0 = sb;

                // Other extra point is based on the omitted axis.
                uchar c = (aPoint | bPoint);
                if ((c & 0x01) == 0) {
                    d_ext1 = d0 + (double3)(1,-1,-1) - SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + (int3)(-1,1,1);
                } else if ((c & 0x02) == 0) {
                    d_ext1 = d0 + (double3)(-1,1,-1) - SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + (int3)(1,-1,1);
                } else {
                    d_ext1 = d0 + (double3)(-1,-1,1) - SQUISH_CONSTANT_3D;
                    sv_ext1 = sb + (int3)(1,1,-1);
                }
            }
        } else { // One point on (0,0,0) side, one point on (1,1,1) side
            uchar c1, c2;
            if (aIsFurtherSide) {
                c1 = aPoint;
                c2 = bPoint;
            } else {
                c1 = bPoint;
                c2 = aPoint;
            }

            // One contribution is a permutation of (1,1,-1)
            if ((c1 & 0x01) == 0) {
                d_ext0 = d0 + (double3)(1,-1,-1) - SQUISH_CONSTANT_3D;
                sv_ext0 = sb + (int3)(-1,1,1);
            } else if ((c1 & 0x02) == 0) {
                d_ext0 = d0 + (double3)(-1,1,-1) - SQUISH_CONSTANT_3D;
                sv_ext0 = sb + (int3)(1,-1,1);
            } else {
                d_ext0 = d0 + (double3)(-1,-1,1) - SQUISH_CONSTANT_3D;
                sv_ext0 = sb + (int3)(1,1,-1);
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
        double3 d1 = (double3)(d0 + (double3)(-1,0,0)) - SQUISH_CONSTANT_3D;
        double attn1 = 2.0 - dot(d1, d1);
        if (attn1 > 0)
            value += pow(attn1, 4) * extrapolate3(sb.x + 1, sb.y + 0, sb.z + 0, d1, seed);

        // Contribution (0,1,0)
        double3 d2 = (double3)(d0.x - SQUISH_CONSTANT_3D, d0.y - 1 - SQUISH_CONSTANT_3D, d1.z);
        double attn2 = 2.0 - dot(d2, d2);
        if (attn2 > 0)
            value += pow(attn2, 4)* extrapolate3(sb.x + 0, sb.y + 1, sb.z + 0, d2, seed);

        // Contribution (0,0,1)
        double3 d3 = (double3)(d2.x, d1.y, d0.z - 1 - SQUISH_CONSTANT_3D);
        double attn3 = 2.0 - dot(d3, d3);
        if (attn3 > 0)
            value += pow(attn3, 4)* extrapolate3(sb.x + 0, sb.y + 0, sb.z + 1, d3, seed);

        // Contribution (1,1,0)
        double3 d4 = d0 - (double3)(1,1,0) - 2 * SQUISH_CONSTANT_3D;
        double attn4 = 2.0 - dot(d4, d4);
        if (attn4 > 0)
            value += pow(attn4, 4)* extrapolate3(sb.x + 1, sb.y + 1, sb.z + 0, d4, seed);

        // Contribution (1,0,1)
        double3 d5 = (double3)(d4.x, d0.y - 2 * SQUISH_CONSTANT_3D, d0.z - 1 - 2 * SQUISH_CONSTANT_3D);
        double attn5 = 2.0 - dot(d5, d5);
        if (attn5 > 0)
            value += pow(attn5, 4)* extrapolate3(sb.x + 1, sb.y + 0, sb.z + 1, d5, seed);

        // Contribution (0,1,1)
        double3 d6 = (double3)(d0.x - 2 * SQUISH_CONSTANT_3D, d4.y, d5.z);
        double attn6 = 2.0 - dot(d6, d6);
        if (attn6 > 0)
            value += pow(attn6, 4) * extrapolate3(sb.x + 0, sb.y + 1, sb.z + 1, d6, seed);
    }
    // First extra vertex
    double attn_ext0 = 2.0 - dot(d_ext0, d_ext0);
    if (attn_ext0 > 0)
        value += pow(attn_ext0, 4) * extrapolate3(sv_ext0.x, sv_ext0.y, sv_ext0.z, d_ext0, seed);

    // Second extra vertex
    double attn_ext1 = 2.0 - dot(d_ext1, d_ext1);
    if (attn_ext1 > 0)
        value += pow(attn_ext1, 4) * extrapolate3(sv_ext1.x, sv_ext1.y, sv_ext1.z, d_ext1, seed);

    return value / NORM_CONSTANT_3D;
}

//////////////////////////////////////////////////////////////////////////

double2 p_worley (const double2 p, uint seed)
{
    double2 t = floor(p);
    int2 xy0 = (int2)((int)t.x, (int)t.y);
    double2 xyf = p - t;

    double f0 = 9999.9;
    double f1 = 9999.9;

    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {
            int2 square = xy0 + (int2)(i,j);
            uint h = rng(hash(square.x + seed, square.y));

            double2 rnd_pt;
            rnd_pt.x = (double)i + ((double)(h & 0xFFFF) / (double)0x10000);
            h = rng(h);
            rnd_pt.y = (double)j + ((double)(h & 0xFFFF) / (double)0x10000);

            double dist = distance(xyf, rnd_pt);
            if (dist < f0)
            {
                f1 = f0;
                f0 = dist;
            }
            else if (dist < f1)
            {
                f1 = dist;
            }
        }
    }
    return (double2)(f0, f1);
}

double2 p_worley3 (const double3 p, uint seed)
{
    double3 t = floor(p);
    int3 xyz0 = (int3)((int)t.x, (int)t.y, (int)t.z);
    double3 xyzf = p - t;

    double f0 = 9999.9;
    double f1 = 9999.9;

    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {
            for (int k = -1; k < 2; ++k)
            {
                int3 square = xyz0 + (int3)(i,j,k);
                uint h = rng(hash3(square.x + seed, square.y, square.z));

                double3 rnd_pt;
                rnd_pt.x = (double)i + ((double)(h & 0xFFFF) / (double)0x10000);
                h = rng(h);
                rnd_pt.y = (double)j + ((double)(h & 0xFFFF) / (double)0x10000);
                h = rng(h);
                rnd_pt.z = (double)k + ((double)(h & 0xFFFF) / (double)0x10000);

                double dist = distance(xyzf, rnd_pt);
                if (dist < f0)
                {
                    f1 = f0;
                    f0 = dist;
                }
                else if (dist < f1)
                {
                    f1 = dist;
                }
            }
        }
    }
    return (double2)(f0, f1);
}

//////////////////////////////////////////////////////////////////////////

double2 p_voronoi (const double2 p, uint seed)
{
    double2 t = floor(p);
    int2 xy0 = (int2)((int)t.x, (int)t.y);
    double2 xyf = p - t;

    double f0 = 9999.9;
    double2 nearest;

    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {
            int2 square = xy0 + (int2)(i,j);
            uint h = rng(hash(square.x + seed, square.y));

            double2 rnd_pt;
            rnd_pt.x = (double)i + ((double)(h & 0xFFFF) / (double)0x10000);
            h = rng(h);
            rnd_pt.y = (double)j + ((double)(h & 0xFFFF) / (double)0x10000);

            double dist = distance(xyf, rnd_pt);
            if (dist < f0)
            {
                nearest = rnd_pt;
                f0 = dist;
            }
        }
    }
    return t + nearest;
}

//////////////////////////////////////////////////////////////////////////

inline double2 p_rotate (double2 p, double a)
{
    double t = a * M_PI;
    return (double2)(p.x * cos(t) - p.y * sin(t), p.x * sin(t) + p.y * cos(t));
}

inline double16 rotMatrix(double3 a, double angle)
{
    const double u2 = a.x * a.x;
    const double v2 = a.y * a.y;
    const double w2 = a.z * a.z;
    const double l = u2 + v2 + w2;
    const double sl = sqrt(l);
    const double sa = sin(angle);
    const double ca = cos(angle);

    return (double16)(

    (u2 + (v2 + w2) * ca) / l,
    (a.x * a.y * (1.0 - ca) - a.z * sl * sa) / l,
    (a.x * a.z * (1.0 - ca) + a.y * sl * sa) / l,
    0.0,

    (a.x * a.y * (1.0 - ca) + a.z * sl * sa) / l,
    (v2 + (u2 + w2) * ca) / l,
    (a.y * a.z * (1.0 - ca) - a.x * sl * sa) / l,
    0.0,

    (a.x * a.z * (1.0 - ca) - a.y * sl * sa) / l,
    (a.y * a.z * (1.0 - ca) + a.x * sl * sa) / l,
    (w2 + (u2 + v2) * ca) / l,
    0.0,

    0.0, 0.0, 0.0, 1.0
    );
}

inline double3 p_rotate3 (double3 p, double3 axis, double a)
{
    double4 h = (double4)(p, 1);
    double16 m = rotMatrix(axis, a * M_PI);
    return (double3)(
                dot(m.s048C, h),
                dot(m.s159D, h),
                dot(m.s26AE, h)
                );
}

inline double2 p_swap (double2 p)
{
    return (double2)(p.y, p.x);
}

inline double p_angle (double2 p)
{
    return atan2pi(p.y, p.x);
}

inline double p_chebyshev (double2 p)
{
    return fmax(fabs(p.x), fabs(p.y));
}

inline double p_chebyshev3 (double3 p)
{
    return fmax(fmax(fabs(p.x), fabs(p.y)), fabs(p.z));
}

inline double p_saw (double n)
{
    return n - floor(n);
}

inline double p_checkerboard (double2 p)
{
    double2 sp = p - floor(p);
    return (sp.x < 0.5) ^ (sp.y < 0.5) ? 1.0 : -1.0;
}

inline double p_checkerboard3 (double3 p)
{
    double3 sp = p - floor(p);
    return (sp.x < 0.5) ^ (sp.y < 0.5) ^ (sp.z < 0.5) ? 1.0 : -1.0;
}

inline double p_manhattan (double2 p)
{
    return fabs(p.x) + fabs(p.y);
}

inline double p_manhattan3 (double3 p)
{
    return fabs(p.x) + fabs(p.y) + fabs(p.z);
}

inline double p_blend (double x, double a, double b)
{
    x = (clamp(x, -1.0, 1.0) + 1.0) / 2.0;
    return lerp(x, a, b);
}

inline double p_range (double x, double a, double b)
{
    return clamp((a * 0.5 + x + 1.0) * ((b - a) * 0.5), a, b);
}

inline bool p_is_in_circle (double2 p, double r)
{
    return length(p) <= r;
}

inline bool p_is_in_rectangle (double2 p, double x1, double y1, double x2, double y2)
{
    return p.x >= x1 && p.y >= y1 && p.x <= x2 && p.y <= y2;
}


)xxxxz";

} // namespace noise
} // namespace hexa
