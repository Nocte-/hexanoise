
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

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
  
inline int2 floor2di (double2 p)
{
    double2 t = floor(p);
    int2 xy0 = (int2)((int)t.x, (int)t.y);
}

inline double lerp (double x, double a, double b)
{
    return mad(x, b - a, a);
}

inline double blend3 (const double a)
{
    return a * a * (3.0 - 2.0 * a);
}

inline double blend5 (const double a)
{
    const double a3 = a * a * a;
    const double a4 = a3 * a;
    const double a5 = a4 * a;

    return 10.0 * a3 - 15.0 * a4 + 6.0 * a5;
}

inline double2 lerp2d (const double x, const double2 a, const double2 b)
{
    return mad(x, b - a, a);
}

inline uint poisson_dist9 (const uint v)
{
    if(v< 393325350) return 1;
    if(v< 1022645910) return 2;
    if(v< 1861739990) return 3;
    if(v< 2700834071) return 4;
    if(v< 3372109335) return 5;
    if(v< 3819626178) return 6;
    if(v< 4075350088) return 7;
    if(v< 4203212043) return 8;
    return 9;
}


inline double gradient_noise2d (double2 xy, int2 ixy, uint seed)
{
    ixy.y += seed;
    ixy &= P_MASK;

    int index = (P[ixy.x+P[ixy.y]] & G_MASK) * G_VECSIZE;
    double2 g = (double2)(G[index], G[index+1]);

    return dot(xy, g);

    /*
    int index = dot(int2(1619, 31337), ixy) + 1013 * seed;
    index ^= index >> 8;
    index &= 0xff;

    const double2 gradient = randomvectors[index];
    const double2 f = (xy - ixy);
    return dot(f, gradient);
    */
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

    return lerp(blend5(xyf.y), n2.x, n2.y);
}

double2 p_worley (double2 xy, uint seed)
{
    double2 f = (double2)(3.0, 3.0);

    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {

        }
    }

    return f;
}

inline double2 p_rotate (double2 p, double a)
{
    double t = a * M_PI;
    return (double2)(p.x * cos(t) - p.y * sin(t), p.x * sin(t) + p.y * cos(t));
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

inline double p_saw (double n)
{
    return n - floor(n);
}

inline double p_checkerboard (double2 p)
{
    double2 sp = p - floor(p);
    return (sp.x < 0.5 && sp.y < 0.5) || (sp.x >= 0.5 && sp.y >= 0.5)
           ? 1.0 : -1.0;
}

inline double p_manhattan (double2 p)
{
    return fabs(p.x) + fabs(p.y);
}

inline double p_blend (double x, double a, double b)
{
    x = (clamp(x, -1.0, 1.0) + 1.0) / 2.0;
    return lerp(x, a, b);
}


