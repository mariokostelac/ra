/*
* PartialOrderdAlignment.hpp
*
* Created on: Jun 11, 2015
*     Author: rvaser
*
* Wrapper function for cpppoa by @mculinovic
*     Github: https://github.com/mculinovic/cpppoa
*/

#pragma once

#include "Read.hpp"
#include "StringGraph.hpp"
#include "CommonHeaders.hpp"

extern std::string consensus(const Contig* contig, const std::vector<Read*>& reads);
