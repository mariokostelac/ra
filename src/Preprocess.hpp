/*
* Preprocess.hpp
*
* Created on: Apr 27, 2015
*     Author: rvaser
*/

#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

// error correction of reads based on k-mer frequencies
// - path is used to cache the ReadIndex
void errorCorrection(std::vector<Read*>& reads, int k, int c, const char* path = NULL);
