
#ifndef VERSION
#define VERSION "NO_VERSION"
#endif

#include "cmdline/cmdline.h"
#include "ra/ra.hpp"
#include <vector>

using std::string;
using std::vector;

// global vars
cmdline::parser args;
int thread_num;
string reads_format;
string reads_filename;
string depot_path;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("depot", 'd', "depot path", true);
  args.add<string>("reads", 'r', "reads file", false);
  args.add<string>("reads_format", 's', "reads format; supported: fasta, fastq, afg", false, "fasta");

  args.parse_check(argc, argv);
}

void read_args() {
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  reads_filename = args.get<string>("reads");
  reads_format = args.get<string>("reads_format");
  depot_path = args.get<string>("depot");
}

void import_reads_cmd() {
  vector<Read*> reads;

  fprintf(stderr, "Reading %s...\n", reads_filename.c_str());
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

  fprintf(stderr, "Filling store...\n");
  Depot* depot = new Depot(depot_path);

  depot->store_reads(reads);

  fprintf(stderr, "Store filled\n");

  for (auto r: reads)    delete r;
  delete depot;
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  if (args.rest().size() < 1) {
    fprintf(stderr, "Command not defined\n");
    args.usage();
    exit(1);
  }

  const string cmd = args.rest()[0];

  if (cmd == "import_reads") {
    import_reads_cmd();
  } else {
    fprintf(stderr, "Command '%s' not defined\n", cmd.c_str());
    args.usage();
    exit(1);
  }

  return 0;
}
