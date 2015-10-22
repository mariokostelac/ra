#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "overlap2dot.h"
#include "cmdline/cmdline.h"
#include "ra/ra.hpp"

using std::cerr;
using std::cin;
using std::cout;
using std::fstream;
using std::string;
using std::vector;
using std::endl;

int main(int argc, char **argv) {
  cmdline::parser args;
  args.parse_check(argc, argv);

  vector<string> input_streams;

  for (uint i = 0; i < args.rest().size(); ++i) {
    input_streams.emplace_back(args.rest()[i]);
  }
  if (input_streams.size() == 0) {
    input_streams.emplace_back("-");
  }

  vector<Read*> reads;
  // FILL READS!!!!!

  vector<Overlap*> overlaps;

  for (auto stream_name : input_streams) {
    cerr << "Starting reading from " << stream_name << endl;
    FILE* fd = stream_name == "-" ? stdin : must_fopen(stream_name, "r");
    read_dovetail_overlaps(overlaps, reads, fd);
    fclose(fd);
  }

  cerr <<  "Read " << overlaps.size() << " overlaps." << endl;
  dot_graph(cout, overlaps);
}
