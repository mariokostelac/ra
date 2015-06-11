/*
* EditDistance.hpp
*
* Created on: Jun 07, 2015
*     Author: rvaser
*
* Wrapper function for edlib by @Martinsos
*     Github: https://github.com/Martinsos/edlib
*/

#pragma once

#include "CommonHeaders.hpp"

#include "../vendor/edlib/edlib.h"

extern int32_t editDistance(const std::string& query, const std::string& target);
