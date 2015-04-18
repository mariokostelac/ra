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

    EnhancedSuffixArray(const std::string& data);
    EnhancedSuffixArray(const std::vector<std::string>& data);

    void print();

private:

    void createSuffixArray();

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