//---------------------------------------------------------------------------
// hexanoise/generator_context.cpp
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include "generator_context.hpp"

#include <stdexcept>
#include <cstdio>
#include <boost/property_tree/ptree.hpp>

#include "ast.hpp"
#include "parser.hpp"
#include "tokens.hpp"

extern int yyparse(hexa::noise::function **func, yyscan_t scanner);

namespace hexa {
namespace noise {

namespace {

#if HAVE_PNG
#include <png.h>

generator_context::image
png_load(const std::string& file_name)
{
    png_byte header[8];
    FILE *fp = fopen(file_name.c_str(), "rb");
    if (fp == 0)
        throw std::runtime_error("cannot open file " + file_name);

    auto read (fread(header, 1, 8, fp));
    if (read != 8 || png_sig_cmp(header, 0, 8))
    {
        fclose(fp);
        throw std::runtime_error(file_name + " is not a PNG file");
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(fp);
        throw std::runtime_error("png_create_read_struct failed");
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        throw std::runtime_error("png_create_info_struct failed");
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        throw std::runtime_error("png_create_info_struct failed");
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        throw std::runtime_error("setjmp failed");
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth,
                 &color_type, 0, 0, 0);

    png_read_update_info(png_ptr, info_ptr);
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    generator_context::image result;

    result.width = temp_width;
    result.height = temp_height;
    result.bitdepth = bit_depth;
    result.buffer.resize(rowbytes * temp_height);

    png_byte* image_data (&result.buffer[0]);

    png_bytep* row_pointers = (png_bytep*)malloc(temp_height * sizeof(png_bytep));
    if (row_pointers == NULL)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        throw std::runtime_error("out of memory");
    }

    for (unsigned int i (0); i < temp_height; ++i)
        row_pointers[i] = image_data + i * rowbytes;

    png_read_image(png_ptr, row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(row_pointers);
    fclose(fp);

    return result;
}

#else

generator_context::image
png_load(const std::string& file_name)
{
    throw std::runtime_error("no PNG support");
}

#endif

class global_variables_null : public global_variables_i
{
public:
    bool exists (const std::string& name) const override
    {
        return name == "seed";
    }

    var_type get (const std::string& name) const override
    {
        if (name == "seed")
            return 0.0;

        throw variable_not_found(name);
    }
};

static global_variables_null global_null;

} // anonymous namespace

//---------------------------------------------------------------------------

generator_context::generator_context ()
    : variables_(global_null)
{
}

generator_context::generator_context (const global_variables_i& v)
    : variables_(v)
{
}

void
generator_context::set_script (const std::string& name,
                               const std::string& script)
{
    function* func;
    yyscan_t scanner;
    YY_BUFFER_STATE  state;

    if (yylex_init(&scanner))
        throw std::runtime_error("yylex_init failed");

    state = yy_scan_string(script.c_str(), scanner);
    if (yyparse(&func, scanner))
        throw std::runtime_error("yyparse failed");

    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    scripts_.emplace(std::make_pair(name, node(func, *this)));
    delete func;
}

const node&
generator_context::get_script (const std::string& name) const
{
    auto found (scripts_.find(name));
    if (found == scripts_.end())
        throw std::runtime_error("script '" + name + "'' not found");

    return found->second;
}

generator_context::variable
generator_context::get_global (const std::string& name) const
{
    return variables_.get(name);
}

bool
generator_context::exists_global (const std::string& name) const
{
    return variables_.exists(name);
}

void
generator_context::set_image(const std::string& name, image&& data)
{
    images_[name] = std::move(data);
}

void
generator_context::load_png_image(const std::string& name, const std::string& png_file)
{
    images_[name] = png_load(png_file);
}

const generator_context::image&
generator_context::get_image (const std::string& name) const
{
    auto found (images_.find(name));
    if (found == images_.end())
        throw std::runtime_error("image " + name + " not found");

    return found->second;
}

}} // namespace hexa::noise

