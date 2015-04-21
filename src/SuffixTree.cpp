/*
* SuffixTree.cpp
*
* Created on: Apr 16, 2015
*     Author: rvaser
*
* The code for the Ukkonnen algorithm has been modified from:
*     stackoverflow.com/questions/9452701/ukkonens-suffix-tree-algorithm-in-plain-english
*/

#include "SuffixTree.hpp"

#define ALPHABET_SIZE 28

// Allowed characters: English alphabet + delimeter # + sentinel ~
static const int CODER[] = {
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  26,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,   0,   1,   2,   3,   4,
      5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
     25,  -1,  -1,  -1,  -1,  -1,  -1,   0,   1,   2,
      3,   4,   5,   6,   7,   8,   9,  10,  11,  12,
     13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
     23,  24,  25,  -1,  -1,  -1,  27,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1
};

static int coder(char c) {
    return CODER[(unsigned char) c];
}

Node::Node(int edgeStart, int edgeEnd) :
    root_(edgeStart == -1), edgeStart_(edgeStart), edgeEnd_(edgeEnd), suffixLink_(-1), children_(ALPHABET_SIZE, -1) {
}

SuffixTree::SuffixTree(const std::string& str) :
    str_(str), activeNode_(0), activeChar_(-1), activeLength_(0), remainder_(0), suffixLinkStart_(-1) {

    tree_.push_back(new Node());
    createSuffixTree();
}

SuffixTree::~SuffixTree() {
    for (const auto& it : tree_) {
        delete it;
    }
}

void SuffixTree::toSuffixArray(std::vector<int>& suffixArray) {
    depthFirstSearch(0, 0, suffixArray);
}

void SuffixTree::print() {
    printTree(0, 0);
}

void SuffixTree::createSuffixTree() {

    for (int i = 0; i < (int) str_.size(); ++i) {
        ++remainder_;
        suffixLinkStart_ = -1;

        while (remainder_ > 0) {

            if (activeLength_ == 0) activeChar_ = i;

            if (tree_[activeNode_]->getNextNode(coder(str_[activeChar_])) == -1) {
                // New leaf node
                tree_.push_back(new Node(i));
                int leaf = tree_.size() - 1;

                tree_[activeNode_]->setNextNode(coder(str_[activeChar_]), leaf);

                // Rule 2
                addSuffixLink(activeNode_);

            } else {
                if (adjustActivePoint()) continue;

                int next = tree_[activeNode_]->getNextNode(coder(str_[activeChar_]));

                if (str_[tree_[next]->getEdgeStart() + activeLength_] == str_[i]) {
                    ++activeLength_;

                    // Rule 2
                    addSuffixLink(activeNode_);
                    break;
                }

                // Split edge by char c
                // - new internal node
                tree_.push_back(new Node(tree_[next]->getEdgeStart(), tree_[next]->getEdgeStart() + activeLength_));
                int internal = tree_.size() - 1;

                // - update activeNode (former parent of next)
                tree_[activeNode_]->setNextNode(coder(str_[activeChar_]), internal);

                // - update next
                tree_[next]->setEdgeStart(tree_[internal]->getEdgeEnd());

                // - new leaf node
                tree_.push_back(new Node(i));
                int leaf = tree_.size() - 1;

                // - connect internal with next and leaf
                tree_[internal]->setNextNode(coder(str_[tree_[next]->getEdgeStart()]), next);
                tree_[internal]->setNextNode(coder(str_[i]), leaf);

                // Rule 2
                addSuffixLink(internal);
            }

            --remainder_;

            if (tree_[activeNode_]->isRoot() && activeLength_ > 0) {
                // Rule 1
                --activeLength_;
                activeChar_ = i - remainder_ + 1;
            } else {
                // Rule 3
                activeNode_ = tree_[activeNode_]->getSuffixLink() == -1 ? 0 : tree_[activeNode_]->getSuffixLink();
            }
        }
    }
}

void SuffixTree::addSuffixLink(int suffixLinkEnd) {
    if (suffixLinkStart_ != -1) {
        tree_[suffixLinkStart_]->setSuffixLink(suffixLinkEnd);
    }
    suffixLinkStart_ = suffixLinkEnd;
}

bool SuffixTree::adjustActivePoint() {
    if (activeLength_ == 0) return false;

    int next = tree_[activeNode_]->getNextNode(coder(str_[activeChar_]));
    if (tree_[next]->getEdgeEnd() == -1) return false;

    int length = tree_[next]->getEdgeEnd() - tree_[next]->getEdgeStart();
    if (activeLength_ < length) return false;

    activeNode_ = next;
    activeLength_ -= length;
    activeChar_ += length;

    return true;
}

void SuffixTree::depthFirstSearch(int node, int edgeLen, std::vector<int>& suffixArray) {

    if (!tree_[node]->isRoot()) {
        // Leaf node
        if (tree_[node]->getEdgeEnd() == -1) {
            suffixArray.push_back(tree_[node]->getEdgeStart() - edgeLen);
            return;
        }

        edgeLen += tree_[node]->getEdgeEnd() - tree_[node]->getEdgeStart();
    }

    const std::vector<int>& children = tree_[node]->getChildren();
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        if (children[i] == -1) continue;

        depthFirstSearch(children[i], edgeLen, suffixArray);
    }
}

void SuffixTree::printTree(int i, int lvl) {
    if (tree_[i]->isRoot()) printf("*\n");

    ++lvl;

    const std::vector<int>& children = tree_[i]->getChildren();
    for (int j = 0; j < (int) children.size(); ++j) {
        if (children[j] == -1) continue;

        int len = (tree_[children[j]]->getEdgeEnd() == -1 ? str_.size() : tree_[children[j]]->getEdgeEnd()) - tree_[children[j]]->getEdgeStart();
        printf("%s%s\n", std::string(lvl, ' ').c_str(), str_.substr(tree_[children[j]]->getEdgeStart(), len).c_str());

        printTree(children[j], lvl);
    }
}