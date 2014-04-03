//---------------------------------------------------------------------------
/// \file   hexanoise/node.hpp
/// \brief  Single node in a compiled HNDL script
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>

namespace hexa {
namespace noise {

class function;
class generator_context;

/** Variable types. */
typedef enum
{
    /** 2-D coordinates */
    xy,
    /** Scalar value */
    var,
    /** Text string (only used as a constexpr) */
    string,
    /** Boolean value */
    boolean
}
var_t;

/** A node in the expression tree.
 *  A compiled HDNL script is represented by a node. */
class node
{
public:

    typedef enum
    {
        entry_point,
        const_var,
        const_str,
        const_bool,

        funcdef_xy_xy,

        rotate,
        scale,
        shift,
        swap,
        map,
        turbulence,

        funcdef_xy_v,

        angle,
        chebyshev,
        checkerboard,
        distance,
        external_,
        lambda_,
        manhattan,
        fractal,
        perlin,
        png_lookup,
        simplex,
        voronoi,
        worley,
        x,
        y,

        funcdef_v_v,

        abs,
        add,
        blend,
        cos,
        curve_spline,
        curve_linear,
        div,
        max,
        min,
        mul,
        neg,
        pow,
        saw,
        sin,
        sqrt,
        sub,
        tan,

        funcdef_bool_bool,

        band,
        bnot,
        bor,
        bxor,

        funcdef_v_bool,

        is_equal,
        is_greaterthan,
        is_gte,
        is_lessthan,
        is_lte,

        funcdef_xy_bool,

        is_in_rectangle,
        is_in_circle,

        funcdef_bool_xy,

        then_else
    }
    func_t;

    func_t  type;
    std::vector<node> input;
    var_t   return_type;
    bool    is_const;

    std::string     aux_string;
    double          aux_var;
    bool            aux_bool;

    node (function* in, const generator_context& ctx);
    node (func_t t, bool c = false, var_t rt = var);

    explicit node (double value)
        : type(const_var), return_type(var), is_const(true), aux_var(value)
    { }

    explicit node (const std::string& value)
        : type(const_str), return_type(string), is_const(true), aux_string(value)
    { }

    explicit node (bool value)
        : type(const_bool), return_type(boolean), is_const(true), aux_bool(value)
    { }

    node (const node&) = default;
    node& operator= (const node&) = default;

    // Visual Studio 2013 still doesn't support default move :/
    node (node&& m)
        : type(m.type)
        , input(std::move(m.input))
        , return_type(m.return_type)
        , is_const(m.is_const)
        , aux_string(std::move(m.aux_string))
        , aux_var(m.aux_var)
        , aux_bool(m.aux_bool)
    { }
};

}} // namespace hexa::noise

