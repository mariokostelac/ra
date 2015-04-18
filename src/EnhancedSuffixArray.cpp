/*
* EnhancedSuffixArray.cpp
*
* Created on: Apr 18, 2015
*     Author: rvaser
*
* Algorithms were rewritten to c++ from the following paper:
*     Title: Replacing suffix trees with enhanced suffix arrays
*     Authors: Mohamed Ibrahim Abouelhoda, Stefan Kurtz, Enno Ohlebusch
*/

#include "SuffixTree.hpp"
#include "EnhancedSuffixArray.hpp"

#define DELIMETER '#'
#define SENTINEL '~'

static int lcp(int s1, int s2, const std::string& str) {
    int n = str.size();
    int lcp = 0;

    for (int i = s1, j = s2; i < n && j < n; ++i, ++j) {
        if (str[i] != str[j]) break;
        ++lcp;
    }

    return lcp;
}

EnhancedSuffixArray::EnhancedSuffixArray(const std::string& data) {
    str_ += data + SENTINEL;

    createSuffixArray();
    createLongestCommonPrefixTable();
    createChildTable();
}

EnhancedSuffixArray::EnhancedSuffixArray(const std::vector<std::string>& data) {
    for (const auto& it : data) str_ += it + DELIMETER;
    str_ += SENTINEL;

    createSuffixArray();
    createLongestCommonPrefixTable();
    createChildTable();   
}

void EnhancedSuffixArray::createSuffixArray() {
    n_ = str_.size();

    SuffixTree* st = new SuffixTree(str_);
    st->toSuffixArray(suftab_);
    delete st;
}

void EnhancedSuffixArray::print() {
    printf("  Idx Suftab LcpTab ChildTab Suffix\n");

    for (int i = 0; i < n_; ++i) {
        printf("%5d %6d %6d %8d %-s\n", i, suftab_[i], lcptab_[i], childtab_[i],
            str_.substr(suftab_[i], n_ - suftab_[i]).c_str());
    }
}

void EnhancedSuffixArray::createLongestCommonPrefixTable() {
    lcptab_.resize(n_, 0);

    for (int i = 1; i < n_; ++i) {
        lcptab_[i] = lcp(suftab_[i], suftab_[i - 1], str_);
    }
}

void EnhancedSuffixArray::createChildTable() {
    childtab_.resize(n_, -1);

    // childTable = .up + .down + .nextlIndex (which can be stored in 4B)
    // 1. Construction of .up and .down
    std::stack<int> st;
    int lastIndex = -1;

    st.push(0);
    for (int i = 1; i < n_; ++i) {
        while (lcptab_[i] < lcptab_[st.top()]) {
            lastIndex = st.top();
            st.pop();
            if ((lcptab_[i] <= lcptab_[st.top()]) && (lcptab_[st.top()] != lcptab_[lastIndex])) {
                childtab_[st.top()] = lastIndex;
            }
        }

        if (lastIndex != -1) {
            childtab_[i - 1] = lastIndex;
            lastIndex = -1;
        }

        st.push(i);
    }

    // 2. Construciton of .nextlIndex
    std::stack<int>().swap(st);

    st.push(0);
    for (int i = 1; i < n_; ++i) {
        while (lcptab_[i] < lcptab_[st.top()]) st.pop();

        if (lcptab_[i] == lcptab_[st.top()]) {
            childtab_[st.top()] = i;
            st.pop();
        }

        st.push(i);
    }
}