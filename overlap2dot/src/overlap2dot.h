
#ifndef _OVERLAP2DOT_H
#define _OVERLAP2DOT_H

#include <vector>
#include <string>
#include <iostream>
#include "ra/EdgesSet.hpp"

using std::endl;
using std::ostream;
using std::string;
using std::vector;
using Graph::EdgesSet;

const string get_edge_style(const bool use_start, const bool use_end);

template <typename V>
int dot_graph(ostream& output, vector<V>& overlaps) {
  int lines = 0;

  output << "graph overlaps {\n";
  lines++;

  for (const auto& overlap : overlaps) {
    uint32_t a_id = overlap->a();
    uint32_t b_id = overlap->b();
    output << a_id << " -- " << b_id << " [";

    string tail_style = get_edge_style(overlap->isUsingPrefix(a_id), overlap->isUsingSuffix(a_id));
    string head_style = get_edge_style(overlap->isUsingPrefix(b_id), overlap->isUsingSuffix(b_id));

    output << "dir=both arrowtail=" << tail_style << " arrowhead=" << head_style << ", label=\"" << overlap->quality() << "\"];" << endl;
    lines++;
  }

  output << "}\n";
  lines++;

  return lines;
}

#endif
