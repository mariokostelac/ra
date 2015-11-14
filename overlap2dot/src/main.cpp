#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "overlap2dot.h"
#include "cmdline/cmdline.h"
#include "ra/ra.hpp"

using std::cout;
using std::string;
using std::vector;
using std::endl;

cmdline::parser args;

void load_reads(vector<Read*>* reads, const string reads_path, const string reads_format) {
  if (reads_path.size() == 0) {
    fprintf(stderr, "Reads filename is not provided\n");
    exit(1);
  }

  fprintf(stderr, "Reading %s...\n", reads_path.c_str());
  if (reads_format == "fasta") {
    readFastaReads(*reads, reads_path.c_str());
  } else if (reads_format == "fastq") {
    readFastqReads(*reads, reads_path.c_str());
  } else if (reads_format == "afg") {
    readAfgReads(*reads, reads_path.c_str());
  } else {
    assert(false);
  }

  fprintf(stderr, "Read %lu reads\n", reads->size());
}

void load_overlaps(vector<Overlap*>* overlaps, vector<Read*>& reads, const string overlaps_path) {
  fprintf(stderr, "Reading from %s...\n", overlaps_path.c_str());
  FILE* fd = overlaps_path == "-" ? stdin : must_fopen(overlaps_path, "r");
  read_dovetail_overlaps(*overlaps, reads, fd);
  fclose(fd);

  fprintf(stderr, "Read %lu overlaps\n", overlaps->size());
}

void init_args(int argc, char** argv) {
  args.add<string>("reads", 'r', "reads file", false);
  args.add<string>("overlaps", 'x', "overlaps file", false);
  args.add<string>("reads_format", 's', "reads format; supported: fasta, fastq, afg", false, "fasta");

  args.parse_check(argc, argv);
}

int main(int argc, char **argv) {
  init_args(argc, argv);

  string reads_path = args.get<string>("reads");
  string reads_format = args.get<string>("reads_format");
  string overlaps_path = args.get<string>("overlaps");

  vector<Read*> reads;
  vector<Overlap*> overlaps;
  load_reads(&reads, reads_path, reads_format);
  load_overlaps(&overlaps, reads, overlaps_path);

  dot_graph(cout, overlaps);
}
