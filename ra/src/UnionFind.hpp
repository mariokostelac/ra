#pragma once

// copied from https://github.com/mariokostelac/croler/blob/a52cd1153ab77834c576bc7d7f9bc852f7c67a17/pipeline/brahle_assembly/src/layout/union_find.hpp
class UnionFind {
  private:
    struct Node {
      int value_;
      int count_;
      int parent_;
      Node() : Node(0, 1, -1) {}
      Node(int value, int parent) : Node(value, 1, parent) {}
      Node(int value, int count, int parent) :
        value_(value), count_(count), parent_(parent) {}
    };

    Node *data_;

  public:
    UnionFind(int n);
    ~UnionFind();
    int find(int x);
    int join(int x, int y);
    int size(int x);
};
