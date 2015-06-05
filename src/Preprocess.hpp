/*
* Preprocess.hpp
*
* Created on: Apr 27, 2015
*     Author: rvaser
*
* Error correction was rewritten and modified from SGA (String Graph Assembler)
*     Paper: Efficient de novo assembly of large genomes using compressed data structures (+ supplemmental material)
*     Authors: Jared T. Simpson and Richard Durbin
*     Github: https://github.com/jts/sga
*/

#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

class KmerDistribution {
public:

    KmerDistribution() {}
    ~KmerDistribution() {}

    void add(int kmerCount) {
        histogram_[kmerCount]++;
    }

    // returns the proportion of the distribution less than or equal to n
    double cumulativeProportion(int n) const;

    // returns the boundary of the left tail which holds erroneous kmers
    int errorBoundary() const;
    int errorBoundary(double ratio) const;

    // returns the mode of the distribution if the first n values are ignored
    int ignoreMode(size_t n) const;

    void toCountVector(std::vector<int>& dst, int max) const;

private:

    std::map<int, int> histogram_;
};

// error correction of reads based on k-mer frequencies
// - path is used to cache the ReadIndex
void correctReads(std::vector<Read*>& reads, int k, int c, int threadLen, const char* path);

void filterReads(std::vector<Read*>& dst, std::vector<Read*>& reads, bool view = true);
