#include <fstream>
#include <vector>
#include <string>
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
  args.add<string>("format", 'f', "input file format; supported: afg, mhap", false, "afg");
  args.parse_check(argc, argv);

  vector<string> input_streams;

  for (uint i = 0; i < args.rest().size(); ++i) {
    input_streams.emplace_back(args.rest()[i]);
  }
  if (input_streams.size() == 0) {
    input_streams.emplace_back("-");
  }

  vector<Overlap*> overlaps;

  string format = args.get<string>("format");

  for (auto stream_name : input_streams) {
    cerr << "Starting reading from " << stream_name << endl;

    istream* input = stream_name == "-" ? &cin : new fstream(stream_name);

    if (format == "afg") {
      readAfgOverlaps(overlaps, *input);
    } else if (format == "mhap") {
      MHAP::read_overlaps(*input, &overlaps);
    }
  }

  cerr <<  "Read " << overlaps.size() << " overlaps." << endl;
  dot_graph(cout, overlaps);
}
