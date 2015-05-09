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

    Node(int edgeStart = -1, int edgeEnd = -1);
    ~Node() {}

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

    int getNextNode(int idx) const {
        return children_[idx];
    }

    void setNextNode(int idx, int nextNode) {
        children_[idx] = nextNode;
    }

private:

    int edgeStart_;
    int edgeEnd_;
    int suffixLink_;
    std::vector<int> children_;
};

class SuffixTree {
public:

    SuffixTree(const std::string& str);
    ~SuffixTree();

    void toSuffixArray(std::vector<int>& suffixArray) const;

    void print() const;

private:

    // Ukkonen algorithm O(n)
    void createSuffixTree();

    // O(|E|)
    void depthFirstSearch(int node, int edgeLen, std::vector<int>& suffixArray) const;

    void printTree(int i, int lvl) const;

    std::string str_;
    std::vector<Node*> tree_;
};
