//---------------------------------------------------------------------------
// hexanoise/generator_slowinterpreter.cpp
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include "generator_slowinterpreter.hpp"

#include <cmath>
#include <stdexcept>
#include "node.hpp"

#ifndef INTERPRETER_OCTAVES_LIMIT
# define INTERPRETER_OCTAVES_LIMIT 16
#endif

namespace hexa {
namespace noise {

namespace {

const double pi = 3.14159265358979323846;

#define ONE_F1                 (1.0f)
#define ZERO_F1                (0.0f)

const int P_MASK = 255;
const int P_SIZE = 256;
static const int P[512] = {151,160,137,91,90,15,
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

const int G_MASK = 15;
const int G_SIZE = 16;
const int G_VECSIZE = 4;
static const float G[16*4] = {
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

inline double clamp (double x, double min, double max)
{
    return std::min(std::max(x,min),max);
}

inline double lerp (double x, double a, double b)
{
    return a + x * (b - a);
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

inline double interp_cubic (double v0, double v1, double v2, double v3, double a)
{
    const double x (v3 - v2 - v0 + v1);
    const double a2 (a * a);
    const double a3 (a2 * a);
    return x * a3 + (v0 - v1 - x) * a2 + (v2 - v0) * a + v1;
}

inline glm::dvec2 lerp2d (const double x, const glm::dvec2& a, const glm::dvec2& b)
{
    return a + x * (b - a);
}



#define OFFSET_BASIS 2166136261
#define FNV_PRIME 16777619

// FNV hash: http://isthe.com/chongo/tech/comp/fnv/#FNV-source
inline uint32_t hash (uint32_t i, uint32_t j, uint32_t k)
{
  return (uint32_t)((((((OFFSET_BASIS ^ i) * FNV_PRIME) ^ j) * FNV_PRIME) ^ k) * FNV_PRIME);
}

inline uint32_t hash (uint32_t i, uint32_t j)
{
  return (uint32_t)((((OFFSET_BASIS ^ i) * FNV_PRIME) ^ j) * FNV_PRIME);
}

inline uint32_t rng(uint32_t last)
{
    return (1103515245 * last + 12345) & 0x7FFFFFFF;
}

inline double gradient_noise2d (const glm::dvec2& xy, glm::ivec2 ixy, uint32_t seed)
{
    ixy.x += seed * 1013;
    ixy.y += seed * 1619;
    ixy &= P_MASK;

    int index ((P[ixy.x+P[ixy.y]] & G_MASK) * G_VECSIZE);
    glm::dvec2 g (G[index], G[index+1]);

    return glm::dot(xy, g);

    /*
    int index = dot(int2(1619, 31337), ixy) + 1013 * seed;
    index ^= index >> 8;
    index &= 0xff;

    const double2 gradient = randomvectors[index];
    const double2 f = (xy - ixy);
    return dot(f, gradient);
    */
}

double p_perlin (const glm::dvec2& xy, uint32_t seed)
{
    glm::dvec2 t (glm::floor(xy));
    glm::ivec2 xy0 ((int) t.x, (int) t.y);
    glm::dvec2 xyf (xy - t);

    const glm::ivec2 I01 (0, 1);
    const glm::ivec2 I10(1, 0);
    const glm::ivec2 I11(1, 1);

    const glm::dvec2 F01 (0.0, 1.0);
    const glm::dvec2 F10 (1.0, 0.0);
    const glm::dvec2 F11 (1.0, 1.0);

    const double n00 = gradient_noise2d(xyf      , xy0, seed);
    const double n10 = gradient_noise2d(xyf - F10, xy0 + I10, seed);
    const double n01 = gradient_noise2d(xyf - F01, xy0 + I01, seed);
    const double n11 = gradient_noise2d(xyf - F11, xy0 + I11, seed);

    const glm::dvec2 n0001 (n00, n01);
    const glm::dvec2 n1011 (n10, n11);
    const glm::dvec2 n2 = lerp2d(blend5(xyf.x), n0001, n1011);

    return lerp(blend5(xyf.y), n2.x, n2.y) * 1.1;
}

glm::dvec2 p_worley (const glm::dvec2& xy, uint32_t seed)
{
    glm::dvec2 t (glm::floor(xy));
    glm::ivec2 xy0 ((int) t.x, (int) t.y);
    glm::dvec2 xyf (xy - t);

    double f0 (99), f1 (99);

    for (int i (-1); i < 2; ++i)
    {
        for (int j (-1); j < 2; ++j)
        {
            glm::ivec2 square (xy0 + glm::ivec2(i,j));
            auto rnglast (rng(hash(square.x + seed, square.y)));

            glm::dvec2 rnd_pt;
            rnd_pt.x = i + (double)rnglast / (double)0x7FFFFFFF;
            rnglast = rng(rnglast);
            rnd_pt.y = j + (double)rnglast / (double)0x7FFFFFFF;

            double dist (glm::distance(xyf, rnd_pt));
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
    return glm::dvec2(f0, f1);
}

glm::dvec2 p_voronoi (const glm::dvec2& xy, uint32_t seed)
{
    glm::dvec2 t (glm::floor(xy));
    glm::ivec2 xy0 ((int) t.x, (int) t.y);
    glm::dvec2 xyf (xy - t);
    glm::dvec2 result;

    double f0 (99);

    for (int i (-1); i < 2; ++i)
    {
        for (int j (-1); j < 2; ++j)
        {
            glm::ivec2 square (xy0 + glm::ivec2(i,j));
            auto rnglast (rng(hash(square.x + seed, square.y)));

            glm::dvec2 rnd_pt;
            rnd_pt.x = i + (double)rnglast / (double)0x7FFFFFFF;
            rnglast = rng(rnglast);
            rnd_pt.y = j + (double)rnglast / (double)0x7FFFFFFF;

            double dist (glm::distance(xyf, rnd_pt));
            if (dist < f0)
            {
                f0 = dist;
                result = rnd_pt;
            }
        }
    }
    return t + result;
}

} // anonymous namespace

//---------------------------------------------------------------------------

generator_slowinterpreter::generator_slowinterpreter
                                (const generator_context& context,
                                 const node& n)
    : generator_i(context)
    , n_(n)
{
}

std::vector<double>
generator_slowinterpreter::run (const glm::dvec2& corner,
                                const glm::dvec2& step,
                                const glm::ivec2& count)
{
    std::vector<double> result (count.x * count.y);
    size_t i (0);

    for (int y (0); y < count.y; ++y)
        for (int x (0); x < count.x; ++x)
            result[i++] = eval(corner + glm::dvec2(x,y) * step, n_);

    return result;
}

double
generator_slowinterpreter::eval (const glm::dvec2& p, const node& n)
{
    p_ = p;
    return eval_v(n);
}

double
curve_linear (double x, const std::vector<node::control_point>& curve)
{
    auto i (curve.begin());
    if (x < i->in)
        return i->out;

    for (; i != curve.end(); ++i)
    {
        if (x < i->in)
        {
            --i;
            double deltax ((i+1)->in - i->in);
            return lerp((x - i->in) / deltax, i->out, (i+1)->out);
        }
    }
    return std::prev(i)->out;
}

double
curve_spline (double x, const std::vector<node::control_point>& curve)
{
    int index (0);
    for (; index < (int)curve.size(); ++index)
    {
        if (x < curve[index].in)
            break;
    }

    const int lim (curve.size() - 1);
    const int index0 (clamp(index-2, 0, lim));
    const int index1 (clamp(index-1, 0, lim));
    const int index2 (clamp(index,   0, lim));
    const int index3 (clamp(index+1, 0, lim));

    if (index1 == index2)
        return curve[index1].out;

    const double in0 (curve[index1].in);
    const double in1 (curve[index2].in);
    const double a   ((x - in0) / (in1 - in0));

    const double out0 (curve[index0].out);
    const double out1 (curve[index1].out);
    const double out2 (curve[index2].out);
    const double out3 (curve[index3].out);

    return interp_cubic(out0, out1, out2, out3, a);
}

double png (const glm::dvec2& p, const generator_context::image& img)
{
    glm::dvec2 fl (glm::floor(p));
    glm::dvec2 fr (p - fl);

    glm::ivec2 i (fr.x * img.width, fr.y * img.height);
    return ((double)img.buffer[i.y * img.width + i.x] - 127.5) / 127.5;
}

double
generator_slowinterpreter::eval_v (const node& n)
{
    if (n.type == node::const_var)
        return n.aux_var;

    auto& in (n.input[0]);

    switch (n.type)
    {
    case node::angle:
    {
        auto p (eval_xy(in));
        return std::atan2(p.y, p.x) / pi;
    }
    case node::chebyshev:
    {
        auto p (eval_xy(in));
        return std::max(p.x, p.y);
    }

    case node::checkerboard:
    {
        auto p (eval_xy(in));
        auto fl (glm::floor(p));
        auto fr (p - fl);
        return ((fr.x < 0.5 && fr.y < 0.5) || (fr.x >= 0.5 && fr.y >= 0.5))
                ? 1 : -1;
    }

    case node::distance:
    {
        auto p (eval_xy(in));
        return std::sqrt(p.x*p.x + p.y*p.y);
    }

    case node::perlin:
    {
        auto p (eval_xy(in));
        auto seed (eval_v(n.input[1]));
        return p_perlin(p, seed);
    }

    case node::worley:
    {
        auto p (eval_xy(in));
        auto seed (eval_v(n.input[2]));

        auto tmp (p_);
        p_ = p_worley(p, seed);
        auto result (eval_v(n.input[1]));
        p_ = tmp;
        return result;
    }

    case node::voronoi:
    {
        auto p (eval_xy(in));
        auto seed (eval_v(n.input[2]));

        auto tmp (p_);
        p_ = p_voronoi(p, seed);
        auto result (eval_v(n.input[1]));
        p_ = tmp;
        return result;
    }

    case node::external_:
    {
        auto tmp (p_);
        p_ = eval_xy(in);
        auto result (eval_v(cntx_.get_script(n.aux_string)));
        p_ = tmp;
        return result;
    }

    case node::lambda_:
    {
        auto tmp (p_);
        p_ = eval_xy(in);
        auto result (eval_v(n.input[1]));
        p_ = tmp;
        return result;
    }

    case node::manhattan:
    {
        auto p (eval_xy(in));
        return std::max(std::abs(p.x), std::abs(p.y));
    }

    case node::x:
        return eval_xy(in).x;

    case node::y:
        return eval_xy(in).y;

    case node::fractal:
    {
        auto tmp(p_);
        p_ = eval_xy(n.input[0]);

        auto& f(n.input[1]);
        int  octaves(eval_v(n.input[2]));

        octaves = std::min(octaves, INTERPRETER_OCTAVES_LIMIT);

        double lacunarity(eval_v(n.input[3]));
        double persistence(eval_v(n.input[4]));

        double div(0.0), mul(1.0), result(0.0);
        for (int i(0); i < octaves; ++i)
        {
            result += eval_v(f) * mul;
            div += mul;
            mul *= lacunarity;
            p_ *= persistence;
        }
        p_ = tmp;
        return result / div;
    }

    case node::abs:
        return std::abs(eval_v(in));

    case node::add:
        return eval_v(in) + eval_v(n.input[1]);

    case node::blend:
    {
        double l ((eval_v(in) + 1.0) / 2.0);
        double a (eval_v(in)), b (eval_v(n.input[1]));
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

    case node::saw:
    {
        auto v (eval_v(in));
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
        return (eval_bool(n.input[0])) ?
                    eval_v(n.input[1]) : eval_v(n.input[2]);

    case node::curve_linear:
        return curve_linear(eval_v(in), n.curve);

    case node::curve_spline:
        return curve_spline(eval_v(in), n.curve);

    case node::png_lookup:
        return png(eval_xy(in), cntx_.get_image(n.input[1].aux_string));

    case node::simplex:
        throw std::runtime_error("'simplex' not implemented yet");

    default:
        throw std::runtime_error("type mismatch");
    }
}

glm::dvec2
generator_slowinterpreter::eval_xy (const node& n)
{
    switch (n.type)
    {
    case node::entry_point:
        return p_;

    case node::rotate:
    {
        auto p (eval_xy(n.input[0]));
        auto t (eval_v(n.input[1]) * pi);
        auto ct (std::cos(t));
        auto st (std::sin(t));
        return glm::dvec2(p.x*ct-p.y*st, p.x*st+p.y*ct);
    }

    case node::scale:
    {
        auto p (eval_xy(n.input[0]));
        auto s (eval_v(n.input[1]));
        return glm::dvec2(p.x / s, p.y / s);
    }

    case node::shift:
    {
        auto p (eval_xy(n.input[0]));
        auto sx (eval_v(n.input[1]));
        auto sy (eval_v(n.input[2]));
        return glm::dvec2(p.x + sx, p.y + sy);
    }

    case node::map:
    {
        auto tmp(p_);
        p_ = eval_xy(n.input[0]);
        auto x (eval_v(n.input[1]));
        auto y (eval_v(n.input[2]));
        p_ = tmp;
        return glm::dvec2(x, y);
    }

    case node::turbulence:
    {
        auto p (eval_xy(n.input[0]));
        auto x (eval_v(n.input[1]));
        auto y (eval_v(n.input[2]));

        return glm::dvec2(p.x + x, p.y + y);
    }

    case node::swap:
    {
        auto p (eval_xy(n.input[0]));
        return glm::dvec2(p.y, p.x);
    }

    default:
        throw std::runtime_error("type mismatch");
    }
}

bool
generator_slowinterpreter::eval_bool (const node& n)
{
    switch (n.type)
    {
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

    default:
        throw std::runtime_error("type mismatch");
    }
}


}}

