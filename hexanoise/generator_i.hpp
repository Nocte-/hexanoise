//---------------------------------------------------------------------------
/// \file   hexanoise/generator_i.hpp
/// \brief  Base class for noise generators
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "generator_context.hpp"

namespace hexa {
namespace noise {

class generator_i
{
public:
    generator_i (const generator_context& c) : cntx_(c) { }
    virtual ~generator_i() { }

    const generator_context::image&
    image (const std::string& name) const;

    const generator_context::variable&
    global (const std::string& name) const;

    bool
    have_blobal (const std::string& name) const;

    virtual std::vector<double>
    run (const glm::dvec2& corner,
         const glm::dvec2& step,
         const glm::ivec2& count) = 0;

protected:
    const generator_context&  cntx_;
};

}} // namespace hexa::noise

