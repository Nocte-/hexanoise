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

namespace hexa {
namespace noise {

class function
{
public:
    typedef enum
    {
        func, const_v, const_s, const_bool, global, lambda, external
    }
    types;

    types                   type;
    std::string             name;
    function*               input;
    std::vector<function*>* args;
    double                  value;
    std::string             text;

    size_t                  line;
    size_t                  pos;

    function(const std::string& n = std::string())
        : type(func), name(n), input(0), args(0) { }

    ~function()
    {
        delete input;
        if (args)
        {
            for (auto p : *args)
                delete p;
        }

        delete args;
    }
};

}}

