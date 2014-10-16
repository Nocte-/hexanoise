//---------------------------------------------------------------------------
/// \file   hexanoise/node.hpp
/// \brief  Single node in a compiled HNDL script
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>

namespace hexa
{
namespace noise
{

class function;
class generator_context;

/** Variable types. */
typedef enum {
    /** No value */
    none,
    /** Scalar value */
    var,
    /** 2-D coordinates */
    xy,
    /** 3-D coordinates */
    xyz,
    /** Text string (only used as a constexpr) */
    string,
    /** Boolean value */
    boolean,
    /** External value (unknown at compile time) */
    external
} var_t;

/** A node in the expression tree.
 *  A compiled HDNL script is represented by a node, which is the root of
 *  the expression tree.  The root is the last step, the leaves are the
 *  entry points where the coordinates are taken as input. */
class node
{
public:
    typedef enum {
        entry_point,
        const_var,
        const_str,
        const_bool,

        function_list,
        
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
        opensimplex,
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
        round,
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

        then_else,

        funcdef_xyz_xyz,

        rotate3,
        scale3,
        shift3,
        map3,
        turbulence3,

        funcdef_xy_xyz,

        xplane,
        yplane,
        zplane,

        funcdef_xyz_xy,

        xy,

        funcdef_xyz_v,

        chebyshev3,
        checkerboard3,
        distance3,
        manhattan3,
        fractal3,
        perlin3,
        simplex3,
        opensimplex3,
        worley3,
        z

    } func_t;

    /** Defines a control point in an adjustment curve. */
    struct control_point
    {
        double in;
        double out;

        control_point(const std::pair<double, double>& p)
            : in(p.first)
            , out(p.second)
        {
        }
    };

public:
    /** The type of this node. */
    func_t type;
    /** The inputs (or: the child nodes in the tree structure) */
    std::vector<node> input;
    /** The output type of this node. 
     *  In a well-formed expression tree, the output types of the child nodes
     *  should match the input types of the parameters of this node. */
    var_t return_type;
    /** Flag for const expressions (values, booleans, and strings) */
    bool is_const;

    /** Holds a string constant. */
    std::string aux_string;
    /** Holds a double constant. */
    double aux_var;
    /** Holds a boolean constant. */
    bool aux_bool;

    /** Adjustment curve.
     *  This is basically an interpolated lookup table. */
    std::vector<control_point> curve;

public:
    /** Compile an expression tree from the Flex/Bison output. */
    node(function* in, const generator_context& ctx);
    
    node(func_t t, bool c = false, var_t rt = var);

    /** Construct a constant value express. */
    explicit node(double value)
        : type(const_var)
        , return_type(var)
        , is_const(true)
        , aux_var(value)
    {
    }

    /** Construct a constant string expression. */
    explicit node(const std::string& value)
        : type(const_str)
        , return_type(string)
        , is_const(true)
        , aux_string(value)
    {
    }

    /** Construct a constant boolean expression. */    
    explicit node(bool value)
        : type(const_bool)
        , return_type(boolean)
        , is_const(true)
        , aux_bool(value)
    {
    }

    node(const node&) = default;
    node& operator=(const node&) = default;

    // Visual Studio 2013 still doesn't support default move :/
    node(node&& m)
        : type(m.type)
        , input(std::move(m.input))
        , return_type(m.return_type)
        , is_const(m.is_const)
        , aux_string(std::move(m.aux_string))
        , aux_var(m.aux_var)
        , aux_bool(m.aux_bool)
        , curve(std::move(m.curve))
    {
    }

    /** Get references to all leaves of this expression tree. */
    std::vector<std::reference_wrapper<node>> entry_points();
    
    std::vector<std::reference_wrapper<const node>> entry_points() const;
    
    /** Determine the input type of this function.
     *  This is either var_t::xy or var_t::xyz.  If the endpoints() are
     *  all of type 'xy', the whole expression is 2-D.  Otherwise, the
     *  expression expects a 3-D input. */
    var_t input_type() const;
};

} // namespace noise
} // namespace hexa
