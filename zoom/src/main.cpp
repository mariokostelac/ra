
#include "cmdline/cmdline.h"
#include "ra/ra.hpp"
#include <algorithm>
#include <ctime>
#include <iostream>
#include <list>
#include <map>
#include <sys/stat.h>
#include <vector>

using std::cout;
using std::endl;
using std::list;
using std::map;
using std::string;
using std::vector;

map<int, list<Overlap*>> edges;

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
  args.add<string>("depot", 'd', "depot path", true);

  args.parse_check(argc, argv);

  const int root = args.get<int>("root");
  const int depth = args.get<int>("depth");
  const string depot_path = args.get<string>("depot");

  Depot depot(depot_path);

  vector<Read*> reads;
  depot.load_reads(reads);
  fprintf(stderr, "%lu reads loaded\n", reads.size());

  vector<Overlap*> overlaps;
  depot.load_overlaps(overlaps, reads);
  fprintf(stderr, "%lu overlaps loaded\n", overlaps.size());

  for (auto o: overlaps) {
    edges[o->a()].push_back(o);
    edges[o->b()].push_back(o);
  }

  vector<Overlap*> neighborhood;
  map<Overlap*, bool> used;
  dfs(&neighborhood, &used, root, depth);

  fprintf(stderr, "%lu overlaps written\n", neighborhood.size());
  for (auto e: neighborhood) {
    std::cout << *e;
  }

  for (auto o: overlaps) {
    delete o;
  }

  return 0;
}
