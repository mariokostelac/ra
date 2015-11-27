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
string depot_path;
string spec_file_path;
string working_directory;
Settings specs;

double MAX_ABSOLUTE_ERRATE;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("depot", 'd', "depot path", true);
  args.add<string>("spec_file", 's', "spec file path", false);
  args.add<string>("working_directory", 'w', "working directory", false, ".");

  args.parse_check(argc, argv);
}

void read_args() {
  depot_path = args.get<string>("depot");
  spec_file_path = args.get<string>("spec_file");
  working_directory = args.get<string>("working_directory");
}

void init_specs() {

  if (spec_file_path.size() > 0) {
    FILE* spec_file_fd = must_fopen(spec_file_path, "r");
    specs.load_settings(spec_file_fd);
    fclose(spec_file_fd);
  }

  MAX_ABSOLUTE_ERRATE = specs.get_or_store_double("overlap.max_abs_errate", 0.4);
}

void write_specs_to(const string path) {
  FILE* spec_file_fd = must_fopen(path, "w");
  specs.dump_settings(spec_file_fd);
  fclose(spec_file_fd);
}

void filter_overlaps_by_absolute_errate(OverlapSet* overlaps, double upper_limit) {
  fprintf(stderr, "Filtering overlaps with errate > %lf\n", upper_limit);

  int next_position = 0, size_before = overlaps->size();
  for (int i = 0; i < (int) overlaps->size(); ++i) {
    auto o = (*overlaps)[i];
    if (o->err_rate() >= upper_limit) continue;

    (*overlaps)[next_position] = o;
    next_position++;
  }

  overlaps->resize(next_position);

  int diff = size_before - overlaps->size();
  fprintf(stderr, "Filtered %d overlaps (%lf %%)\n", diff, 1.0 * diff / size_before);
}

void print_stats(OverlapSet& overlaps) {
  fprintf(stderr, "Statistic\terror_rate\tcount\n");

  int index = 0.5 * overlaps.size();
  fprintf(stderr, "p50\t%lf\t%d\n", overlaps[index]->err_rate(), index);

  index = 0.6 * overlaps.size();
  fprintf(stderr, "p60\t%lf\t%d\n", overlaps[index]->err_rate(), index);

  index = 0.7 * overlaps.size();
  fprintf(stderr, "p70\t%lf\t%d\n", overlaps[index]->err_rate(), index);

  index = 0.8 * overlaps.size();
  fprintf(stderr, "p80\t%lf\t%d\n", overlaps[index]->err_rate(), index);

  index = 0.9 * overlaps.size();
  fprintf(stderr, "p90\t%lf\t%d\n", overlaps[index]->err_rate(), index);

  index = 0.95 * overlaps.size();
  fprintf(stderr, "p95\t%lf\t%d\n", overlaps[index]->err_rate(), index);

  index = 0.99 * overlaps.size();
  fprintf(stderr, "p99\t%lf\t%d\n", overlaps[index]->err_rate(), index);
}

bool compare_by_errate(Overlap* a, Overlap* b) {
  return a->err_rate() < b->err_rate();
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();
  init_specs();

  vector<Read*> reads;
  vector<Overlap*> overlaps;

  Depot depot(depot_path);

  depot.load_reads(reads);
  fprintf(stderr, "Read %lu reads\n", reads.size());

  depot.load_overlaps(overlaps, reads);
  fprintf(stderr, "Read %lu overlaps\n", overlaps.size());

  sort(overlaps.begin(), overlaps.end(), compare_by_errate);

  print_stats(overlaps);
  filter_overlaps_by_absolute_errate(&overlaps, MAX_ABSOLUTE_ERRATE);
  print_stats(overlaps);

  fprintf(stderr, "Updating depot...\n");
  depot.store_overlaps(overlaps);

  for (auto r: reads)               delete r;
  for (auto o: overlaps)            delete o;

  write_specs_to(working_directory + "/filter_erroneous_overlaps.spec");

  return 0;
}
