//---------------------------------------------------------------------------
// hexanoise/unit_tests/unit_test.cpp
//
// Copyright 2014-2015, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------

#define BOOST_TEST_MODULE hexanoise_unittests test
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>
#include <hexanoise/generator_context.hpp>
#include <hexanoise/generator_opencl.hpp>
#include <hexanoise/generator_slowinterpreter.hpp>
#include <hexanoise/simple_global_variables.hpp>

#ifdef WIN32
#  define OPENCL_DLL_NAME "OpenCL.dll"
#elif defined(MACOSX)
#  define OPENCL_DLL_NAME 0
#else
#  define OPENCL_DLL_NAME "libOpenCL.so"
#endif

using namespace hexa::noise;
using namespace boost::algorithm;

BOOST_AUTO_TEST_CASE(test_full)
{
    BOOST_REQUIRE(clewInit(OPENCL_DLL_NAME) >= 0);

    std::vector<cl::Platform> platform_list;
    cl::Platform::get(&platform_list);
    BOOST_REQUIRE(platform_list.size() != 0);

    auto& pl = platform_list[0];
    std::vector<cl::Device> devices;
    pl.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM,
                                           (cl_context_properties)(pl)(),
                                           0 };
    cl::Context opencl_context{CL_DEVICE_TYPE_ALL, properties};

    
    std::ifstream str {"tests"};
    BOOST_REQUIRE(str);
    std::string line, input, output;
    
    while (std::getline(str, line))
    {
        trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        
        // Compile script
        simple_global_variables gv;        
        gv["one"] = 1.0;
        gv["two"] = 2.0;
                        
        generator_context ctx{gv};        
        ctx.set_script("test", line);
        auto& test = ctx.get_script("test");
        generator_slowinterpreter gl_gen{ctx, test};
        generator_opencl cl_gen{ctx, opencl_context, devices[0], test};
        
        for (;;) {
            std::getline(str, input);
            trim(input);
            if (input.empty())
                break;
            
            std::getline(str, output);
            trim(output);
            double expected = std::stod(output);
            
            typedef boost::char_separator<char> sep_t;
            sep_t sep{","};
            std::vector<double> v;
            for (auto& value : boost::tokenizer<sep_t>{input, sep}) {
                v.emplace_back(std::stod(value));
            }
            
            double result1, result2;
            if (v.size() == 2) {
                result1 = gl_gen.run(glm::dvec2{v[0], v[1]}, 
                                     glm::dvec2{1.0, 1.0},
                                     glm::ivec2{1, 1})[0];
                
                result2 = cl_gen.run(glm::dvec2{v[0], v[1]}, 
                                    glm::dvec2{1.0, 1.0},
                                    glm::ivec2{1, 1})[0];                
            } else if (v.size() == 3) {
                result1 = gl_gen.run(glm::dvec3{v[0], v[1], v[2]}, 
                                    glm::dvec3{1.0, 1.0, 1.0},
                                    glm::ivec3{1, 1, 1})[0];           
                
                result2 = cl_gen.run(glm::dvec3{v[0], v[1], v[2]}, 
                                    glm::dvec3{1.0, 1.0, 1.0},
                                    glm::ivec3{1, 1, 1})[0];                                
            } else {
                throw std::runtime_error(input + " is not a valid position");
            }
            
            bool succ1 = std::abs(result1 - expected) < 0.0001;
            bool succ2 = std::abs(result2 - expected) < 0.0001;
            if (!succ1 || !succ2) {
                std::cerr << "Failed function: " << line 
                          << "\nExpected: " << expected << "\nResult: "
                          << result1 << " / " << result2 << std::endl;
            }

            BOOST_CHECK(succ1 && succ2);
        }
    }
}
