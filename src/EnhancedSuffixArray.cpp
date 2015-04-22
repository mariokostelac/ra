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

static bool equalSubstr(const std::string& str1, int s1, int e1, const std::string& str2, int s2, int e2) {

    if (e1 - s1 != e2 - s2) return false;

    for (int i = 0; i <= e1 - s1; ++i) {
        if (str1[s1 + i] != str2[s2 + i]) return false;
    }

    return true;
}

EnhancedSuffixArray::EnhancedSuffixArray(const Read* read) {

    str_ += read->getSequence();
    str_ += SENTINEL;

    createSuffixArray();
    createLongestCommonPrefixTable();
    createChildTable();
}

EnhancedSuffixArray::EnhancedSuffixArray(const std::vector<Read*>& reads) {

    for (const auto& it : reads) {
        str_ += it->getSequence();
        str_ += DELIMETER;
    }

    str_ += SENTINEL;

    createSuffixArray();
    createLongestCommonPrefixTable();
    createChildTable();   
}

void EnhancedSuffixArray::createSuffixArray() {

    n_ = str_.size();

    SuffixTree* st = new SuffixTree(str_);
    // st->print();
    st->toSuffixArray(suftab_);
    delete st;
}

int EnhancedSuffixArray::getNumberOfOccurrences(const std::string& pattern) {

    if (pattern.empty()) return 0;

    int i, j;
    getInterval(&i, &j, pattern);

    if (i == -1 && j == -1) return 0;
    return j - i + 1;
}

void EnhancedSuffixArray::getOccurrences(std::vector<int>& positions, const std::string& pattern) {

    if (pattern.empty()) return;

    int i, j;
    getInterval(&i, &j, pattern);

    if (i == -1 && j == -1) return;

    for (int k = i; k < j + 1; ++k) {
        positions.push_back(suftab_[k]);
    }
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
                // .down
                childtab_[st.top()] = lastIndex;
            }
        }

        if (lastIndex != -1) {
            // .up
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
            // .nextlIndex
            childtab_[st.top()] = i;
            st.pop();
        }

        st.push(i);
    }
}

void EnhancedSuffixArray::getInterval(int* s, int* e, const std::string& pattern) {

    int m = pattern.size();
    int i, j, c = 0;
    bool found = false;

    getInterval(&i, &j, 0, n_ - 1, pattern[c]);

    while (i != -1 && j != -1 && c < m) {
        found = true;

        if (i != j) {
            int l = getLcp(i, j);
            int min = l < m ? l : m;

            found = equalSubstr(str_, suftab_[i] + c, suftab_[i] + min - 1,
                pattern, c, min - 1);

            c = min;
            if (c == m) break;

            getInterval(&i, &j, i, j, pattern[c]);

        } else {
            found = equalSubstr(str_, suftab_[i] + c, suftab_[i] + m - 1,
                pattern, c, m - 1);
            c = m;
        }

        if (!found) break;
    }

    if (found) {
        *s = i;
        *e = j;
        return;
    }

    *s = -1;
    *e = -1;
}

void EnhancedSuffixArray::getInterval(int* s, int* e, int i, int j, char c) {

    int i1 = (i < childtab_[i] && childtab_[i] <= j) ? childtab_[i] : childtab_[j];

    if (str_[suftab_[i] + lcptab_[i1]] == c) {
        *s = i; *e = i1 - 1;
        return;
    }

    while (childtab_[i1] != -1 && childtab_[i1] != i1 && lcptab_[childtab_[i1]] == lcptab_[i1]) {
        int i2 = childtab_[i1];
        if (str_[suftab_[i1] + lcptab_[i2]] == c) {
            *s = i1; *e = i2 - 1;
            return;
        }
        i1 = i2;
    }

    if (str_[suftab_[i1] + lcptab_[i1]] == c) {
        *s = i1; *e = j;
        return;
    }

    *s = -1;
    *e = -1;
}

int EnhancedSuffixArray::getLcp(int i, int j) {
    return lcptab_[childtab_[(i < childtab_[i] && childtab_[i] <= j) ? i : j]];
}
