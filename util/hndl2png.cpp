//---------------------------------------------------------------------------
/// \file   hexanoise/util/hndl2png.cpp
/// \brief  Tiny commandline utility that renders a PNG image of a
///         given HNDL script
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <png.h>
#include <boost/program_options.hpp>
#include <hexanoise/generator_context.hpp>
#include <hexanoise/generator_opencl.hpp>
#include <hexanoise/generator_slowinterpreter.hpp>

void write_png_file(const std::vector<uint8_t>& buf, int width, int height, const std::string& file_name)
{
    FILE *fp = fopen(file_name.c_str(), "wb");
    if (!fp)
         return;

    auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
         return;

    auto info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
         return;

    if (setjmp(png_jmpbuf(png_ptr)))
         return;

    png_init_io(png_ptr, fp);
    if (setjmp(png_jmpbuf(png_ptr)))
         return;

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
         return;

    auto row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (int y=0; y<height; y++)
            row_pointers[y] = (png_byte*)&buf[y*width];

    png_write_image(png_ptr, row_pointers);

    if (setjmp(png_jmpbuf(png_ptr)))
         return;

    png_write_end(png_ptr, NULL);

    free(row_pointers);
    fclose(fp);
}


namespace po = boost::program_options;
using namespace hexa::noise;

// Example use:
//
// $ echo 'scale(100):distance:sin' | hndl2png
//
int main (int argc, char** argv)
{
    po::variables_map vm;
    po::options_description options;
    options.add_options()
            ("version,v", "print version string")
            ("help", "show help message")

            ("output,o", po::value<std::string>()->default_value("out.png"),
             "output file")

            ("width,w", po::value<unsigned int>()->default_value(800),
            "output width")

            ("height,h", po::value<unsigned int>()->default_value(800),
            "output height")

            ("input,i", po::value<std::string>()->default_value("-"),
            "input file, use '-' for stdin")
            ;

    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }
    if (vm.count("version"))
    {
        std::cout << "hndl2png " << std::endl;
        return EXIT_SUCCESS;
    }

    std::string script;
    std::string file (vm["input"].as<std::string>());
    if (file == "-")
    {
        std::cin >> script;
    }
    else
    {
        return -1;
    }

    generator_context context;
    context.set_script("main", script);
    auto& n (context.get_script("main"));

    std::unique_ptr<generator_i> gen;

    std::vector<cl::Platform> platform_list;
    cl::Platform::get(&platform_list);
    if (platform_list.empty())
    {
        gen = std::unique_ptr<generator_i>(new generator_slowinterpreter(context, n));
    }
    else
    {
        auto& pl (platform_list[0]);
        std::vector<cl::Device> devices;
        pl.getDevices(CL_DEVICE_TYPE_ALL, &devices);

        cl_context_properties properties[] =
            { CL_CONTEXT_PLATFORM, (cl_context_properties)(pl)(), 0};
        cl::Context opencl_context (CL_DEVICE_TYPE_ALL, properties);

        gen = std::unique_ptr<generator_i>(new generator_opencl(context, opencl_context, devices[0], n));
    }

    auto width (vm["width"].as<unsigned int>());
    auto height (vm["height"].as<unsigned int>());

    glm::dvec2 center       (0, 0);
    glm::dvec2 step         (1, 1);
    glm::ivec2 pixel_size   (width, height);
    glm::dvec2 corner       (center - glm::dvec2(width, height) * step * 0.5);

    auto result (gen->run(corner, step, pixel_size));
    std::vector<uint8_t> pixmap (result.size());
    std::transform(result.begin(), result.end(), pixmap.begin(),
                   [](double i){ return static_cast<uint8_t>(127.0 + 127.0 * std::min(1.0, std::max(-1.0, i))); });

    write_png_file(pixmap, width, height, vm["output"].as<std::string>());

    return EXIT_SUCCESS;
}
