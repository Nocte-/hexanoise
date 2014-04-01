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

/** Generator shared data: bitmaps, global variables, and script source
 ** code. */
class generator_context
{
public:
    /** Bitmap data for lookup_png. */
    struct image
    {
        /** Width in pixels */
        size_t  width;
        /** Height in pixels */
        size_t  height;
        /** Bit depth (must be 1, 8, or 16) */
        uint8_t bitdepth;
        /** The actual bitmap data */
        std::vector<uint8_t>    buffer;
    };

    /** Global variables can be either doubles, bools, or strings. */
    typedef boost::variant<double, bool, std::string>  variable;

public:
    generator_context ();

    /** Initialize the context with a Hexahedra world setup file.
     *  This is usually a JSON file, but anything that fits in a Boost
     *  property tree works.  The generator_context only loads the
     *  global settings and the area generator scripts.  For the
     *  full documentation, please refer to the Hexahedra docs. */
    generator_context (const boost::property_tree::ptree& conf);

    /** Add a HNDL script. */
    void
    set_script (const std::string& name, const std::string& script);

    /** Get a compiled version of a script by name.
     * @throw std::runtime_error if \a name was not found */
    const node&
    get_script(const std::string& name) const;

    /** Set a global variable to a given value. */
    void
    set_global (const std::string& name, const variable& var);

    /** Get a global variable by name.
     * @throw std::runtime_error if \a name was not found */
    const variable&
    get_global (const std::string& name) const;

    /** Check if a global variable exists. */
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

