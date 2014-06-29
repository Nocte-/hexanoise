//---------------------------------------------------------------------------
/// \file   hexanoise/simple_global_variables.hpp
/// \brief  Very basic implementation of global variables
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <unordered_map>
#include <string>
#include "global_variables_i.hpp"

namespace hexa
{
namespace noise
{

/** Very basic class to hold global variables.
 * @code

 simple_global_variables gv;
 generator_context context (gv);

 gv["foo"] = 6.2;
 gv["bar"] = "Baz";

 context.set_script("test", "@foo:mul(2)");

 * @endcode
 */
class simple_global_variables
    : public global_variables_i,
      public std::unordered_map<std::string, global_variables_i::var_type>
{
public:
    simple_global_variables() { emplace("seed", 0.0); }

    /** Check if a variable exists. */
    bool exists(const std::string& name) const override
    {
        return count(name) > 0;
    }

    /** Get the value of a global variable.
     * @throw variable_not_found if \a name is not a known variable */
    var_type get(const std::string& name) const override
    {
        auto found(find(name));
        if (found == end())
            throw variable_not_found(name);

        return found->second;
    }
};

} // namespace noise
} // namespace hexa
