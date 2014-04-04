//---------------------------------------------------------------------------
// hexanoise/generator_opencl.cpp
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include "generator_opencl.hpp"

#include <iostream>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include "node.hpp"

#ifndef OPENCL_OCTAVES_LIMIT
# define OPENCL_OCTAVES_LIMIT 16
#endif

namespace hexa {
namespace noise {

generator_opencl::generator_opencl (const generator_context& ctx,
                                    cl::Context& opencl_context,
                                    cl::Device&  opencl_device,
                                    const node& n,
                                    const std::string& opencl_file)
    : generator_i(ctx)
    , count_(1)
    , context_(opencl_context)
    , queue_ (opencl_context, opencl_device)
{
    std::string body (co(n));

    std::ifstream file (opencl_file.c_str(), std::ios::binary);
    if (!file)
        throw std::runtime_error("cannot open OpenCL file " + opencl_file);

    file.seekg(0, std::ios::end);
    main_.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&main_[0], main_.size());
    file.close();

    main_ += "\n";
    for (auto& p : functions_)
    {
        main_ += p;
        main_ += "\n\n";
    }
    main_ += "\n" \
    "__kernel void noisemain(\n" \
    "  __global double* output, const double2 start, const double2 step, const int2 count)\n" \
    "{\n"\
    "    int i = get_global_id(0);\n" \
    "    double x = (double)(i % count.x);\n" \
    "    double y = (double)(i / count.x);\n" \
    "    double2 p = start + step * (double2)(x,y);\n" \
    "    output[i] = ";

    main_ += body;
    main_ += ";\n}\n";

    std::vector<cl::Device> device_vec;
    device_vec.emplace_back(opencl_device);

    cl::Program::Sources sources (1, { main_.c_str(), main_.size() });
    program_ = cl::Program(opencl_context, sources);
    try
    {
        program_.build(device_vec);
    }
    catch (cl::Error&)
    {
        std::cerr << program_.getBuildInfo<CL_PROGRAM_BUILD_LOG>(opencl_device) << std::endl;
        throw;
    }

