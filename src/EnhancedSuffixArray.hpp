/*
* EnhancedSuffixArray.hpp
*
* Created on: Apr 18, 2015
*     Author: rvaser
*
* Algorithms were rewritten to c++ from following papers:
*     1. Title: Two efficient algorithms for linear time suffix array construction
*        Authors: Ge Nong, Sen Zhang, Wai Hong Chan
*     2. Title: Replacing suffix trees with enhanced suffix arrays
*        Authors: Mohamed Ibrahim Abouelhoda, Stefan Kurtz, Enno Ohlebusch
*/

#pragma once

#include "CommonHeaders.hpp"

#define DELIMITER '#'
#define SENTINEL_H '~'
#define SENTINEL_L '!'

// Enhaced suffix array = suffix array + longest common prefix table + child table
// (+ other tables which are not needed here)

class EnhancedSuffixArray {
public:

    EnhancedSuffixArray(const std::string& str);
    EnhancedSuffixArray(const std::vector<const std::string*>& vstr);
    ~EnhancedSuffixArray() {}

    int getLength() const {
        return n_;
    }

    int getSuffix(int i) const {
        ASSERT(i >= 0 && i < n_, "ESA", "index out of range");
        return suftab_[i];
    }

    // O(m)
    void getInterval(int* s, int* e, const char* pattern, int m) const;

    // O(1)
    void getSubInterval(int* s, int* e, int i, int j, char c) const;

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

    int getLcp(int i, int j) const;

    int n_;
    std::string str_;
    std::vector<int> suftab_;
    std::vector<int> lcptab_;
    std::vector<int> childtab_;
};
