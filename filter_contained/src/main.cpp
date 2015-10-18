
#ifndef VERSION
#define VERSION "NO_VERSION"
#endif

#include "cmdline/cmdline.h"
#include "ra/ra.hpp"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sys/stat.h>
#include <vector>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::fstream;
using std::make_pair;
using std::map;
using std::max;
using std::min;
using std::pair;
using std::set;
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
    if (reads.size() <= o->a()) {
      fprintf(stderr, "Cannot find read %d\n", o->a());
      exit(1);
    };
    if (reads.size() <= o->b()) {
      fprintf(stderr, "Cannot find read %d\n", o->b());
      exit(1);
    };

    o->set_read_a(reads[o->a()]);
    o->set_read_b(reads[o->b()]);
  }

  vector<Overlap*> nocontainments;
  filterContainedOverlaps(nocontainments, overlaps, reads, true);

  write_overlaps(nocontainments, assembly_directory + "/overlaps.nocont");

  for (auto r: reads)    delete r;
  for (auto o: overlaps) delete o;

  return 0;
}
