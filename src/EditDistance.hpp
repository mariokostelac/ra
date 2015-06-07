/*
* EditDistance.hpp
*
* Created on: Jun 07, 2015
*     Author: rvaser
*
* Wrapper function for edlib
*/

#pragma once

#include "edlib/edlib.h"
#include "CommonHeaders.hpp"

extern int32_t editDistance(const std::string& query, const std::string& target);
