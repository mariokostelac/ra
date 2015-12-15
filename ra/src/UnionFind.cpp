#include "UnionFind.hpp"

// copied from https://github.com/mariokostelac/croler/blob/a52cd1153ab77834c576bc7d7f9bc852f7c67a17/pipeline/brahle_assembly/src/layout/union_find.cpp

UnionFind::UnionFind(int n) {
  data_ = new Node[n];
  for (int i = 0; i < n; ++i) {
    data_[i] = Node(i, i);
  }
}

UnionFind::~UnionFind() {
  delete [] data_;
}

int UnionFind::find(int x) {
  if (x == data_[x].parent_) {
    return x;
  }
  return data_[x].parent_ = find(data_[x].parent_);
}

int UnionFind::join(int x, int y) {
  x = find(x);
  y = find(y);
  if (x == y) {
    return -1;
  }
  if (data_[x].count_ < data_[y].count_) {
    data_[x].parent_ = y;
    data_[y].count_ += data_[x].count_;
    return y;
  } else {
    data_[y].parent_ = x;
    data_[x].count_ += data_[y].count_;
    return x;
  }
}

int UnionFind::size(int x) {
  return data_[find(x)].count_;
}
