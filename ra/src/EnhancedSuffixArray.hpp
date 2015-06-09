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
*     3. Title: Linear-time longest-common-prefix computation in suffix arrays and its applications
*        Authors: Toru Kasai, Gunho Lee, Hiroki Arimura, Setsuo Arikawa, Kunsoo Park
*/

#pragma once

#include "CommonHeaders.hpp"

// Enhaced suffix array = suffix array + longest common prefix table + child table
// (+ other tables which are not needed here)

class EnhancedSuffixArray {
public:

    EnhancedSuffixArray(const std::string& str);
    ~EnhancedSuffixArray() {}

    int getLength() const {
        return n_;
    }

    const std::string& getString() const {
        return str_;
    }

    int getSuffix(int i) const {
        ASSERT(i >= 0 && i < n_, "ESA", "index out of range");
        return suftab_[i];
    }

    // O(1)
    void intervalSubInterval(int* s, int* e, int i, int j, char c) const;

    // O(1)
    int intervalLcpLen(int i, int j) const;

    size_t sizeInBytes() const;

    void serialize(char** bytes, size_t* bytesLen) const;
    static EnhancedSuffixArray* deserialize(const char* bytes);

    void print() const;

    friend class ReadIndex;

private:

    EnhancedSuffixArray() {};

    // Induced sorting O(n)
    void createSuffixArray(const unsigned char* s, int n, int csize, int alphabetSize = 256);

    // From suftab O(n)
    void createLongestCommonPrefixTable();

    // From lcptab O(n)
    void createChildTable();

    int n_;
    std::string str_;
    std::vector<int> suftab_;
    std::vector<int> lcptab_;
    std::vector<int> childtab_;
};
