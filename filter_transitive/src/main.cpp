
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
string reads_filename;
string overlaps_filename;
string assembly_directory;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("reads", 'r', "reads file", true);
  args.add<string>("overlaps", 'x', "overlaps file", true);
  args.add<string>("directory", 'd', "assembly_directory", false, ".");

  args.parse_check(argc, argv);
}

void read_args() {
  assembly_directory = args.get<string>("directory");
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  overlaps_filename = args.get<string>("overlaps");
  reads_filename = args.get<string>("reads");
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  vector<Read*> reads;
  readFastaReads(reads, reads_filename.c_str());

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

  for (auto o: overlaps)    delete o;

  return 0;
}
