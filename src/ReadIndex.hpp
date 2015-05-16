/*
* ReadIndex.hpp
*
* Created on: May 02, 2015
*     Author: rvaser
*/

#pragma once

#include "Read.hpp"
#include "EnhancedSuffixArray.hpp"
#include "CommonHeaders.hpp"

class ReadIndex {
public:

    ReadIndex(const Read* read, int rk = 0);
    ReadIndex(const std::vector<Read*>& reads, int rk = 0);
    ~ReadIndex();

    // O(m)
    size_t getNumberOfOccurrences(const char* pattern, int m) const;

    // O(m + z)
    void getPrefixSuffixMatches(std::vector<int>& dst, const char* pattern, int m,
        int minOverlapLen) const;

    void serialize(char** bytes, size_t* bytesLen) const;
    static ReadIndex* deserialize(char* bytes);

    size_t getSizeInBytes() const;

private:

    ReadIndex() {}

    int getIndex(int fragment, int position) const;

    int n_;
    std::vector<int> offsets_;
    std::vector<std::vector<int>> dictionary_;
    std::vector<EnhancedSuffixArray*> fragments_;
};

ReadIndex* createReadIndex(const std::vector<Read*>& reads, int rk, const char* path,
    const char* ext);
