
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
string overlaps_filename;
string assembly_directory;
string depot_path;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("overlaps", 'x', "overlaps file", true);
  args.add<string>("directory", 'd', "assembly_directory", false, ".");
  args.add<string>("depot", 'D', "depot path", true);

  args.parse_check(argc, argv);
}

void read_args() {
  assembly_directory = args.get<string>("directory");
  overlaps_filename = args.get<string>("overlaps");
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
  Depot depot(depot_path);
  depot.load_reads(reads);

  fprintf(stderr, "%lu reads loaded from depot\n", reads.size());

  fprintf(stderr, "Reading %s...\n", overlaps_filename.c_str());

  vector<Overlap*> overlaps;

  fstream overlaps_file(overlaps_filename);
  MHAP::read_overlaps(overlaps, reads, overlaps_file);
  overlaps_file.close();

  fprintf(stderr, "%lu overlaps read\n", overlaps.size());

  fprintf(stderr, "Updating reads in store...\n");
  fill_reads_coverage(reads, overlaps);

  depot.store_reads(reads);

  for (auto r: reads)       delete r;
  for (auto o: overlaps)    delete o;

  return 0;
}
