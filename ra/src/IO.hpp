/*
* IO.hpp
*
* Created on: Apr 21, 2015
*     Author: rvaser
*/

#pragma once

#include "Read.hpp"
#include "Overlap.hpp"
#include "StringGraph.hpp"
#include "CommonHeaders.hpp"

void readFastaReads(std::vector<Read*>& reads, const char* path);

void readFastqReads(std::vector<Read*>& reads, const char* path);

void readAfgReads(std::vector<Read*>& reads, const char* path);

void writeAfgReads(const std::vector<Read*>& reads, const char* path);

void readAfgOverlaps(std::vector<Overlap*>& overlaps, const char* path);

void writeAfgOverlaps(const std::vector<Overlap*>& overlaps, const char* path);

void readAfgContigs(std::vector<Contig*>& contigs, const char* path);

void writeAfgContigs(const std::vector<Contig*>& contigs, const char* path);

bool fileExists(const char* path);

void fileRead(char** bytes, const char* path);

void fileWrite(const char* bytes, size_t bytesLen, const char* path);