    kernel_ = cl::Kernel(program_, "noisemain");
}

std::vector<double>
generator_opencl::run (const glm::dvec2& corner,
                       const glm::dvec2& step,
                       const glm::ivec2& count)
{
    std::vector<double> result (count.x * count.y);
    cl::Buffer output (context_, CL_MEM_WRITE_ONLY, result.size() * sizeof(double));

    kernel_.setArg(0, output);
    kernel_.setArg(1, sizeof(corner), (void*)&corner);
    kernel_.setArg(2, sizeof(step),   (void*)&step);
    kernel_.setArg(3, sizeof(count),  (void*)&count);

    queue_.enqueueNDRangeKernel(kernel_, cl::NullRange, result.size(), cl::NullRange);
    queue_.enqueueReadBuffer(output, CL_TRUE, 0, result.size() * sizeof(double), &result[0]);

    return result;
}

std::string generator_opencl::pl (const node& n)
{
    std::string result ("(");
    for (auto i (n.input.begin()); i != n.input.end(); )
    {
        result += co(*i);
        ++i;
        if (i != n.input.end())
            result.push_back(',');
    }
    result += ')';
    return result;
}

std::string generator_opencl::co (const node& n)
{
    switch (n.type)
    {
    case node::entry_point:     return "p";
    case node::const_var:       return std::to_string(n.aux_var);
    case node::const_bool:      return std::to_string(n.aux_bool);
    case node::const_str:       throw std::runtime_error("string encountered");

    case node::rotate:          return "p_rotate" + pl(n);
    case node::scale:           return "("+co(n.input[0])+"/"+co(n.input[1])+")";
    case node::shift:           return "("+co(n.input[0])+"+"+co(n.input[1])+")";
    case node::swap:            return "p_swap" + pl(n);

    case node::map:
    {
        std::string func_name ("p_map" + std::to_string(count_++));

        std::stringstream func_body;
        func_body
        << "inline double2 " << func_name << " (const double2 p) { return (double2)("
        << co(n.input[1]) << ", "
        << co(n.input[2]) << "); }" << std::endl;

        functions_.emplace_back(func_body.str());

        return func_name + "("+ co(n.input[0]) + ")";
    }

    case node::turbulence:
    {
        std::string func_name ("p_map" + std::to_string(count_++));

        std::stringstream func_body;
        func_body
        << "inline double2 " << func_name << " (const double2 p) { return (double2)("
        << "p.x+(" << co(n.input[1]) << "), "
        << "p.y+(" << co(n.input[2]) << ")); }" << std::endl;

        functions_.emplace_back(func_body.str());

        return func_name + "("+ co(n.input[0]) + ")";
    }

    case node::worley:
    {
        std::string func_name ("p_worley" + std::to_string(count_++));

        std::stringstream func_body;
        func_body
        << "inline double " << func_name << " (const double2 q, uint seed) { "
        << "  double2 p = p_worley(q, seed);"
        << "  return " << co(n.input[1]) << "; }" << std::endl;

        functions_.emplace_back(func_body.str());
        return func_name + "("+co(n.input[0])+ ","+co(n.input[2])+")";
    }

    case node::voronoi:
    {
        std::string func_name ("p_voronoi" + std::to_string(count_++));

        std::stringstream func_body;
        func_body
        << "inline double " << func_name << " (const double2 q, uint seed) { "
        << "  double2 p = p_voronoi(q, seed);"
        << "  return " << co(n.input[1]) << "; }" << std::endl;

        functions_.emplace_back(func_body.str());
        return func_name + "("+co(n.input[0])+ ","+co(n.input[2])+")";
    }

    case node::angle:           return "p_angle" + pl(n);
    case node::chebyshev:       return "p_chebyshev" + pl(n);
    case node::checkerboard:    return "p_checkerboard" + pl(n);
    case node::distance:        return "length" + pl(n);
    case node::manhattan:       return "p_manhattan" + pl(n);
    case node::perlin:          return "p_perlin" + pl(n);
    case node::x:               return co(n.input[0])+".x";
    case node::y:               return co(n.input[0])+".y";

    case node::add:             return "("+co(n.input[0])+"+"+co(n.input[1])+")";
    case node::sub:             return "("+co(n.input[0])+"-"+co(n.input[1])+")";
    case node::mul:             return "("+co(n.input[0])+"*"+co(n.input[1])+")";
    case node::div:             return "("+co(n.input[0])+"/"+co(n.input[1])+")";

    case node::abs:             return "fabs" + pl(n);
    case node::blend:           return "p_blend" + pl(n);
    case node::cos:             return "cospi" + pl(n);
    case node::min:             return "fmin" + pl(n);
    case node::max:             return "fmax" + pl(n);
    case node::neg:             return "-" + co(n.input[0]);

    case node::pow:
    {
        if (n.input[1].is_const)
        {
            double exp (std::floor(n.input[1].aux_var));
            if (std::abs(exp - n.input[1].aux_var) < 1e-9)
                return "pown("+co(n.input[0])+","+std::to_string((int)exp)+")";
        }
        return "pow"+pl(n);
    }

    case node::saw:             return "p_saw" + pl(n);
    case node::sin:             return "sinpi" + pl(n);
    case node::sqrt:            return "sqrt" + pl(n);
    case node::tan:             return "tanpi" + pl(n);

    case node::band:            return co(n.input[0])+"&&"+co(n.input[1]);
    case node::bor:             return co(n.input[0])+"||"+co(n.input[1]);
    case node::bxor:            return co(n.input[0])+"^^"+co(n.input[1]);
    case node::bnot:            return "!"+co(n.input[0]);

    case node::is_equal:        return co(n.input[0])+"=="+co(n.input[1]);
    case node::is_greaterthan:  return co(n.input[0])+">"+co(n.input[1]);
    case node::is_gte:          return co(n.input[0])+">="+co(n.input[1]);
    case node::is_lessthan:     return co(n.input[0])+"<"+co(n.input[1]);
    case node::is_lte:          return co(n.input[0])+"<="+co(n.input[1]);

    case node::is_in_circle:    return "p_is_in_circle" + pl(n);
    case node::is_in_rectangle: return "p_is_in_rectangle" + pl(n);

    case node::then_else:
        return "("+co(n.input[0])+")?("+co(n.input[1])+"):("+co(n.input[2])+")";

    case node::fractal:
    {
        assert(n.input.size() == 5);
        if (!n.input[2].is_const)
            throw std::runtime_error("fractal octave count must be a constexpr");

        int octaves (std::min<int>(n.input[2].aux_var, OPENCL_OCTAVES_LIMIT));

        std::string func_name ("p_fractal_" + std::to_string(count_++));

        std::stringstream func_body;
        func_body
        << "double " << func_name << " (double2 p, const double lac, const double per) {"
        << "double result = 0.0; double div = 0.0; double step = 1.0;"
        << "for(int i = 0; i < " << octaves << "; ++i)"
        << "{"
        << "  result += " << co(n.input[1]) << " * step;"
        << "  div += step;"
        << "  step *= lac;"
        << "  p *= per;"
        << "}"
        << "return result / div;"
        << "}";

        functions_.emplace_back(func_body.str());

        return func_name + "("
               + co(n.input[0]) + "," + co(n.input[3]) + ","
               + co(n.input[4]) + ")";
    }

    case node::lambda_:
    {
        assert(n.input.size() == 2);
        std::string func_name ("p_lambda_" + std::to_string(count_++));

        std::stringstream func_body;
        func_body
        << "double " << func_name << " (double2 p) {"
        << "return " << co(n.input[1]) << ";}" << std::endl;

        functions_.emplace_back(func_body.str());

        return func_name +"("+ co(n.input[0])+")";
    }

    case node::external_:
        throw std::runtime_error("OpenCL @external not implemented yet");

    case node::simplex:
        throw std::runtime_error("OpenCL simplex not implemented yet");

    case node::curve_linear:
        throw std::runtime_error("OpenCL curve_linear not implemented yet");

    case node::curve_spline:
        throw std::runtime_error("OpenCL curve_spline not implemented yet");

    case node::png_lookup:
        throw std::runtime_error("OpenCL png_lookup not implemented yet");

    default:
        throw std::runtime_error("function not implemented in OpenCL yet");
    }

    return std::string();
}

}} // namespace hexa::noise
