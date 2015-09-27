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

// trimming params
int READ_LEN_THRESHOLD = 100000;
uint32_t MAX_READS_IN_TIP = 2;
uint32_t MAX_DEPTH_WITHOUT_EXTRA_FORK = 5;

// BFS params in bubble popping
size_t MAX_NODES = 160;
int MAX_DISTANCE = MAX_NODES * 10000;
double MAX_DIFFERENCE = 0.25;

// contig extraction params
size_t MAX_BRANCHES = 18;
size_t MAX_START_NODES = 100;
double LENGTH_THRESHOLD = 0.05;
double QUALITY_THRESHOLD = 0.2;

// filter reads param
size_t READS_MIN_LEN = 3000;

// filter overlaps param
double OVERLAPS_MIN_QUALITY = 0;

// global vars
cmdline::parser args;
int thread_num;
string overlaps_filename;
string assembly_directory;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("overlaps", 'x', "overlaps file", true);
  args.add<string>("directory", 'd', "assembly_directory", false, ".");

  args.parse_check(argc, argv);
}

void read_args() {
  assembly_directory = args.get<string>("directory");
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  overlaps_filename = args.get<string>("overlaps");
}

void filter_bad_overlaps(vector<DovetailOverlap*>* dst, vector<DovetailOverlap*>& src, double percentage) {
  *dst = src;

  std::sort(dst->begin(), dst->end(), [](const DovetailOverlap* a, const DovetailOverlap* b) {
      return a->errate() < b->errate();
  });

  dst->resize(dst->size() * (1 - percentage));
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  vector<DovetailOverlap*> overlaps;
  FILE *overlaps_fd = must_fopen(overlaps_filename, "r");
  read_dovetail_overlaps(&overlaps, overlaps_fd);
  fclose(overlaps_fd);

  fprintf(stderr, "%lu overlaps read\n", overlaps.size());

  vector<DovetailOverlap*> filtered;
  filter_bad_overlaps(&filtered, overlaps, 0.05);

  write_overlaps(filtered, nullptr);

  for (auto o: overlaps)    delete o;

  return 0;
}
