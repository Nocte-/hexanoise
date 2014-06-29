//---------------------------------------------------------------------------
// hexanoise/unit_tests/unit_test.cpp
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#define BOOST_TEST_MODULE hexanoise_unittests test
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>
#include <vector>

#include <hexanoise/generator_context.hpp>
#include <hexanoise/generator_opencl.hpp>
#include <hexanoise/generator_slowinterpreter.hpp>
#include <hexanoise/simple_global_variables.hpp>

#ifdef WIN32
#define OPENCL_DLL_NAME "OpenCL.dll"
#elif defined(MACOSX)
#define OPENCL_DLL_NAME 0
#else
#define OPENCL_DLL_NAME "libOpenCL.so"
#endif

using namespace hexa::noise;

inline void check_err(cl_int err, const std::string& name)
{
    if (err != CL_SUCCESS) {
        std::cerr << "Error: " << name << std::endl;
        exit(EXIT_FAILURE);
    }
}

BOOST_AUTO_TEST_CASE(opencl_basic)
{
    BOOST_REQUIRE(clewInit(OPENCL_DLL_NAME) >= 0);

    try
    {
        std::vector<cl::Platform> platform_list;
        cl::Platform::get(&platform_list);
        BOOST_CHECK(platform_list.size() != 0);

        for (auto& pl : platform_list) {
            std::cout << "Platform: " << pl.getInfo<CL_PLATFORM_NAME>()
                      << std::endl;

            std::vector<cl::Device> devices;
            pl.getDevices(CL_DEVICE_TYPE_ALL, &devices);
            for (auto& dv : devices) {
                std::cout << "  Device: " << dv.getInfo<CL_DEVICE_NAME>()
                          << std::endl;
            }
        }

        auto& pl(platform_list[0]);
        std::vector<cl::Device> devices;
        pl.getDevices(CL_DEVICE_TYPE_ALL, &devices);

        cl_context_properties properties[] = { CL_CONTEXT_PLATFORM,
                                               (cl_context_properties)(pl)(),
                                               0 };
        cl::Context opencl_context(CL_DEVICE_TYPE_ALL, properties);

        simple_global_variables gv;
        generator_context ctx(gv);
        gv["devil"] = 666.0;
        ctx.set_script("lol", "distance:sin");

        auto& sc = ctx.get_script("lol");
        BOOST_CHECK_EQUAL(sc.type, node::sin);
        BOOST_CHECK_EQUAL(sc.input.size(), 1);
        
        auto& sc2 = sc.input[0];
        BOOST_CHECK_EQUAL(sc2.type, node::distance);
        BOOST_CHECK_EQUAL(sc2.input.size(), 1);
        
        auto& sc3 = sc2.input[0];
        BOOST_CHECK_EQUAL(sc3.type, node::entry_point);
        BOOST_CHECK_EQUAL(sc3.input.size(), 0);
                
        //generator_opencl cl_gen(ctx, opencl_context, devices[0],
        //                        sc);
        //std::cout << cl_gen.opencl_sourcecode() << std::endl;

        generator_slowinterpreter sl_gen(ctx, sc);

        auto result(sl_gen.run(glm::dvec2(5.5, 5.5), glm::dvec2(1.0, 1.0),
                               glm::ivec2(50, 50)));
        std::cout << result[0] << std::endl;

        //auto result2(cl_gen.run(glm::dvec2(5.5, 5.5), glm::dvec2(1.0, 1.0),
        //                        glm::ivec2(50, 50)));
        //std::cout << result[0] << std::endl;
    }
    catch (cl::Error & e)
    {
        std::cerr << e.what() << ": " << e.err() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(test_3d)
{
    generator_context ctx;
    ctx.set_script("lol", "distance3");
    auto& sc = ctx.get_script("lol");
    generator_slowinterpreter sl_gen(ctx, sc);
    
    auto result(sl_gen.run(glm::dvec3(8.,8.,8.), glm::dvec3(1.0, 1.0, 1.0),
                           glm::ivec3(1,1,1)));
    std::cout << result[0] << std::endl;
}
