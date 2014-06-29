//---------------------------------------------------------------------------
/// \file   hexanoise/ast.hpp
/// \brief  Abstract tree built by parser.y
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace hexa
{
namespace noise
{

/** Intermediate representation of an HNDL script.
 *  This class is only used by the Bison parser.  The library itself
 *  uses the node class to represent the syntax tree.  The compiler
 *  recurses down the function objects, looks up function definitions,
 *  checks all types, looks up external functions and global variables,
 *  and builds a hexa::noise::node object. */
class function
{
public:
    /** Function type. */
    typedef enum {
        /** Normal function.
         *  'name' is the identifier, 'input' has the curry arguments. */
        func,

        /** Const variable.
         *  'name' and 'input' are empty, 'value' holds the variable. */
        const_v,

        /** Const string.
         *  'name' and 'input' are empty, 'text' holds the variable. */
        const_s,

        /** Const boolean.
         *  'name' and 'input' are empty, 'value' holds the variable. */
        const_bool,

        /** Global variable.
         *  'name' holds the name of the variable, 'value' is still empty. */
        global,

        /** Lambda function.
         *  Works like a normal function, but 'name' is empty.  The first
         *  argument is the lambda function itself. */
        lambda,

        /** External function.
         *  'name' holds the name of the external function.  The input
         *  parameters are empty. */
        external
    } types;

    types type;
    std::string name;
    function* input;
    std::vector<function*>* args;
    double value;

    size_t line;
    size_t pos;

    function(const std::string& n = std::string())
        : type(func)
        , name(n)
        , input(0)
        , args(0)
    {
    }

    ~function()
    {
        delete input;
        if (args) {
            for (auto p : *args)
                delete p;
        }

        delete args;
    }
};

} // namespace noise
} // namespace hexa
