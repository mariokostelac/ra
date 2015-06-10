/*
* Contig.hpp
*
* Created on: Jun 10, 2015
*     Author: rvaser
*/

#pragma once

#include "Read.hpp"
#include "Overlap.hpp"
#include "CommonHeaders.hpp"

class Contig {
public:

    Contig() {}
    ~Contig() {}

private:

    const std::vector<Read*>* reads_;
    const std::vector<Overlap*>* overlaps_;
};