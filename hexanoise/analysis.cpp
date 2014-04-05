//---------------------------------------------------------------------------
// hexanoise/analysis.cpp
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include "analysis.hpp"

#include "node.hpp"

namespace hexa {
namespace noise {

size_t
fractal_depth (const node& n)
{
    return 0;
}

size_t
weight (const node& n)
{
    size_t result;

    switch (n.type)
    {
        case node::entry_point:
        case node::const_var:
        case node::const_str:
        case node::const_bool:
            return 0;

        case node::fractal:
            return n.input[2].aux_var * weight(n.input[1]) + weight(n.input[0]);

        case node::then_else:
            return weight(n.input[0]) + std::max(weight(n.input[1]), weight(n.input[2]));

        case node::perlin:
        case node::worley:
        case node::voronoi:
        case node::simplex:
            result = 5; break;

        default:
            result = 1;
    }

    for (auto& p : n.input)
        result += weight(p);

    return result;
}

void
referred_images (const node& n, std::unordered_set<std::string>& in)
{
    if (n.type == node::png_lookup)
        in.insert(n.input[1].aux_string);

    for (auto& p : n.input)
        referred_images(p, in);
}

std::unordered_set<std::string>
referred_images (const node& n)
{
    std::unordered_set<std::string> result;
    referred_images(n, result);
    return result;
}

void
referred_scripts (const node& n, std::unordered_set<std::string>& in)
{
    if (n.type == node::external_)
        in.insert(n.input[1].aux_string);

    for (auto& p : n.input)
        referred_images(p, in);
}

std::unordered_set<std::string>
referred_scripts (const node& n)
{
    std::unordered_set<std::string> result;
    referred_scripts(n, result);
    return result;
}

}} // namespace hexa::noise

