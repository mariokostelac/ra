
#ifndef VERSION
#define VERSION "NO_VERSION"
#endif

#include "cmdline/cmdline.h"
#include "ra/ra.hpp"
#include <fstream>
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
  fprintf(stderr, "%lu reads read\n", reads.size());

  depot.load_overlaps(overlaps, reads);
  fprintf(stderr, "%lu overlaps reads\n", overlaps.size());

  vector<Overlap*> notransitives;
  filterTransitiveOverlaps(notransitives, overlaps, thread_num, true);

  depot.store_overlaps(notransitives);

  for (auto r: reads)       delete r;
  for (auto o: overlaps)    delete o;

  return 0;
}
