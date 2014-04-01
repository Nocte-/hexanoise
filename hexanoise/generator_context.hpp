//---------------------------------------------------------------------------
/// \file   hexanoise/generator_context.hpp
/// \brief  Holds all data that is shared between noise generators.
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/variant.hpp>

#include "node.hpp"

namespace hexa {
namespace noise {

class generator_context
{
public:
    struct image
    {
        size_t  width;
        size_t  height;
        uint8_t bitdepth;
        std::vector<uint8_t>    buffer;
    };

    typedef boost::variant<double, bool, std::string>  variable;

public:
    generator_context ();
    generator_context (const boost::property_tree::ptree& conf);

    void
    set_script (const std::string& name, const std::string& script);

    const node&
    get_script(const std::string& name) const;

    void
    set_global (const std::string& name, const variable& var);

    const variable&
    get_global (const std::string& name) const;

    bool
    exists_global (const std::string& name) const;


private:
    void init();

private:
    std::unordered_map<std::string, node>       scripts_;
    std::unordered_map<std::string, image>      images_;
    std::unordered_map<std::string, variable>   variables_;
};

}} // namespace hexa::noise

