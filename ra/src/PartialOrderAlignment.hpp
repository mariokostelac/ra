/*!
 * @file PartialOrderAlignment.hpp
 *
 * @brief Cpppoa wrapper header file
 * @details Wrapper for cpppoa by @mculinovic (Github: https://github.com/mculinovic/cpppoa).
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Jun 11, 2015
 */

#pragma once

#include "Read.hpp"
#include "StringGraph.hpp"
#include "CommonHeaders.hpp"

/*!
 * @brief Partial order alignment consensus wrapper
 * @details Method returns consensus string with the help of partial order alignment.
 *
 * @param [in] contig Contig object
 * @param [in] reads vector of Read object pointers
 * @return consensus sequence
 */
extern std::string consensus(const Contig* contig, const std::vector<Read*>& reads);
