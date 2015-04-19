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
    EnhancedSuffixArray(const std::vector<std::string>& strs);

    // O(m)
    int getNumberOfOccurrences(const std::string& pattern);

    // O(m + z)
    void getOccurrences(std::vector<int>& positions, const std::string& pattern);

    void print();

private:

    // From suffix tree O(n)
    void createSuffixArray();

    // From suftab O(n)
    void createLongestCommonPrefixTable();

    // From lcptab O(n)
    void createChildTable();

    void getInterval(int* s, int* e, const std::string& pattern);
    void getInterval(int* s, int* e, int i, int j, char c);
    int getLcp(int i, int j);

    int n_;
    std::string str_;
    std::vector<int> suftab_;
    std::vector<int> lcptab_;
    std::vector<int> childtab_;
};
