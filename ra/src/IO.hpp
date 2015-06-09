/*
* IO.hpp
*
* Created on: Apr 21, 2015
*     Author: rvaser
*/

#pragma once

#include "Read.hpp"
#include "Overlap.hpp"
#include "CommonHeaders.hpp"

class Options {
public:

    Options() {}
    Options(const char* readsPath, int threadLen, int k, int c, int minOverlapLen);
    ~Options() {}

    static Options* parseOptions(int argc, char** argv);

    static void help();

    const char* readsPath;
    int threadLen;
    int k;
    int c;
    int minOverlapLen;

private:

    static const struct option options_[];
};

void readFastaReads(std::vector<Read*>& reads, const char* path);

void readFastqReads(std::vector<Read*>& reads, const char* path);

void readAfgReads(std::vector<Read*>& reads, const char* path);

void writeAfgReads(const std::vector<Read*>& reads, const char* path);

void readAfgOverlaps(std::vector<Overlap*>& overlaps, const char* path);

void writeAfgOverlaps(const std::vector<Overlap*>& overlaps, const char* path);

bool fileExists(const char* path);

void fileRead(char** bytes, const char* path);

void fileWrite(const char* bytes, size_t bytesLen, const char* path);
