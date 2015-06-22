/*!
 * @file EditDistance.hpp
 *
 * @brief Edlib wrapper header file
 * @details Wrapper for edlib by @Martinsos (Github: https://github.com/Martinsos/edlib).
 *
 * @author rvaser (robert.vaser@gmail.com)
 * @date Jun 07, 2015
 */

#pragma once

#include "CommonHeaders.hpp"

/*!
 * @brief Edit distance wrapper
 * @details Method returns the edit distance between two strings.
 *
 * @param [in] query string
 * @param [in] target string
 * @return edit distance
 */
extern int32_t editDistance(const std::string& query, const std::string& target);
