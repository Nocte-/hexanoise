//---------------------------------------------------------------------------
/// \file   hexanoise/analysis.hpp
/// \brief  Analyse scripts
//
// Copyright 2014, nocte@hippie.nu            Released under the MIT License.
//---------------------------------------------------------------------------
#pragma once

#include <string>
#include <unordered_set>

namespace hexa
{
namespace noise
{

class node;

/** Make an estimate of the execution length of the function.
 * All functions are weighted with a factor 1, all noise functions have
 * weight 5.
 * @param n  The function to analyse
 * @return   Estimated execution length (sum of the weight of all nodes) */
size_t weight(const node& n);

/** Get a list of all images used by a function.
 * @param n  The function to analyse
 * @return  A list of all image files referenced by png_lookup */
std::unordered_set<std::string> referred_images(const node& n);

/** Get a list of all external scripts used by a function.
 * @param n  The function to analyse
 * @return  A list of all scripts referenced by the @-operator */
std::unordered_set<std::string> referred_scripts(const node& n);

} // namespace noise
} // namespace hexa
