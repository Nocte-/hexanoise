//---------------------------------------------------------------------------
// hexanoise/generator_slowinterpreter.cpp
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include "generator_slowinterpreter.hpp"

#include <cmath>
#include <stdexcept>
#include "node.hpp"

namespace hexa {
namespace noise {

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
        return std::atan2(p.y, p.x) / M_PI;
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
        return std::sqrt(glm::dot(p, p));
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
        return std::cos(eval_v(in) * M_PI);

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
        return std::sin(eval_v(in) * M_PI);

    case node::sqrt:
        return std::sqrt(eval_v(in));

    case node::sub:
        return eval_v(in) - eval_v(n.input[1]);

    case node::tan:
        return std::tan(eval_v(in) * M_PI);

    case node::then_else:
        return (eval_bool(n.input[0])) ?
                    eval_v(n.input[1]) : eval_v(n.input[2]);

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
        auto t (eval_v(n.input[1]) * M_PI);
        auto ct (std::cos(t));
        auto st (std::sin(t));
        return glm::dvec2(p.x*ct-p.y*st, p.x*st+p.y*ct);
    }

    case node::scale:
        return eval_xy(n.input[0]) * eval_v(n.input[1]);

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

