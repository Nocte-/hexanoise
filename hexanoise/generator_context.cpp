//---------------------------------------------------------------------------
// hexanoise/generator_context.cpp
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------

#include "generator_context.hpp"

#include <stdexcept>
#include <boost/property_tree/ptree.hpp>
#include "ast.hpp"
#include "parser.hpp"
#include "tokens.hpp"

extern int yyparse(hexa::noise::function **func, yyscan_t scanner);

namespace hexa {
namespace noise {

generator_context::generator_context ()
{
    init();
}

generator_context::generator_context (const boost::property_tree::ptree& conf)
{
    init();
}

void
generator_context::init()
{
    if (variables_.count("seed") == 0)
        variables_.emplace("seed", 0.0);
}

void
generator_context::set_script (const std::string& name,
                               const std::string& script)
{
    function* func;
    yyscan_t scanner;
    YY_BUFFER_STATE  state;

    if (yylex_init(&scanner))
        throw std::runtime_error("yylex_init failed");

    state = yy_scan_string(script.c_str(), scanner);
    if (yyparse(&func, scanner))
        throw std::runtime_error("yyparse failed");

    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    scripts_.emplace(std::make_pair(name, node(func, *this)));
    delete func;
}

const node&
generator_context::get_script (const std::string& name) const
{
    auto found (scripts_.find(name));
    if (found == scripts_.end())
        throw std::runtime_error("script " + name + " not found");

    return found->second;
}

void
generator_context::set_global (const std::string& name, const variable& var)
{
    variables_[name] = var;
}

const generator_context::variable&
generator_context::get_global (const std::string& name) const
{
    auto found (variables_.find(name));
    if (found == variables_.end())
        throw std::runtime_error("variable '" + name + "'' not defined");

    return found->second;
}

bool
generator_context::exists_global (const std::string& name) const
{
    return variables_.count(name) > 0;
}

const generator_context::image&
generator_context::get_image (const std::string& name) const
{
    auto found (images_.find(name));
    if (found == images_.end())
        throw std::runtime_error("image '" + name + "'' not found");

    return found->second;
}

}} // namespace hexa::noise

