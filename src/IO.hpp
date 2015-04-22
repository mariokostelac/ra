/*
* IO.hpp
*
* Created on: Apr 21, 2015
*     Author: rvaser
*/

#pragma once

#include "CommonHeaders.hpp"
#include "Read.hpp"

class IO {
public:

    IO() {}

    void readFastaReads(std::vector<Read*>& reads, const char* path);
    void readFastqReads(std::vector<Read*>& reads, const char* path);
    void readAfgReads(std::vector<Read*>& reads, const char* path);

private:

};