//---------------------------------------------------------------------------
/// \file   hexanoise/global_variables_i.hpp
/// \brief  Interface to the application's global variables
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <stdexcept>
#include <string>
#include <boost/variant.hpp>

namespace hexa
{
namespace noise
{

/** This exception is thrown by global_variables_i::get() */
class variable_not_found : public std::runtime_error
{
public:
    variable_not_found(const std::string& name)
        : std::runtime_error("the global variable '" + name +
                             "' does not exist"),
          name_(name)
    {
    }

    std::string name() const
    {
        return name_;
    }

private:
    std::string name_;
};

/** Interface for accessing gloabal variables. */
class global_variables_i
{
public:
    typedef boost::variant<bool, double, std::string> var_type;

public:
    virtual ~global_variables_i()
    {
    }

    /** Check if a variable exists. */
    virtual bool exists(const std::string& name) const = 0;

    /** Get the value of a global variable.
     * @throw variable_not_found if \a name is not a known variable */
    virtual var_type get(const std::string& name) const = 0;
};
}
} // namespace hexa::noise
