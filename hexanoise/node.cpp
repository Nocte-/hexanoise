//---------------------------------------------------------------------------
// hexanoise/node.cpp
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include "node.hpp"

#include <map>
#include <stdexcept>
#include <unordered_map>
#include "ast.hpp"
#include "generator_context.hpp"

namespace hexa
{
namespace noise
{

struct paramdef
{
    std::string name;
    var_t type;

    bool is_optional;
    double default_value;

    paramdef(var_t t)
        : type(t)
        , is_optional(false)
    {
    }

    paramdef(const std::string& n, var_t t)
        : name(n)
        , type(t)
        , is_optional(false)
    {
    }

    paramdef(const std::string& n, var_t t, double d)
        : name(n)
        , type(t)
        , is_optional(true)
        , default_value(d)
    {
    }
};

struct funcdef
{
    node::func_t id;
    var_t return_type;
    bool is_const;

    std::vector<paramdef> parameters;

    // Fix a VS2013 bug:
    funcdef(node::func_t i, const var_t& rt, bool ic,
            const std::vector<paramdef>& pd)
        : id(i)
        , return_type(rt)
        , is_const(ic)
        , parameters(pd)
    {
    }

    funcdef(node::func_t i, const var_t& rt, const std::vector<paramdef>& pd)
        : id(i)
        , return_type(rt)
        , is_const(false)
        , parameters(pd)
    {
    }
};

class global_visitor : public boost::static_visitor<node>
{
public:
    template <typename t>
    node operator()(t s) const
    {
        return node(s);
    }
};

static const std::unordered_map<std::string, funcdef> functions{
    {"!entry", {node::entry_point, xy, false, {}}},
    {"!var", {node::const_var, var, true, {}}},
    {"!str", {node::const_str, string, true, {}}},
    {"!bool", {node::const_bool, boolean, true, {}}},
    {"rotate", {node::rotate, xy, {xy, {"angle", var}}}},
    {"scale", {node::scale, xy, {xy, {"div", var}}}},
    {"shift", {node::shift, xy, {xy, {"add_x", var}, {"add_y", var}}}},
    {"swap", {node::swap, xy, {xy}}},
    {"turbulence", {node::turbulence, xy, {xy, {"x", var}, {"y", var}}}},
    {"map", {node::map, xy, {xy, {"x", var}, {"y", var}}}},
    {"angle", {node::angle, var, {xy}}},
    {"chebyshev", {node::chebyshev, var, {xy}}},
    {"checkerboard", {node::checkerboard, var, {xy}}},
    {"distance", {node::distance, var, {xy}}},
    {"manhattan", {node::manhattan, var, {xy}}},
    {"fractal",
     {node::fractal,
      var,
      {xy,
       {"noise_function", var},
       {"octaves", var, 2},
       {"lacunarity", var, 0.5},
       {"persistence", var, 2.0}}}},
    {"perlin", {node::perlin, var, {xy, {"seed", var, 0}}}},
    {"png_lookup", {node::png_lookup, var, {xy, {"filename", string}}}},
    {"simplex", {node::simplex, var, {xy, {"seed", var, 0}}}},
    {"opensimplex", {node::opensimplex, var, {xy, {"seed", var, 0}}}},
    {"voronoi", {node::voronoi, var, {xy, {"func", var}, {"seed", var, 0}}}},
    {"worley", {node::worley, var, {xy, {"func", var}, {"seed", var, 0}}}},
    {"x", {node::x, var, {xy}}},
    {"y", {node::y, var, {xy}}},
    {"add", {node::add, var, {var, {"n", var, 1.0}}}},
    {"sub", {node::sub, var, {var, {"n", var, 1.0}}}},
    {"mul", {node::mul, var, {var, {"n", var, 2.0}}}},
    {"div", {node::div, var, {var, {"n", var, 2.0}}}},
    {"abs", {node::abs, var, {var}}},
    {"blend", {node::blend, var, {var, {"a", var}, {"b", var}}}},
    {"cos", {node::cos, var, {var}}},
    {"min", {node::min, var, {var, {"n", var, 0.0}}}},
    {"max", {node::max, var, {var, {"n", var, 0.0}}}},
    {"neg", {node::neg, var, {var}}},
    {"pow", {node::pow, var, {var, {"n", var, 2.0}}}},
    {"round", {node::round, var, {var}}},
    {"saw", {node::saw, var, {var}}},
    {"sin", {node::sin, var, {var}}},
    {"sqrt", {node::sqrt, var, {var}}},
    {"tan", {node::tan, var, {var}}},
    {"and", {node::band, boolean, {boolean, {"x", boolean}}}},
    {"or", {node::bor, boolean, {boolean, {"x", boolean}}}},
    {"xor", {node::bxor, boolean, {boolean, {"x", boolean}}}},
    {"not", {node::bnot, boolean, {boolean}}},
    {"is_equal", {node::is_equal, boolean, {var, {"c", var}}}},
    {"is_greaterthan", {node::is_greaterthan, boolean, {var, {"c", var}}}},
    {"is_gte", {node::is_gte, boolean, {var, {"c", var}}}},
    {"is_lessthan", {node::is_lessthan, boolean, {var, {"c", var}}}},
    {"is_lte", {node::is_lte, boolean, {var, {"c", var}}}},
    {"is_in_circle", {node::is_in_circle, boolean, {xy, {"radius", var}}}},
    {"is_in_rectangle",
     {node::is_in_rectangle,
      boolean,
      {xy, {"x1", var}, {"y1", var}, {"x2", var}, {"y2", var}}}},
    {"then_else",
     {node::then_else, var, {boolean, {"a", var, 1.0}, {"b", var, -1.0}}}},
    {"curve_linear", {node::curve_linear, var, {var}}},
    {"curve_spline", {node::curve_spline, var, {var}}},

    {"rotate3", {node::rotate3, xyz, {xyz, {"axis", xyz}, {"angle", var}}}},
    {"scale3", {node::scale3, xyz, {xyz, {"div", var}}}},
    {"shift3",
     {node::shift3,
      xyz,
      {xyz, {"add_x", var}, {"add_y", var}, {"add_z", var}}}},
    {"turbulence3",
     {node::turbulence3, xyz, {xyz, {"x", var}, {"y", var}, {"z", var}}}},
    {"map3", {node::map3, xyz, {xyz, {"x", var}, {"y", var}, {"z", var}}}},
    {"fractal3",
     {node::fractal3,
      var,
      {xyz,
       {"noise_function", var},
       {"octaves", var, 2},
       {"lacunarity", var, 0.5},
       {"persistence", var, 2.0}}}},
    {"xplane", {node::zplane, xyz, {xy, {"x", var, 0.0}}}},
    {"yplane", {node::zplane, xyz, {xy, {"y", var, 0.0}}}},
    {"zplane", {node::zplane, xyz, {xy, {"z", var, 0.0}}}},
    {"xy", {node::xy, xy, {xyz}}},
    {"z", {node::z, var, {xyz}}},
    {"chebyshev3", {node::chebyshev3, var, {xyz}}},
    {"checkerboard3", {node::checkerboard3, var, {xyz}}},
    {"distance3", {node::distance3, var, {xyz}}},
    {"manhattan3", {node::manhattan3, var, {xyz}}},
    {"perlin3", {node::perlin3, var, {xyz, {"seed", var, 0}}}},
    {"simplex3", {node::simplex3, var, {xyz, {"seed", var, 0}}}},
    {"opensimplex3", {node::opensimplex3, var, {xyz, {"seed", var, 0}}}}
};

bool is_coordinate(var_t v)
{
    return v == var_t::xy || v == var_t::xyz;
}

bool types_match(var_t lhs, var_t rhs)
{
    return lhs == rhs || (lhs == var_t::external && is_coordinate(rhs));
}

node::node(function* in, const generator_context& ctx)
{
    switch (in->type) {
    case function::func: {
        is_const = false;

        auto f = functions.find(in->name);

        if (f == functions.end())
            throw std::runtime_error("unknown function " + in->name);

        auto& fdef = f->second;
        type = fdef.id;
        return_type = fdef.return_type;

        if (in->input) {
            input.emplace_back(in->input, ctx);
        } else {
            input.emplace_back(node::entry_point, false,
                               fdef.parameters.front().type);
        }

        // Special case: curve_linear and curve_spline take a list of
        // const var parameters.
        //
        if (type == node::curve_linear || type == node::curve_spline) {
            std::vector<double> params;
            for (auto ptr : *(in->args)) {
                if (ptr->type != function::const_v)
                    throw std::runtime_error(
                        "curve_* only takes const variables as parameters");

                params.push_back(ptr->value);
            }
            if (params.size() % 2 != 0)
                throw std::runtime_error(
                    "curve_* must have an even number of parameters");

            if (type == node::curve_linear && params.size() < 4)
                throw std::runtime_error(
                    "curve_linear must have at least 4 parameters");

            if (type == node::curve_spline && params.size() < 8)
                throw std::runtime_error(
                    "curve_spline must have at least 8 parameters");

            std::map<double, double> result;
            for (auto i(params.begin()); i != params.end(); i += 2)
                result[*i] = *std::next(i);

            for (auto& p : result)
                curve.push_back(control_point(p));

            break;
        }

        // Handle parameters for all other functions.
        //
        if (in->args) {
            if (in->args->size() >= fdef.parameters.size())
                throw std::runtime_error(in->name + ": too many parameters");

            for (auto ptr : *(in->args))
                input.emplace_back(ptr, ctx);
        }

        auto chk = input.begin();
        auto ck2 = fdef.parameters.begin();

        for (; chk != input.end() && ck2 != fdef.parameters.end();
             ++chk, ++ck2) {
            if (!types_match(chk->return_type, ck2->type)) {
                throw std::runtime_error(in->name + " " + ck2->name
                                         + ": type mismatch");
            }
        }
        for (; ck2 != fdef.parameters.end(); ++ck2) {
            if (!ck2->is_optional) {
                throw std::runtime_error(in->name + ": parameter " + ck2->name
                                         + " missing");
            }
            input.emplace_back(node(ck2->default_value));
        }
        break;
    }

    case function::const_v:
        type = const_var;
        return_type = var;
        is_const = true;
        aux_var = in->value;
        break;

    case function::const_s:
        type = const_str;
        return_type = string;
        is_const = true;
        aux_string = in->name.substr(1, in->name.size() - 2);
        break;

    case function::const_bool:
        type = const_bool;
        return_type = boolean;
        is_const = true;
        aux_bool = in->value > 0;
        break;

    case function::global: {
        is_const = true;
        if (!ctx.exists_global(in->name))
            throw std::runtime_error("global variable '" + in->name
                                     + "' not defined");

        auto global = ctx.get_global(in->name);
        *this = boost::apply_visitor(global_visitor(), global);
    } break;

    case function::lambda: {
        is_const = false;
        if (!in->args || in->args->size() != 1)
            throw std::runtime_error(
                "lambda function must take one input parameter");

        type = lambda_;
        return_type = var;

        node lambda{(*in->args)[0], ctx};
        if (lambda.return_type != var)
            throw std::runtime_error("lambda function must return a scalar");

        auto lambda_input_t = lambda.input_type();
        if (!is_coordinate(lambda_input_t))
            throw std::runtime_error("lambda function must take a coordinate");

        if (in->input)
            input.emplace_back(in->input, ctx);
        else
            input.emplace_back(entry_point, false, lambda_input_t);

        input.emplace_back(std::move(lambda));
    } break;

    case function::external:
        is_const = false;
        type = external_;
        return_type = var;
        aux_string = in->name;

        if (in->input)
            input.emplace_back(in->input, ctx);
        else
            input.emplace_back(entry_point, false, var_t::external);

        break;
    }
}

node::node(func_t t, bool c, var_t rt)
    : type(t)
    , return_type(rt)
    , is_const(c)
{
}

std::vector<std::reference_wrapper<const node>> node::entry_points() const
{
    std::vector<std::reference_wrapper<const node>> result;

    if (is_const) {
        return result;
    }

    if (type == entry_point) {
        result.emplace_back(*this);
    } else {
        for (auto& i : input) {
            auto additional = i.entry_points();
            result.insert(result.end(), additional.begin(), additional.end());
        }
    }
    return result;
}

std::vector<std::reference_wrapper<node>> node::entry_points()
{
    std::vector<std::reference_wrapper<node>> result;

    if (is_const) {
        return result;
    }

    if (type == entry_point) {
        result.emplace_back(*this);
    } else {
        for (auto& i : input) {
            auto additional = i.entry_points();
            result.insert(result.end(), additional.begin(), additional.end());
        }
    }
    return result;
}

var_t node::input_type() const
{
    auto eps = entry_points();
    if (eps.empty()) {
        return var_t::none;
    }
    for (const node& i : eps) {
        if (i.return_type == var_t::xyz) {
            return var_t::xyz;
        }
    }
    return var_t::xy;
}

} // namespace noise
} // namespace hexa
