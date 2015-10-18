
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
string reads_filename;
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
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  overlaps_filename = args.get<string>("overlaps");
  depot_path = args.get<string>("depot");
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  vector<Read*> reads;
  Depot depot(depot_path);
  depot.load_reads(reads);
  fprintf(stderr, "%lu reads read\n", reads.size());

  vector<DovetailOverlap*> overlaps;
  FILE *overlaps_fd = must_fopen(overlaps_filename, "r");
  read_dovetail_overlaps(&overlaps, overlaps_fd);
  fclose(overlaps_fd);

  fprintf(stderr, "%lu overlaps read\n", overlaps.size());

  for (auto o : overlaps) {
    o->set_read_a(reads[o->a()]);
    o->set_read_b(reads[o->b()]);
  }

  vector<DovetailOverlap*> notransitives;
  filterTransitiveOverlaps(notransitives, overlaps, thread_num, true);

  write_overlaps(notransitives, assembly_directory + "/overlaps.notran");

  for (auto r: reads)       delete r;
  for (auto o: overlaps)    delete o;

  return 0;
}
