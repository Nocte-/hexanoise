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

#include "global_variables_i.hpp"
#include "node.hpp"

namespace hexa
{
namespace noise
{

/** Generator shared data: bitmaps, global variables, and script source
* * code. */
class generator_context
{
public:
    typedef global_variables_i::var_type variable;

public:
    /** Bitmap data for lookup_png. */
    struct image
    {
        /** Width in pixels */
        size_t width;
        /** Height in pixels */
        size_t height;
        /** Bit depth (must be 1, 8, or 16) */
        uint8_t bitdepth;
        /** The actual bitmap data */
        std::vector<uint8_t> buffer;

        image() {}

        image(image&& m)
            : width(m.width)
            , height(m.height)
            , bitdepth(m.bitdepth)
            , buffer(std::move(m.buffer))
        {
        }

        image& operator=(image&& m)
        {
            if (&m != this) {
                width = m.width;
                height = m.height;
                bitdepth = m.bitdepth;
                buffer = std::move(m.buffer);
            }
            return *this;
        }
    };

public:
    /** Create a context without global variables. */
    generator_context();

    /** Create a context with global variables. */
    generator_context(const global_variables_i& global_vars);

    /** Add a HNDL script. */
    void set_script(const std::string& name, const std::string& script);

    /** Get a compiled version of a script by name.
     * @throw std::runtime_error if \a name was not found */
    const node& get_script(const std::string& name) const;

    /** Get a global variable by name.
     * @throw std::runtime_error if \a name was not found */
    variable get_global(const std::string& name) const;

    /** Check if a global variable exists. */
    bool exists_global(const std::string& name) const;

    /** Register image data. */
    void set_image(const std::string& name, image&& data);

    /** Load an image from a greyscale PNG file. */
    void load_png_image(const std::string& name, const std::string& png_file);

    /** Get an image by name. */
    const image& get_image(const std::string& name) const;

private:
    void init();

private:
    const global_variables_i& variables_;
    std::unordered_map<std::string, node> scripts_;
    std::unordered_map<std::string, image> images_;
};

} // namespace noise
} // namespace hexa
