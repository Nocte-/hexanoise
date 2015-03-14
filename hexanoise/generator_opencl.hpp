//---------------------------------------------------------------------------
/// \file   hexanoise/generator_opencl.hpp
/// \brief  Builds and executes an OpenCL kernel from a compiled HNDL script
//
// Copyright 2014-2015, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <string>
#include <sstream>
#include <list>

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "generator_i.hpp"

namespace hexa
{
namespace noise
{

class node;

/** Use OpenCL to execute a HNDL script. */
class generator_opencl : public generator_i
{
public:
    /** Set up a new generator
     * @param context  Shared data
     * @param opencl_context  The OpenCL context
     * @param opencl_device   The script will be executed on this device
     * @param n               The compiled script
     */
    generator_opencl(const generator_context& context,
                     cl::Context& opencl_context, cl::Device& opencl_device,
                     const node& n);

    /** Returns the generated OpenCL source code. */
    std::string opencl_sourcecode() const { return main_; }

    std::vector<double> run(const glm::dvec2& corner, const glm::dvec2& step,
                            const glm::ivec2& count) override;

    std::vector<int16_t> run_int16(const glm::dvec2& corner,
                                   const glm::dvec2& step,
                                   const glm::ivec2& count) override;

    std::vector<double> run(const glm::dvec3& corner, const glm::dvec3& step,
                            const glm::ivec3& count) override;

    std::vector<int16_t> run_int16(const glm::dvec3& corner,
                                   const glm::dvec3& step,
                                   const glm::ivec3& count) override;

private:
    std::string pl(const node& n);
    std::string co(const node& n);

private:
    size_t count_;
    std::string main_;
    std::list<std::string> functions_;

    cl::Context context_;
    cl::Device device_;
    cl::CommandQueue queue_;
    cl::Program program_;
    cl::Kernel kernel_;
    cl::Kernel kernel_int16_;
    cl::Kernel kernel3_;
};

}
}
