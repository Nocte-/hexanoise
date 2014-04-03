//---------------------------------------------------------------------------
/// \file   hexanoise/generator_opencl.hpp
/// \brief  Builds and executes an OpenCL kernel from a compiled HNDL script
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#pragma once

#include <string>
#include <sstream>
#include <list>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include "generator_i.hpp"

namespace hexa {
namespace noise {

class node;

/** Use OpenCL to execute a script. */
class generator_opencl : public generator_i
{
public:
    /** Set up a new generator
     * @param context  Shared data
     * @param opencl_context  The OpenCL context
     * @param opencl_device   The script will be executed on this device
     * @param n               The compiled script
     * @param opencl_file     A text file that defines the base functions
     *                        required by the OpenCL kernel.
     */
    generator_opencl (const generator_context& context,
                      cl::Context& opencl_context,
                      cl::Device&  opencl_device,
                      const node& n,
                      const std::string& opencl_file = "opencl.cl");

    /** Returns the generated OpenCL source code. */
    std::string opencl_sourcecode() const
    {
        return main_;
    }

    std::vector<double>
    run (const glm::dvec2& corner,
         const glm::dvec2& step,
         const glm::ivec2& count) override;

private:
    std::string pl (const node& n);
    std::string co (const node& n);

private:
    size_t count_;
    std::string main_;
    std::list<std::string> functions_;

    cl::Context      context_;
    cl::CommandQueue queue_;
    cl::Program      program_;
    cl::Kernel       kernel_;
};

}}

