//---------------------------------------------------------------------------
/// \file   hexanoise/generator_slowinterpreter.hpp
/// \brief  Interprets a compiled HNDL script
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#pragma once

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <glm/glm.hpp>

#include "generator_i.hpp"

namespace hexa {
namespace noise {

class node;

class generator_slowinterpreter : public generator_i
{
public:
    generator_slowinterpreter (const generator_context& context,
                               const node& n);

    std::vector<double>
    run (const glm::dvec2& corner,
         const glm::dvec2& step,
         const glm::ivec2& count) override;

private:
    double     eval (const glm::dvec2& p, const node& n);

    double     eval_v  (const node& n);
    glm::dvec2 eval_xy (const node& n);
    bool       eval_bool(const node& n);

private:
    const node& n_;
    glm::dvec2 p_;
};

}}

