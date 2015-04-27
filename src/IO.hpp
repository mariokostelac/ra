/*
* IO.hpp
*
* Created on: Apr 21, 2015
*     Author: rvaser
*/

#pragma once

#include "CommonHeaders.hpp"
#include "Read.hpp"


void readFastaReads(std::vector<Read*>& reads, const char* path);

void readFastqReads(std::vector<Read*>& reads, const char* path);

void readAfgReads(std::vector<Read*>& reads, const char* path);

void readFromFile(char** bytes, int* bytesLen, const char* path);

void writeToFile(const char* bytes, int bytesLen, const char* path);
