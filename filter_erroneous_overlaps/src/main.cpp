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

void init_specs(FILE *fd) {

  specs.load_settings(fd);

  MAX_ABSOLUTE_ERRATE = specs.get_or_store_double("overlap.max_abs_errate", 0.4);
}

void write_specs_to(const string path) {
  FILE* spec_file_fd = must_fopen(path, "w");
  specs.dump_settings(spec_file_fd);
  fclose(spec_file_fd);
}

void filter_overlaps_by_absolute_errate(OverlapSet* overlaps, double upper_limit) {
  fprintf(stderr, "Filtering overlaps with errate > %lf\n", upper_limit);

  int next_position = 0;
  for (int i = 0; i < (int) overlaps->size(); ++i) {
    auto o = (*overlaps)[i];
    if (o->err_rate() >= upper_limit) continue;

    (*overlaps)[next_position] = o;
    next_position++;
  }

  overlaps->resize(next_position);
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  if (spec_file_path.size() > 0) {
    FILE* spec_file_fd = must_fopen(spec_file_path, "r");
    init_specs(spec_file_fd);
    fclose(spec_file_fd);
  }

  vector<Read*> reads;
  vector<Overlap*> overlaps;

  Depot depot(depot_path);

  depot.load_reads(reads);
  fprintf(stderr, "Read %lu reads\n", reads.size());

  depot.load_overlaps(overlaps, reads);
  fprintf(stderr, "Read %lu overlaps\n", overlaps.size());

  filter_overlaps_by_absolute_errate(&overlaps, MAX_ABSOLUTE_ERRATE);

  fprintf(stderr, "Updating depot...\n");
  depot.store_overlaps(overlaps);

  for (auto r: reads)               delete r;
  for (auto o: overlaps)            delete o;

  write_specs_to(working_directory + "/filter_erroneous_overlaps.spec");

  return 0;
}
