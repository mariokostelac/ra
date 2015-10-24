
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
int thread_num = std::max(std::thread::hardware_concurrency(), 1U);
string depot_path;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("depot", 'd', "depot path", true);

  args.parse_check(argc, argv);
}

void read_args() {
  depot_path = args.get<string>("depot");
}

void fill_reads_coverage(ReadSet& reads, vector<Overlap*>& overlaps) {
  for (uint32_t i = 0, size = overlaps.size(); i < size; ++i) {
    Overlap* overlap = overlaps[i];
    uint32_t a = overlap->a();
    uint32_t b = overlap->b();
    reads[a]->add_coverage(overlap->covered_percentage(a));
    reads[b]->add_coverage(overlap->covered_percentage(b));
  }
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  vector<Read*> reads;
  vector<Overlap*> overlaps;

  Depot depot(depot_path);

  depot.load_reads(reads);
  fprintf(stderr, "%lu reads loaded from depot\n", reads.size());

  depot.load_overlaps(overlaps, reads);
  fprintf(stderr, "%lu overlaps loaded from depot\n", overlaps.size());

  fprintf(stderr, "Calculating read coverage...\n");
  fill_reads_coverage(reads, overlaps);

  fprintf(stderr, "Updating reads in depot...\n");
  depot.store_reads(reads);

  for (auto r: reads)       delete r;
  for (auto o: overlaps)    delete o;

  return 0;
}
