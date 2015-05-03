/*
* EnhancedSuffixArray.hpp
*
* Created on: Apr 18, 2015
*     Author: rvaser
*/

#pragma once

#include "CommonHeaders.hpp"

// Enhaced suffix array = suffix array + longest common prefix table + child table
// (+ other tables which are not needed here)

class EnhancedSuffixArray {
public:

    EnhancedSuffixArray(const std::string& str);
    EnhancedSuffixArray(const std::vector<const std::string*>& vstr);

    // O(m)
    int getNumberOfOccurrences(const char* pattern, int m) const;

    // O(m + z)
    void getOverlaps(std::vector<std::vector<int>>& overlaps, const char* pattern, int m) const;

    void serialize(char** bytes, size_t* bytesLen) const;
    static EnhancedSuffixArray* deserialize(const char* bytes);

    void print() const;

    size_t getSizeInBytes() const;

private:

    EnhancedSuffixArray() {};

    // From suffix tree O(n)
    void createSuffixArrayST();

    // Induced sorting O(n)
    void createSuffixArray(const unsigned char* s, int n, int csize, int alphabetSize = 256);

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
