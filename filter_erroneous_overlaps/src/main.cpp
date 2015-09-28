#ifndef VERSION
#define VERSION "NO_VERSION"
#endif

#include "cmdline/cmdline.h"
#include "ra/ra.hpp"
#include <algorithm>
#include <cassert>
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

void filter_bad_overlaps(vector<DovetailOverlap*>* dst, vector<DovetailOverlap*>& src) {
  *dst = src;

  // compress errates into buckets
  int buckets_size = 50;
  int divisor = 100 / buckets_size;

  int most_count = 0;
  int most_count_idx = -1;

  vector<int> bucket_count(buckets_size, 0);
  for (int i = 0, n = src.size(); i < n; ++i) {
    int bucket_idx = 100 * src[i]->errate() / 2;
    bucket_count[bucket_idx]++;
    most_count = max(most_count, bucket_count[bucket_idx]);

    if (most_count == bucket_count[bucket_idx]) {
      most_count_idx = bucket_idx;
    }
  }

  // find first bucket that contains elements
  int best_errate_idx = -1;
  for (int i = 0; i < buckets_size; ++i) {
    if (bucket_count[i] > 0) {
      best_errate_idx = i;
      break;
    }
  }
  assert(best_errate_idx >= 0);

  // scale errates back
  int best_errate = best_errate_idx * divisor;
  int most_counts_errate = most_count_idx * divisor;
  int worst_errate = most_counts_errate + (most_counts_errate - best_errate) + 1;

  fprintf(stderr, "Most overlaps have errate ~%d%%\n", most_counts_errate);
  fprintf(stderr, "Trimming all overlaps with errate higher than %d%%\n", worst_errate);

  std::sort(dst->begin(), dst->end(), [](const DovetailOverlap* a, const DovetailOverlap* b) {
      return a->errate() < b->errate();
  });

  int size = 0;
  for (uint32_t i = 0; i < dst->size(); ++i) {
    if (100 * (*dst)[i]->errate() > worst_errate) {
      break;
    }
    size = i;
  }

  dst->resize(size);
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
  filter_bad_overlaps(&filtered, overlaps);

  write_overlaps(filtered, nullptr);

  for (auto o: overlaps)    delete o;

  return 0;
}
