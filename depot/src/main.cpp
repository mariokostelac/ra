
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
int thread_num;
string reads_format;
string reads_filename;
string overlaps_filename;
string depot_path;

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("depot", 'd', "depot path", true);
  args.add<string>("reads", 'r', "reads file", false);
  args.add<string>("overlaps", 'x', "overlaps file", false);
  args.add<string>("reads_format", 's', "reads format; supported: fasta, fastq, afg", false, "fasta");

  args.parse_check(argc, argv);
}

void read_args() {
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  depot_path = args.get<string>("depot");
  reads_filename = args.get<string>("reads");
  reads_format = args.get<string>("reads_format");
  overlaps_filename = args.get<string>("overlaps");
}

void load_reads(vector<Read*>* reads) {
  if (reads_filename.size() == 0) {
    fprintf(stderr, "Reads filename is not provided\n");
    exit(1);
  }

  fprintf(stderr, "Reading %s...\n", reads_filename.c_str());
  if (reads_format == "fasta") {
    readFastaReads(*reads, reads_filename.c_str());
  } else if (reads_format == "fastq") {
    readFastqReads(*reads, reads_filename.c_str());
  } else if (reads_format == "afg") {
    readAfgReads(*reads, reads_filename.c_str());
  } else {
    assert(false);
  }

  fprintf(stderr, "Read %lu reads\n", reads->size());
}

void import_reads_cmd() {
  vector<Read*> reads;

  load_reads(&reads);

  fprintf(stderr, "Filling depot with reads...\n");
  Depot depot(depot_path);

  depot.store_reads(reads);

  fprintf(stderr, "Depot filled\n");

  for (auto r: reads)    delete r;
}

void import_overlaps_cmd() {
  if (overlaps_filename.size() == 0) {
    fprintf(stderr, "Overlaps filename is not provided\n");
    exit(1);
  }

  vector<Read*> reads;
  vector<Overlap*> overlaps;

  Depot depot(depot_path);

  fprintf(stderr, "Reading reads from depot...\n");
  depot.load_reads(reads);

  if (reads.size() == 0) {
    fprintf(stderr, "Read 0 reads. Reads have to be imported to depot!\n");
    exit(1);
  }

  fstream overlaps_file(overlaps_filename);
  MHAP::read_overlaps(overlaps, reads, overlaps_file);
  overlaps_file.close();
  fprintf(stderr, "Read %lu overlaps\n", overlaps.size());

  fprintf(stderr, "Filling depot with overlaps...\n");

  depot.store_overlaps(overlaps);

  fprintf(stderr, "Depot filled\n");

  for (auto r: reads)     delete r;
  for (auto o: overlaps)  delete o;
}

void dump_overlaps_cmd() {
  vector<Read*> reads;
  vector<Overlap*> overlaps;

  Depot depot(depot_path);

  fprintf(stderr, "Reading reads...\n");
  depot.load_reads(reads);
  fprintf(stderr, "Read %lu reads\n", reads.size());

  fprintf(stderr, "Reading overlaps...\n");
  depot.load_overlaps(overlaps, reads);
  fprintf(stderr, "Read %lu overlaps\n", overlaps.size());

  fprintf(stdout, "a_id\tb_id\ttype\ta_lo\ta_hi\ta_len\tb_lo\tb_hi\tb_len\torig_error\twiden_error\n");

  for (auto o : overlaps) {
    fprintf(stdout, "%d\t%d\t%c\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\t\n",
        o->a(), o->b(), o->is_innie() ? 'I' : 'N',
        o->a_lo(), o->a_hi(), o->read_a()->length(),
        o->b_lo(), o->b_hi(), o->read_b()->length(),
        o->orig_err_rate(), o->err_rate()
    );
  }
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
  } else if (cmd == "import_overlaps") {
    import_overlaps_cmd();
  } else if (cmd == "dump_overlaps") {
    dump_overlaps_cmd();
  } else {
    fprintf(stderr, "Command '%s' not defined\n", cmd.c_str());
    args.usage();
    exit(1);
  }

  return 0;
}
