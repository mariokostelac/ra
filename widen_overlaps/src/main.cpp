
#ifndef VERSION
#define VERSION "NO_VERSION"
#endif

#include "cmdline/cmdline.h"
#include "ra/ra.hpp"
#include <vector>

using std::fstream;
using std::string;
using std::vector;

// global vars
cmdline::parser args;
int thread_num;
string depot_path;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("depot", 'd', "depot path", true);

  args.parse_check(argc, argv);
}

void read_args() {
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  depot_path = args.get<string>("depot");
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  vector<Read*> reads;
  vector<Overlap*> overlaps;

  Depot depot(depot_path);

  depot.load_reads(reads);
  fprintf(stderr, "Read %lu reads\n", reads.size());

  depot.load_overlaps(overlaps, reads);
  fprintf(stderr, "Read %lu overlaps\n", overlaps.size());

  vector<Overlap*> dovetail_overlaps;
  for (auto o : overlaps) {
    dovetail_overlaps.push_back(forcedDovetailOverlap(o, true));
  }

  fprintf(stderr, "Updating depot...");
  depot.store_overlaps(dovetail_overlaps);

  for (auto r: reads)               delete r;
  for (auto o: overlaps)            delete o;
  for (auto do_: dovetail_overlaps) delete do_;

  return 0;
}
