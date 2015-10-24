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

int main(int argc, char **argv) {
  cmdline::parser args;

  args.parse_check(argc, argv);

  if (args.rest().size() != 2) {
    fprintf(stderr, "Invocation: %s <reads.fasta> <overlaps.mhap>\n", argv[0]);
    exit(1);
  }

  string reads_filename = args.rest()[0];
  string overlaps_filename = args.rest()[1];

  vector<Read*> reads;
  {
    fprintf(stderr, "Reading from %s...\n", reads_filename.c_str());
    readFastaReads(reads, reads_filename.c_str());

    fprintf(stderr, "Read %lu reads\n", reads.size());
  }

  vector<Overlap*> overlaps;
  {
    fprintf(stderr, "Reading from %s...\n", overlaps_filename.c_str());
    FILE* fd = overlaps_filename == "-" ? stdin : must_fopen(overlaps_filename, "r");
    read_dovetail_overlaps(overlaps, reads, fd);
    fclose(fd);

    fprintf(stderr, "Read %lu overlaps\n", overlaps.size());
  }

  dot_graph(cout, overlaps);
}
