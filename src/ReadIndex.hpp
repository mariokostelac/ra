/*
* ReadIndex.hpp
*
* Created on: May 02, 2015
*     Author: rvaser
*/

#pragma once

#include "IO.hpp"
#include "Read.hpp"
#include "EnhancedSuffixArray.hpp"
#include "CommonHeaders.hpp"

class ReadIndex {
public:

    ReadIndex(const std::vector<Read*>& reads, int rk = 0);
    ~ReadIndex();

    // O(m)
    size_t numberOfOccurrences(const char* pattern, int m) const;

    // O(m + z)
    void readDuplicates(std::vector<int>& dst, const Read* read) const;

    // O(m + z)
    void readPrefixSuffixMatches(std::vector<std::pair<int, int>>& dst, const Read* read,
        int rk, int minOverlapLen) const;

    size_t sizeInBytes() const;

    void serialize(char** bytes, size_t* bytesLen) const;
    static ReadIndex* deserialize(char* bytes);

    void store(const char* path) const;
    static ReadIndex* load(const char* path);

private:

    ReadIndex() {}

    void updateFragment(int fragment, int start, int end, const std::vector<Read*>& reads);

    int n_;
    std::vector<int> fragmentSizes_;
    std::vector<EnhancedSuffixArray*> fragments_;
};
