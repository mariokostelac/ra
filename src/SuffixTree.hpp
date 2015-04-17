/*
* SuffixTree.hpp
*
* Created on: Apr 16, 2015
*     Author: rvaser
*/

#pragma once

#include "CommonHeaders.hpp"


class Node {
public:

    Node();
    Node(int edgeStart);
    Node(int edgeStart, int edgeEnd);

    int isRoot() {
        return root_;
    }

    int getEdgeStart() const {
        return edgeStart_;
    }

    void setEdgeStart(int start) {
        edgeStart_ = start;
    }

    int getEdgeEnd() const {
        return edgeEnd_;
    }

    void setEdgeEnd(int end) {
        edgeEnd_ = end;
    }

    int getSuffixLink() const {
        return suffixLink_;
    }

    void setSuffixLink(int suffixLink) {
        suffixLink_ = suffixLink;
    }

    const std::vector<int>& getChildren() const {
        return children_;
    }

    int getNextNode(int idx) {
        return children_[idx];
    }

    void setNextNode(int idx, int nextNode) {
        children_[idx] = nextNode;
    }

private:

    int root_;
    int edgeStart_;
    int edgeEnd_;
    int suffixLink_;
    std::vector<int> children_;
};


class SuffixTree {
public:

    SuffixTree(const std::string& str);
    ~SuffixTree();

    void toSuffixArray(std::vector<int>& suffixArray);

private:

    // Ukkonen algorithm O(n)
    void createSuffixTree(const std::string& str);

    void addSuffixLink(int suffixLinkEnd);
    bool adjustActivePoint(const std::string& str);

    int activeNode_;
    int activeChar_;
    int activeLength_;
    int remainder_;
    int suffixLinkStart_;
    std::vector<Node*> tree_;
};
