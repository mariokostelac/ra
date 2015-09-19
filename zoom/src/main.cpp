
#include "cmdline/cmdline.h"
#include "ra/ra.hpp"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sys/stat.h>
#include <vector>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::fstream;
using std::map;
using std::max;
using std::pair;
using std::make_pair;
using std::string;
using std::vector;

map<int, std::list<Overlap*>> edges;

void dfs(vector<Overlap*>* neighborhood, map<Overlap*, bool>* used, const int node, const int depth) {
  if (depth <= 0) {
    return;
  }

  for (auto e: edges[node]) {
    auto next = e->a() == node ? e->b() : e->a();
    if (used->count(e)) {
      continue;
    }
    (*used)[e] = true;

    neighborhood->push_back(e);
    dfs(neighborhood, used, next, depth - 1);
  }
}

int main(int argc, char **argv) {

  cmdline::parser args;

  // input params
  args.add<int>("root", 'r', "root read", true);
  args.add<int>("depth", 'n', "neighborhood depth", true);
  args.add<string>("overlaps", 'x', "overlaps file", true);
  args.add<string>("overlaps_format", 'f', "overlaps file format; supported: afg, mhap", false, "afg");

  args.parse_check(argc, argv);

  const string overlaps_filename = args.get<string>("overlaps");
  const string overlaps_format = args.get<string>("overlaps_format");
  const int root = args.get<int>("root");
  const int depth = args.get<int>("depth");

  vector<Overlap*> overlaps;

  if (overlaps_format == "afg") {
    vector<DovetailOverlap*> overlaps_;
    readAfgOverlaps(overlaps_, overlaps_filename.c_str());
    for (auto o : overlaps_) {
      overlaps.push_back(o);
    }
  } else if (overlaps_format == "mhap") {
    fstream overlaps_file(overlaps_filename);
    MHAP::read_overlaps(overlaps_file, &overlaps);
    overlaps_file.close();
  } else {
    assert(false);
  }

  cerr << overlaps.size() << " overlaps read" << endl;

  for (auto o: overlaps) {
    edges[o->a()].push_back(o);
    edges[o->b()].push_back(o);
  }

  vector<Overlap*> neighborhood;
  map<Overlap*, bool> used;
  dfs(&neighborhood, &used, root, depth);


  cerr << neighborhood.size() << " overlaps written" << endl;
  for (auto e: neighborhood) {
    std::cout << *e;
  }

  for (auto o: overlaps) {
    delete o;
  }

  return 0;
}
