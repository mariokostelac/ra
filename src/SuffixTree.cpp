/*
* SuffixTree.cpp
*
* Created on: Apr 16, 2015
*     Author: rvaser
*/

#include "SuffixTree.hpp"

#define ALPHABET_SIZE 28

// Allowed characters: English alphabet + delimeter # + sentinel ~
static const int CODER[] = {
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  -1,   1,   2,   3,   4,   5,
     6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
    26,  -1,  -1,  -1,  -1,  -1,  -1,   1,   2,   3,
     4,   5,   6,   7,   8,   9,  10,  11,  12,  13,
    14,  15,  16,  17,  18,  19,  20,  21,  22,  23,
    24,  25,  26,  -1,  -1,  -1,  27,  -1,  -1,  -1,
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

static void addSuffixLink(int* suffixLinkStart, int suffixLinkEnd, const std::vector<Node*>& tree) {

    if (*suffixLinkStart != -1) {
        tree[*suffixLinkStart]->setSuffixLink(suffixLinkEnd);
    }

    *suffixLinkStart = suffixLinkEnd;
}

// needed when edge length is lower or equal to activeLength
static bool adjustActivePoint(int* activeNode, int* activeChar, int* activeLength,
    const std::vector<Node*>& tree, const std::string& str) {

    if (*activeLength == 0) return false;

    int next = tree[*activeNode]->getNextNode(coder(str[*activeChar]));
    if (tree[next]->getEdgeEnd() == -1) return false;

    int length = tree[next]->getEdgeEnd() - tree[next]->getEdgeStart();
    if (*activeLength < length) return false;

    *activeNode = next;
    *activeLength -= length;
    *activeChar += length;

    return true;
}

Node::Node(int edgeStart, int edgeEnd) :
    edgeStart_(edgeStart), edgeEnd_(edgeEnd), suffixLink_(-1), children_(ALPHABET_SIZE, -1) {
}

SuffixTree::SuffixTree(const std::string& str) :
    str_(str), tree_() {

    tree_.push_back(new Node());
    createSuffixTree();
}

SuffixTree::~SuffixTree() {
    for (const auto& it : tree_) {
        delete it;
    }
}

void SuffixTree::toSuffixArray(std::vector<int>& suffixArray) const {
    depthFirstSearch(0, 0, suffixArray);
}

void SuffixTree::print() const {
    printTree(0, 0);
}

void SuffixTree::createSuffixTree() {

    int activeNode = 0;
    int activeChar = -1;
    int activeLength = 0;
    int remainder = 0;
    int suffixLinkStart = -1;

    for (int i = 0; i < (int) str_.size(); ++i) {
        ++remainder;
        suffixLinkStart = -1;

        while (remainder > 0) {

            if (activeLength == 0) activeChar = i;

            if (tree_[activeNode]->getNextNode(coder(str_[activeChar])) == -1) {
                // New leaf node
                tree_.push_back(new Node(i));
                int leaf = tree_.size() - 1;

                tree_[activeNode]->setNextNode(coder(str_[activeChar]), leaf);

                // Rule 2
                addSuffixLink(&suffixLinkStart, activeNode, tree_);

            } else {
                if (adjustActivePoint(&activeNode, &activeChar, &activeLength, tree_, str_)) continue;

                int next = tree_[activeNode]->getNextNode(coder(str_[activeChar]));

                if (str_[tree_[next]->getEdgeStart() + activeLength] == str_[i]) {
                    ++activeLength;

                    // Rule 2
                    addSuffixLink(&suffixLinkStart, activeNode, tree_);
                    break;
                }

                // Split edge by char c
                // - new internal node
                tree_.push_back(new Node(tree_[next]->getEdgeStart(), tree_[next]->getEdgeStart() + activeLength));
                int internal = tree_.size() - 1;

                // - update activeNode (former parent of next)
                tree_[activeNode]->setNextNode(coder(str_[activeChar]), internal);

                // - update next
                tree_[next]->setEdgeStart(tree_[internal]->getEdgeEnd());

                // - new leaf node
                tree_.push_back(new Node(i));
                int leaf = tree_.size() - 1;

                // - connect internal with next and leaf
                tree_[internal]->setNextNode(coder(str_[tree_[next]->getEdgeStart()]), next);
                tree_[internal]->setNextNode(coder(str_[i]), leaf);

                // Rule 2
                addSuffixLink(&suffixLinkStart, internal, tree_);
            }

            --remainder;

            if (activeNode == 0 && activeLength > 0) {
                // Rule 1
                --activeLength;
                activeChar = i - remainder + 1;
            } else {
                // Rule 3
                activeNode = tree_[activeNode]->getSuffixLink() == -1 ? 0 : tree_[activeNode]->getSuffixLink();
            }
        }
    }
}

void SuffixTree::depthFirstSearch(int node, int edgeLen, std::vector<int>& suffixArray) const {

    if (node != 0) {
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

void SuffixTree::printTree(int i, int lvl) const {

    if (i == 0) printf("*\n");

    ++lvl;

    const std::vector<int>& children = tree_[i]->getChildren();
    for (int j = 0; j < (int) children.size(); ++j) {
        if (children[j] == -1) continue;

        int len = (tree_[children[j]]->getEdgeEnd() == -1 ? str_.size() :
            tree_[children[j]]->getEdgeEnd()) - tree_[children[j]]->getEdgeStart();

        printf("%s%s\n", std::string(lvl, ' ').c_str(),
            str_.substr(tree_[children[j]]->getEdgeStart(), len).c_str());

        printTree(children[j], lvl);
    }
}