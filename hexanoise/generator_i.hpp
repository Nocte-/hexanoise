//---------------------------------------------------------------------------
/// \file   hexanoise/generator_i.hpp
/// \brief  Base class for noise generators
//
// Copyright 2014-2015, nocte@hippie.nu       Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include "generator_context.hpp"

namespace hexa
{
namespace noise
{

/** Base class for noise generators.
 *  The actual implementations run the HNDL script. */
class generator_i
{
public:
    generator_i(const generator_context& c)
        : cntx_(c)
    {
    }

    virtual ~generator_i() {}

    /** Run the script for a given range, output in double precision.
     * @param corner    The top-left corner of the range
     * @param step      The step size between samples
     * @param count     The number of samples to take in the x and y direction
     * @return A buffer with size (count.x * count.y) holding the results
     */
    virtual std::vector<double> run(const glm::dvec2& corner,
                                    const glm::dvec2& step,
                                    const glm::ivec2& count) = 0;

    /** Run the script for a given range, output in signed 16-bit precision.
     * @param corner    The top-left corner of the range
     * @param step      The step size between samples
     * @param count     The number of samples to take in the x and y direction
     * @return A buffer with size (count.x * count.y) holding the results
     */
    virtual std::vector<int16_t> run_int16(const glm::dvec2& corner,
                                           const glm::dvec2& step,
                                           const glm::ivec2& count) = 0;

    /** Run the script for a given range, output in double precision.
     * @param corner    The corner of the range
     * @param step      The step size between samples
     * @param count     The number of samples to take in the x, y, and z
     * direction
     * @return A buffer with size (count.x * count.y * count.z) holding the
     * results
     */
    virtual std::vector<double> run(const glm::dvec3& corner,
                                    const glm::dvec3& step,
                                    const glm::ivec3& count) = 0;

    /** Run the script for a given range, output in signed 16-bit precision.
     * @param corner    The corner of the range
     * @param step      The step size between samples
     * @param count     The number of samples to take in the x, y, and z
     * direction
     * @return A buffer with size (count.x * count.y * count.z) holding the
     * results
     */
    virtual std::vector<int16_t> run_int16(const glm::dvec3& corner,
                                           const glm::dvec3& step,
                                           const glm::ivec3& count) = 0;

protected:
    const generator_context& cntx_;
};
}
} // namespace hexa::noise
