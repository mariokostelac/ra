
#ifndef VERSION
#define VERSION "NO_VERSION"
#endif

#include "cmdline/cmdline.h"
#include "ra/ra.hpp"
#include <vector>
#include <set>

using std::fstream;
using std::string;
using std::vector;
using std::set;
using std::min;

const double MAX_OVERHANG_PERCENTAGE = 0.1;

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

void filterNonDovetailOverlaps(vector<Overlap*>* overlaps) {
  uint32_t idx = 0;
  for (uint32_t i = 0; i < overlaps->size(); ++i) {
    auto o = overlaps->at(i);
    double start_offset_a = o->a_lo() / (double) o->read_a()->length();
    double end_offset_a = (o->read_a()->length() - o->a_hi()) / (double) o->read_a()->length();
    double start_offset_b = o->b_lo() / (double) o->read_b()->length();
    double end_offset_b = (o->read_b()->length() - o->b_hi()) / (double) o->read_b()->length();

    double offset_a = min(start_offset_a, end_offset_a);
    double offset_b = min(start_offset_b, end_offset_b);

    if (offset_a > MAX_OVERHANG_PERCENTAGE || offset_b > MAX_OVERHANG_PERCENTAGE) {
      continue;
    }

    (*overlaps)[idx] = o;
    idx++;
  }

  overlaps->resize(idx);
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

  int size_before = overlaps.size();
  filterNonDovetailOverlaps(&overlaps);
  int size_after = overlaps.size();
  int non_dovetail = size_before - size_after;

  fprintf(stderr, "Filtered %d (%lf%%) overlaps\n", non_dovetail, 100. * non_dovetail/(double) size_before);

  vector<Overlap*> dovetail_overlaps(overlaps.size());
  set<const Read*> contained_reads;
  for (int i = 0; i < (int) overlaps.size(); ++i) {
    auto o = forcedDovetailOverlap(overlaps[i], true);
    if (o == nullptr) {
      continue;
    }

    dovetail_overlaps[i] = o;

    if (o->is_using_prefix(o->a()) && o->is_using_suffix(o->a())) {
      contained_reads.insert(o->read_a());
    }

    if (o->is_using_prefix(o->b()) && o->is_using_suffix(o->b())) {
      contained_reads.insert(o->read_b());
    }
  }

  int idx = 0;
  for (int i = 0; i < (int) dovetail_overlaps.size(); ++i) {
    auto o = dovetail_overlaps[i];
    if (o == nullptr) {
      continue;
    }

    if (contained_reads.count(o->read_a())) {
      fprintf(stderr, "Skipping %d %d because read %d is contained\n", o->a(), o->b(), o->a());
      continue;
    }
    if (contained_reads.count(o->read_b())) {
      fprintf(stderr, "Skipping %d %d because read %d is contained\n", o->a(), o->b(), o->b());
      continue;
    }

    dovetail_overlaps[idx] = o;
    idx++;
  }

  dovetail_overlaps.resize(idx);

  fprintf(stderr, "Writing %u overlaps to depot...\n", dovetail_overlaps.size());
  depot.store_overlaps(dovetail_overlaps);

  for (auto r: reads)               delete r;
  for (auto o: overlaps)            delete o;
  for (auto do_: dovetail_overlaps) delete do_;

  return 0;
}
