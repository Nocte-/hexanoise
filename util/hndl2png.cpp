//---------------------------------------------------------------------------
/// \file   hexanoise/util/hndl2png.cpp
/// \brief  Tiny commandline utility that renders a PNG image of a
///         given HNDL script
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <fstream>
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
    FILE *fp = 0;
    if (file_name.empty())
        fp = stdout;
    else
        fp = fopen(file_name.c_str(), "wb");

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

void opencl_info()
{
    std::vector<cl::Platform> platform_list;
    cl::Platform::get(&platform_list);
    if (platform_list.empty())
    {
        std::cout << "No OpenCL platforms available." << std::endl;
        return;
    }

    int i (0);
    for (auto& p : platform_list)
    {
        std::cout << "Platform " << i++ << ":" << std::endl
                  << "  name    : " << p.getInfo<CL_PLATFORM_NAME>() << std::endl
                  << "  profile : " << p.getInfo<CL_PLATFORM_PROFILE>() << std::endl
                  << "  ext     : " << p.getInfo<CL_PLATFORM_EXTENSIONS>() << std::endl
                  << std::endl;

        int j (0);

        std::vector<cl::Device> devices;
        p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        for (auto& dv : devices)
        {
            std::cout << "  Device " << j++ << " :" << std::endl
                      << "    name    : " << dv.getInfo<CL_DEVICE_NAME>() << std::endl
                      << "    version : " << dv.getInfo<CL_DEVICE_VERSION>() << std::endl
                      << "    driver  : " << dv.getInfo<CL_DRIVER_VERSION>() << std::endl
                      << "    units   : " << dv.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl
                      << "    groups  : " << dv.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl
                      << std::endl;
        }
    }
}


namespace po = boost::program_options;
using namespace hexa::noise;

// Example use:
//
// $ echo 'scale(100):distance:sin' | hndl2png -o out.png
//
int main (int argc, char** argv)
{
    try
    {
    po::variables_map vm;
    po::options_description options;
    options.add_options()
            ("version,v", "print version string")
            ("help", "show help message")
            ("info", "display OpenCL info and exit")
            ("dumpsrc", "print the generated OpenCL source and exit")

            ("output,o", po::value<std::string>()->default_value(""),
             "output file (default to stdout)")

            ("width,w", po::value<unsigned int>()->default_value(500),
            "output width")

            ("height,h", po::value<unsigned int>()->default_value(500),
            "output height")

            ("input,i", po::value<std::string>()->default_value("-"),
            "input file, use '-' for stdin")

            ("platform", po::value<unsigned int>()->default_value(0),
             "choose an OpenCL platform (see also: --info)")

            ("device", po::value<unsigned int>()->default_value(0),
             "choose an OpenCL device (see also: --info)")

            ("force-slow",
             "disable OpenCL and use the interpreter")

            ("time", po::value<unsigned int>()->default_value(1),
             "run the script n times before writing the png file")

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
    if (vm.count("info"))
    {
        try
        {
            opencl_info();
        }
        catch (std::exception& e)
        {
            std::cout << "Exception caught: " << e.what() << std::endl
                      << "hndl2png will use the interpreter to run scripts."
                      << std::endl;
        }
        catch (...)
        {
            std::cout << "Unknown exception caught." << std::endl
                      << "hndl2png will use the interpreter to run scripts."
                      << std::endl;
        }
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
        std::ifstream s (file);
        if (!s){
            std::cerr << "Cannot open file " << file << std::endl;
            return EXIT_FAILURE;
        }
        while (!s.eof())
            s >> script;
    }

    generator_context context;
    context.set_script("main", script);
    auto& n (context.get_script("main"));

    std::unique_ptr<generator_i> gen;

    try
    {
        if (vm.count("force-slow"))
            throw 0; // Fake an error so the interpreter is used

        std::vector<cl::Platform> platform_list;
        cl::Platform::get(&platform_list);
        if (platform_list.empty())
        {
            gen = std::unique_ptr<generator_i>(new generator_slowinterpreter(context, n));
        }
        else
        {
            auto platform_index (vm["platform"].as<unsigned int>());
            if (platform_index >= platform_list.size())
            {
                std::cerr << "'" << platform_index << "' is not in the list of platforms. (See: --info)" << std::endl;
                return EXIT_FAILURE;
            }

            auto& pl(platform_list[platform_index]);
            std::vector<cl::Device> devices;
            pl.getDevices(CL_DEVICE_TYPE_ALL, &devices);

            auto device_index (vm["device"].as<unsigned int>());
            if (device_index >= devices.size())
            {
                std::cerr << "'" << device_index << "' is not in the list of devices. (See: --info)" << std::endl;
                return EXIT_FAILURE;
            }

            cl_context_properties properties [] =
            { CL_CONTEXT_PLATFORM, (cl_context_properties) (pl) (), 0 };
            cl::Context opencl_context(CL_DEVICE_TYPE_ALL, properties);

            auto tmp (new generator_opencl(context, opencl_context, devices[device_index], n));
            if (vm.count("dumpsrc"))
                std::cout << tmp->opencl_sourcecode() << std::endl;

            gen = std::unique_ptr<generator_i>(tmp);
        }
    }
    catch (...)
    {
        gen = std::unique_ptr<generator_i>(new generator_slowinterpreter(context, n));
    }

    if (vm.count("dumpsrc"))
        return EXIT_SUCCESS;

    auto width (vm["width"].as<unsigned int>());
    auto height (vm["height"].as<unsigned int>());

    glm::dvec2 center       (0, 0);
    glm::dvec2 step         (1, 1);
    glm::ivec2 pixel_size   (width, height);
    glm::dvec2 corner       (center - glm::dvec2(width, height) * step * 0.5);

    auto repeat (vm["time"].as<unsigned int>());
    if (repeat > 1)
    {
        for (unsigned int i (1); i < repeat; ++i)
            gen->run(corner, step, pixel_size);
    }
    auto result (gen->run(corner, step, pixel_size));
    std::vector<uint8_t> pixmap (result.size());
    std::transform(result.begin(), result.end(), pixmap.begin(),
                   [](double i){ return static_cast<uint8_t>(127.0 + 127.0 * std::min(1.0, std::max(-1.0, i))); });

    write_png_file(pixmap, width, height, vm["output"].as<std::string>());
    }
    catch (cl::Error& e)
    {
        std::cerr << "Error in " << e.what() << ", code " << e.err() << std::endl;
        return EXIT_FAILURE;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
