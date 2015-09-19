
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

// trimming params
int READ_LEN_THRESHOLD = 100000;
uint32_t MAX_READS_IN_TIP = 2;
uint32_t MAX_DEPTH_WITHOUT_EXTRA_FORK = 5;

// BFS params in bubble popping
size_t MAX_NODES = 160;
int MAX_DISTANCE = MAX_NODES * 10000;
double MAX_DIFFERENCE = 0.25;

// contig extraction params
size_t MAX_BRANCHES = 18;
size_t MAX_START_NODES = 100;
double LENGTH_THRESHOLD = 0.05;
double QUALITY_THRESHOLD = 0.2;

// filter reads param
size_t READS_MIN_LEN = 3000;

// filter overlaps param
double OVERLAPS_MIN_QUALITY = 0;

// global vars
cmdline::parser args;
int thread_num;
string reads_format;
string reads_filename;
string overlaps_filename;
string overlaps_format;
int reads_id_offset;
string settings_file;
string assembly_directory;

// map reads so we can access reads with mapped[read_id]
void map_reads(vector<Read*>* mapped, vector<Read*>& reads) {

  int max_id = -1;
  for (auto r: reads) {
    max_id = max(max_id, r->getId());
  }

  mapped->resize(max_id + 1, nullptr);
  for (auto r: reads) {
    (*mapped)[r->getId()] = r;
  }
}

void must_mkdir(const string& path) {
    if (mkdir(path.c_str(), 0755) == -1) {
        fprintf(stderr, "Can't create directory %s\n", path.c_str());
        exit(1);
    }
}

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("directory", 'd', "assembly directory", false, ".");
  args.add<string>("reads", 'r', "reads file", true);
  args.add<string>("reads_format", 's', "reads format; supported: fasta, fastq, afg", false, "fasta");
  args.add<int>("reads_id_offset", 'a', "reads id offset (first read id)", false, 0);
  args.add<string>("overlaps", 'x', "overlaps file", true);
  args.add<string>("overlaps_format", 'f', "overlaps file format; supported: afg, mhap", false, "afg");
  args.add<string>("settings", 'b', "settings file", false);

  args.parse_check(argc, argv);
}

void read_args() {
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  reads_filename = args.get<string>("reads");
  reads_format = args.get<string>("reads_format");
  overlaps_filename = args.get<string>("overlaps");
  overlaps_format = args.get<string>("overlaps_format");
  reads_id_offset = args.get<int>("reads_id_offset");
  settings_file = args.get<string>("settings");
  assembly_directory = args.get<string>("directory");
}

FILE* must_fopen(const char* path, const char* mode) {
  FILE* res = fopen(path, mode);
  if (res == nullptr) {
    fprintf(stderr, "Cannot open %s with mode %s\n", path, mode);
    exit(1);
  }

  return res;
}

void read_settings(FILE *fd) {
  char buff[4096];

  while (fgets(buff, 4096, fd) != nullptr) {

    // comment
    if (buff[0] == '#') continue;

    if (sscanf(buff, "READS_MIN_LEN: %lu", &READS_MIN_LEN)) {
      debug("READ READS_MIN_LEN: %lu from file\n", READS_MIN_LEN);
    } else if (sscanf(buff, "READ_LEN_THRESHOLD: %d", &READ_LEN_THRESHOLD)) {
      debug("READ READ_LEN_THRESHOLD: %lu from file\n", READ_LEN_THRESHOLD);
    } else if (sscanf(buff, "MAX_READS_IN_TIP: %d", &MAX_READS_IN_TIP)) {
      debug("READ MAX_READS_IN_TIP: %lu from file\n", MAX_READS_IN_TIP);
    } else if (sscanf(buff, "MAX_DEPTH_WITHOUT_EXTRA_FORK: %d", &MAX_DEPTH_WITHOUT_EXTRA_FORK)) {
      debug("READ MAX_DEPTH_WITHOUT_EXTRA_FORK: %lu from file\n", MAX_DEPTH_WITHOUT_EXTRA_FORK);
    } else if (sscanf(buff, "MAX_NODES: %lu", &MAX_NODES)) {
      debug("READ MAX_NODES: %lu from file\n", MAX_NODES);
    } else if (sscanf(buff, "MAX_DISTANCE: %d", &MAX_DISTANCE)) {
      debug("READ MAX_DISTANCE: %lu from file\n", MAX_DISTANCE);
    } else if (sscanf(buff, "MAX_DIFFERENCE: %lf", &MAX_DIFFERENCE)) {
      debug("READ MAX_DIFFERENCE: %lf from file\n", MAX_DIFFERENCE);
    } else if (sscanf(buff, "MAX_BRANCHES: %lu", &MAX_BRANCHES)) {
      debug("READ MAX_BRANCHES: %lu from file\n", MAX_BRANCHES);
    } else if (sscanf(buff, "MAX_START_NODES: %lu", &MAX_START_NODES)) {
      debug("READ MAX_START_NODES: %lu from file\n", MAX_START_NODES);
    } else if (sscanf(buff, "LENGTH_THRESHOLD: %lf", &LENGTH_THRESHOLD)) {
      debug("READ LENGTH_THRESHOLD: %lf from file\n", LENGTH_THRESHOLD);
    } else if (sscanf(buff, "QUALITY_THRESHOLD: %lf", &QUALITY_THRESHOLD)) {
      debug("READ QUALITY_THRESHOLD: %lf from file\n", QUALITY_THRESHOLD);
    } else if (sscanf(buff, "OVERLAPS_MIN_QUALITY: %lf", &OVERLAPS_MIN_QUALITY)) {
      debug("READ OVERLAPS_MIN_QUALITY: %lf from file\n", OVERLAPS_MIN_QUALITY);
    }
  }
}

