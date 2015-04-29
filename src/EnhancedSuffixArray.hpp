/*
* EnhancedSuffixArray.hpp
*
* Created on: Apr 18, 2015
*     Author: rvaser
*/

#pragma once

#include "Read.hpp"
#include "CommonHeaders.hpp"

// Enhaced suffix array = suffix array + longest common prefix table + child table
// (+ other tables which are not needed here)

class EnhancedSuffixArray {
public:

    // InducedSorting = 1, SuffixTree = 0
    EnhancedSuffixArray(const Read* read, int rk = 0, int algorithm = 1);
    EnhancedSuffixArray(const std::vector<Read*>& reads, int rk = 0, int algorithm = 1);

    // O(m)
    int getNumberOfOccurrences(const char* pattern, int m) const;

    // O(m + z)
    void getOverlaps(std::vector<std::vector<int>>& overlaps, const char* pattern, int m) const;

    void serialize(char** bytes, int* bytesLen) const;
    static EnhancedSuffixArray* deserialize(const char* bytes);

    void print() const;

private:

    EnhancedSuffixArray() {};

    // From suffix tree O(n)
    void createSuffixArrayST();

    // Induced sorting O(n)
    void createSuffixArrayIS(const unsigned char* s, int n, int csize, int alphabetSize = 256);

    // From suftab O(n)
    void createLongestCommonPrefixTable();

    // From lcptab O(n)
    void createChildTable();

    void getInterval(int* s, int* e, const char* pattern, int m) const;
    void getSubInterval(int* s, int* e, int i, int j, char c) const;
    int getLcp(int i, int j) const;

    int n_;
    std::string str_;
    std::vector<int> suftab_;
    std::vector<int> lcptab_;
    std::vector<int> childtab_;
};
