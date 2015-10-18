
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
string overlaps_filename;
string assembly_directory;
string depot_path;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("directory", 'd', "assembly directory", false, ".");
  args.add<string>("overlaps", 'x', "overlaps file", true);
  args.add<string>("depot", 'D', "depot path", true);

  args.parse_check(argc, argv);
}

void read_args() {
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  overlaps_filename = args.get<string>("overlaps");
  assembly_directory = args.get<string>("directory");
  depot_path = args.get<string>("depot");
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  vector<Read*> reads;

  Depot depot(depot_path);
  depot.load_reads(reads);

  fprintf(stderr, "Read %lu reads\n", reads.size());

  vector<Overlap*> overlaps;

  fstream overlaps_file(overlaps_filename);
  MHAP::read_overlaps(overlaps_file, &overlaps);
  overlaps_file.close();

  fprintf(stderr, "Read %lu overlaps\n", overlaps.size());

  for (auto o : overlaps) {
    o->set_read_a(reads[o->a()]);
    o->set_read_b(reads[o->b()]);
  }

  vector<Overlap*> dovetail_overlaps;
  for (auto o : overlaps) {
    dovetail_overlaps.push_back(forced_dovetail_overlap(o, true));
  }

  write_overlaps(dovetail_overlaps, assembly_directory + "/overlaps.dovetail");

  for (auto r: reads)               delete r;
  for (auto o: overlaps)            delete o;
  for (auto do_: dovetail_overlaps) delete do_;

  return 0;
}