void write_settings(FILE *fd) {
  fprintf(fd, "# filter reads parameters\n");
  fprintf(fd, "READS_MIN_LEN: %lu\n", READS_MIN_LEN);
  fprintf(fd, "\n");

  fprintf(fd, "# filter overlaps parameters\n");
  fprintf(fd, "OVERLAPS_MIN_QUALITY: %lf\n", OVERLAPS_MIN_QUALITY);
  fprintf(fd, "\n");

  fprintf(fd, "# trimming parameters\n");
  fprintf(fd, "READ_LEN_THRESHOLD: %d\n", READ_LEN_THRESHOLD);
  fprintf(fd, "MAX_READS_IN_TIP: %d\n", MAX_READS_IN_TIP);
  fprintf(fd, "MAX_DEPTH_WITHOUT_EXTRA_FORK: %d\n", MAX_DEPTH_WITHOUT_EXTRA_FORK);
  fprintf(fd, "\n");

  fprintf(fd, "# bubble popping parameters\n");
  fprintf(fd, "MAX_NODES: %lu\n", MAX_NODES);
  fprintf(fd, "MAX_DISTANCE: %d\n", MAX_DISTANCE);
  fprintf(fd, "MAX_DIFFERENCE: %f\n", MAX_DIFFERENCE);
  fprintf(fd, "\n");

  fprintf(fd, "# contig extraction parameters\n");
  fprintf(fd, "MAX_BRANCHES: %lu\n", MAX_BRANCHES);
  fprintf(fd, "MAX_START_NODES: %lu\n", MAX_START_NODES);
  fprintf(fd, "LENGTH_THRESHOLD: %f\n", LENGTH_THRESHOLD);
  fprintf(fd, "QUALITY_THRESHOLD: %f\n", QUALITY_THRESHOLD);
  fprintf(fd, "\n");
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

  if (settings_file.size() > 0) {
    FILE* settings_fd = must_fopen(settings_file.c_str(), "r");
    read_settings(settings_fd);
    fclose(settings_fd);
  }

  vector<Overlap*> all_overlaps, overlaps, filtered;
  vector<Read*> reads;
  vector<Read*> reads_mapped;

  if (reads_format == "fasta") {
    readFastaReads(reads, reads_filename.c_str());
  } else if (reads_format == "fastq") {
    readFastqReads(reads, reads_filename.c_str());
  } else if (reads_format == "afg") {
    readAfgReads(reads, reads_filename.c_str());
  } else {
    assert(false);
  }

  // map reads so we have reads_mapped[read_id] -> read
  map_reads(&reads_mapped, reads);

  std::cerr << "Read " << reads.size() << " reads" << std::endl;

  if (overlaps_format == "afg") {
    readAfgOverlaps(all_overlaps, overlaps_filename.c_str());
  } else if (overlaps_format == "mhap") {
    fstream overlaps_file(overlaps_filename);
    MHAP::read_overlaps(overlaps_file, &all_overlaps);
    overlaps_file.close();
  } else {
    assert(false);
  }

  overlaps = all_overlaps;

  fprintf(stderr, "%lu overlaps read\n", overlaps.size());

  // fix overlap read ids
  for (auto o: overlaps) {
    o->setA(o->getA() - reads_id_offset);
    o->setB(o->getB() - reads_id_offset);
  }

  for (auto o: overlaps) {
    const auto a = o->getA();
    const auto b = o->getB();
    if (reads_mapped[a] == nullptr) {
      cerr << "Read " << a << " not found" << endl;
      exit(1);
    }
    if (reads_mapped[b] == nullptr) {
      cerr << "Read " << b << " not found" << endl;
      exit(1);
    }

    o->setReadA(reads_mapped[a]);
    o->setReadB(reads_mapped[b]);
  }

  vector<Overlap*> nocontainments;
  filterContainedOverlaps(nocontainments, overlaps, reads_mapped, true);

  writeOverlaps(nocontainments, (assembly_directory + "/nocont." + overlaps_format).c_str());

  for (auto r: reads)           delete r;
  for (auto o: all_overlaps)    delete o;

  return 0;
}
