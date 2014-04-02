//---------------------------------------------------------------------------
// hexanoise/node.cpp
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include "node.hpp"

#include <stdexcept>
#include <unordered_map>
#include "ast.hpp"
#include "generator_context.hpp"

namespace hexa {
namespace noise {

struct paramdef
{
    std::string name;
    var_t       type;

    bool        is_optional;
    double      default_value;

    paramdef(var_t t)
        : type (t), is_optional(false) { }

    paramdef(const std::string& n, var_t t)
        : name (n), type (t), is_optional(false) { }

    paramdef(const std::string& n, var_t t, double d)
        : name (n), type (t), is_optional(true), default_value(d) { }
};

struct funcdef
{
    node::func_t    id;
    var_t           return_type;
    bool            is_const;

    std::vector<paramdef>   parameters;
};

class global_visitor : public boost::static_visitor<node>
{
public:
    template <typename t>
    node operator() (const t& s) const
    {
        return node(s);
    }
};

static const std::unordered_map<std::string, funcdef> functions
{
    { "!entry", { node::entry_point,xy, false,    {{ xy }}} },
    { "!var",   { node::const_var,  var, true,     {{ xy }}} },
    { "!str",   { node::const_str,  string, true,  {{ xy }}} },
    { "!bool",  { node::const_bool, boolean, true, {{ xy }}} },

    { "rotate", { node::rotate, xy, false, {{ xy },
                                            { "angle", var } }} },
    { "scale",  { node::scale,  xy, false, {{ xy },
                                            { "div", var }}} },
    { "shift",  { node::shift,  xy, false, {{ xy },
                                            { "add_x", var },
                                            { "add_y", var }}} },
    { "swap",   { node::swap,   xy, false, {{ xy }}} },
    { "turbulence", { node::turbulence, xy, false, {{ xy }, { "x", var }, { "y", var } }} },
    { "map", { node::map, xy, false, {{ xy }, { "x", var }, { "y", var } }} },

    { "angle", { node::angle,   var, false, {{ xy } }} },
    { "chebyshev", { node::chebyshev,   var, false, {{ xy } }} },
    { "checkerboard", { node::checkerboard, var, false, {{ xy } }} },
    { "distance", { node::distance, var, false, {{ xy } }} },
    { "manhattan", { node::manhattan,   var, false, {{ xy } }} },
    { "fractal", { node::fractal,   var, false, {{ xy },
                                               { "noise_function", var },
                                               { "octaves", var, 2 },
                                               { "lacunarity", var, 0.5, },
                                               { "persistence", var, 2.0} }} },

    { "perlin", { node::perlin, var, false, {{ xy }, {"seed", var, 0} }} },

    { "png_lookup", { node::png_lookup, var, false, {{ xy },
                                                { "filename", string } }} },

    { "simplex", { node::simplex,   var, false, {{ xy }, {"seed", var, 0} }} },
    { "worley", { node::worley, var, false, {{ xy }, {"seed", var, 0} }} },
    { "x", { node::x,   var, false, {{ xy } }} },
    { "y", { node::y,   var, false, {{ xy } }} },

    { "add", { node::add, var, false, {{ var }, { "n", var, 1.0 } }} },
    { "sub", { node::sub, var, false, {{ var }, { "n", var, 1.0 } }} },
    { "mul", { node::mul, var, false, {{ var }, { "n", var, 2.0 } }} },
    { "div", { node::div, var, false, {{ var }, { "n", var, 2.0 } }} },

    { "abs", { node::abs, var, false, {{ var } }} },
    { "blend", { node::blend, var, false, {{ var }, {"a",var}, {"b",var} }} },
    { "cos", { node::cos, var, false, {{ var }, }} },
    { "min", { node::min, var, false, {{ var }, { "n", var } }} },
    { "max", { node::max, var, false, {{ var }, { "n", var } }} },
    { "neg", { node::neg, var, false, {{ var } }} },
    { "pow", { node::pow, var, false, {{ var }, { "n", var, 2.0 } }} },
    { "saw", { node::saw, var, false, {{ var } }} },
    { "sin", { node::sin, var, false, {{ var } }} },
    { "sqrt",{ node::sqrt,var, false, {{ var } }} },
    { "tan", { node::tan, var, false, {{ var } }} },

    { "and", { node::band, boolean, false, {{ boolean }, { "x", boolean }}} },
    { "or",  { node::bor,  boolean, false, {{ boolean }, { "x", boolean }}} },
    { "xor", { node::bxor, boolean, false, {{ boolean }, { "x", boolean }}} },
    { "not", { node::bnot, boolean, false, {{ boolean } }} },

    { "is_equal",  { node::is_equal, boolean, false, {{ var }, {"c", var} }} },
    { "is_greaterthan",  { node::is_greaterthan, boolean, false, {{ var }, {"c", var} }} },
    { "is_gte",  { node::is_gte, boolean, false, {{ var }, {"c", var} }} },
    { "is_lessthan",  { node::is_lessthan, boolean, false, {{ var }, {"c", var} }} },
    { "is_lte",  { node::is_lte, boolean, false, {{ var }, {"c", var} }} },

    { "is_in_circle",  { node::is_in_circle, boolean, false, {{ xy }, {"radius", var} }} },
    { "is_in_rectangle",  { node::is_in_rectangle, boolean, false, {{ xy },
                                {"x1", var}, {"y1", var}, {"x2", var}, {"y2", var} }} },

    { "then_else",  { node::then_else, var, false, {{ boolean }, {"a", var}, {"b", var} }} }

};

node::node (function* in, const generator_context& ctx)
{
    switch (in->type)
    {
        case function::func:
        {
            auto f (functions.find(in->name));
            if (f == functions.end())
                throw std::runtime_error("unknown function " + in->name);

            auto& fdef (f->second);
            type = fdef.id;
            return_type = fdef.return_type;

            if (in->input)
                input.emplace_back(node(in->input, ctx));
            else
                input.emplace_back(node(entry_point, false, xy));

            if (in->args)
            {
                if (in->args->size() >= fdef.parameters.size())
                    throw std::runtime_error(in->name + ": too many parameters");

                for (auto ptr : *(in->args))
                    input.emplace_back(node(ptr, ctx));
            }

            auto chk (input.begin());
            auto ck2 (fdef.parameters.begin());

            for (; chk != input.end() && ck2 != fdef.parameters.end(); ++chk, ++ck2)
            {
                if (chk->return_type != ck2->type)
                {
                    throw std::runtime_error(in->name + " " + ck2->name + ": type mismatch");
                }
            }
            for (; ck2 != fdef.parameters.end(); ++ck2)
            {
                if (!ck2->is_optional)
                {
                    throw std::runtime_error(in->name + ": parameter " + ck2->name + " missing");
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
            aux_string = in->name;
            break;

        case function::const_bool:
            type = const_bool;
            return_type = boolean;
            is_const = true;
            aux_bool = in->value > 0;
            break;

        case function::global:
            {
            if (!ctx.exists_global(in->name))
                throw std::runtime_error("global variable '" + in->name + "' not defined");

            auto& global (ctx.get_global(in->name));
            *this = boost::apply_visitor(global_visitor(), global);
            }
            break;

        case function::lambda:
            if (!in->args || in->args->size() != 1)
                throw std::runtime_error("lambda function must take one input parameter");

            type = lambda_;
            return_type = var;
            if (in->input)
                input.emplace_back(node(in->input, ctx));
            else
                input.emplace_back(node(entry_point, false, xy));

            input.emplace_back(node((*in->args)[0], ctx));

            if (input.back().return_type != var)
                throw std::runtime_error("lambda function must return a scalar");

            break;

        case function::external:
            break;
    }
}

node::node (func_t t, bool c, var_t rt)
    : type(t), return_type(rt), is_const(c)
{ }

}}

