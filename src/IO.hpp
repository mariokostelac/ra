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

    static void readFastaReads(std::vector<Read*>& reads, const char* path);
    static void readFastqReads(std::vector<Read*>& reads, const char* path);
    static void readAfgReads(std::vector<Read*>& reads, const char* path);

    static void readFromFile(char** bytes, int* bytesLen, const char* path);
    static void writeToFile(const char* bytes, int bytesLen, const char* path);

private:

};