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

namespace hexa
{
namespace noise
{

class node;

/** A rather slow interpreter for noise scripts. */
class generator_slowinterpreter : public generator_i
{
public:
    /** Set up an interpreter
     * @param context  Shared data
     * @param n        The compiled noise script to execute
     */
    generator_slowinterpreter(const generator_context& context, const node& n);

    std::vector<double> run(const glm::dvec2& corner, const glm::dvec2& step,
                            const glm::ivec2& count) override;

    std::vector<int16_t> run_int16(const glm::dvec2& corner,
                                   const glm::dvec2& step,
                                   const glm::ivec2& count) override;

private:
    double eval(const glm::dvec2& p, const node& n);

    double eval_v(const node& n);
    glm::dvec2 eval_xy(const node& n);
    bool eval_bool(const node& n);

private:
    const node& n_;
    glm::dvec2 p_;
    uint32_t seed_;
};
}
}
