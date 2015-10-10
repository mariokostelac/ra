
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
string reads_format;
string reads_filename;
string overlaps_filename;
string assembly_directory;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("directory", 'd', "assembly directory", false, ".");
  args.add<string>("reads", 'r', "reads file", true);
  args.add<string>("reads_format", 's', "reads format; supported: fasta, fastq, afg", false, "fasta");
  args.add<string>("overlaps", 'x', "overlaps file", true);

  args.parse_check(argc, argv);
}

void read_args() {
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  reads_filename = args.get<string>("reads");
  reads_format = args.get<string>("reads_format");
  overlaps_filename = args.get<string>("overlaps");
  assembly_directory = args.get<string>("directory");
}

void write_version(FILE* fd) {
  fprintf(fd, "# version: %s\n", VERSION);
}

void write_call_cmd(FILE* fd, int argc, char **argv) {
  fprintf(fd, "#");
  for (int i = 0; i < argc; ++i) {
    fprintf(fd, "%s ", argv[i]);
  }
  fprintf(fd, "\n");
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  vector<Overlap*> overlaps;
  vector<Read*> reads;

  if (reads_format == "fasta") {
    readFastaReads(reads, reads_filename.c_str());
  } else if (reads_format == "fastq") {
    readFastqReads(reads, reads_filename.c_str());
  } else if (reads_format == "afg") {
    readAfgReads(reads, reads_filename.c_str());
  } else {
    assert(false);
  }

  fprintf(stderr, "Read %lu reads\n", reads.size());

  fstream overlaps_file(overlaps_filename);
  MHAP::read_overlaps(overlaps_file, &overlaps);
  overlaps_file.close();

  fprintf(stderr, "Read %lu overlaps\n", overlaps.size());

  for (auto o : overlaps) {
    o->set_read_a(reads[o->a()]);
    o->set_read_b(reads[o->b()]);
  }

  vector<Overlap*> nocontainments;
  {
    vector<Overlap*> input;
    for (auto o : overlaps) {
      input.push_back(o);
    }

    filterContainedOverlaps(nocontainments, input, reads, true);
  }

  write_overlaps(nocontainments, assembly_directory + "/overlaps.nocont");

  for (auto r: reads)           delete r;
  for (auto o: overlaps)    delete o;

  return 0;
}
