
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
string settings_file;
string assembly_directory;

void must_one_overlap_per_pair(const vector<DovetailOverlap*>& overlaps) {
  set<pair<uint32_t, uint32_t>> seen;

  for (const auto& overlap: overlaps) {
    uint32_t a = min(overlap->a(), overlap->b());
    uint32_t b = max(overlap->a(), overlap->b());

    if (seen.count(make_pair(a, b)) > 0) {
      fprintf(stderr, "Read pair (%d, %d) has more than one overlap\n", a, b);
      exit(1);
    }

    seen.insert(make_pair(a, b));
  }
}

uint32_t filter_best_overlap_per_pair(vector<DovetailOverlap*>* overlaps) {
  map<pair<uint32_t, uint32_t>, DovetailOverlap*> best;

  for (const auto& overlap: *overlaps) {
    uint32_t a = min(overlap->a(), overlap->b());
    uint32_t b = max(overlap->a(), overlap->b());

    auto key = make_pair(a, b);
    if (best.count(key)) {
      if (overlap->score() > best[key]->score()) {
        best[key] = overlap;
      }

    } else {
      best[key] = overlap;
    }
  }

  int removed = 0;
  for (uint32_t i = 0; i < overlaps->size(); ++i) {
    const auto& overlap = (*overlaps)[i];
    uint32_t a = min(overlap->a(), overlap->b());
    uint32_t b = max(overlap->a(), overlap->b());

    auto key = make_pair(a, b);
    if (best[key] != overlap) {
      removed++;
      delete (*overlaps)[i];
    }
  }

  uint32_t idx = 0;
  for (auto kv: best) {
    (*overlaps)[idx] = kv.second;
    idx++;
  }

  overlaps->resize(best.size());

  return removed;
}

int filter_overlaps_by_min_read_len(vector<DovetailOverlap*>* overlaps, const uint32_t min_length) {
  int skipped = 0;

  for (uint32_t i = 0; i < overlaps->size(); ++i) {
    const auto o = (*overlaps)[i];

    if (o->read_a()->getLength() < min_length || o->read_b()->getLength() < min_length) {
      skipped++;
      continue;
    }

    (*overlaps)[i - skipped] = (*overlaps)[i];
  }
  overlaps->resize(overlaps->size() - skipped);

  return skipped;
}

int filter_overlaps_by_min_qual(vector<DovetailOverlap*>* overlaps, const double min_qual) {
  int skipped = 0;

  for (uint32_t i = 0; i < overlaps->size(); ++i) {
    const auto o = (*overlaps)[i];

    if (o->quality() < min_qual) {
      skipped++;
      continue;
    }

    (*overlaps)[i - skipped] = (*overlaps)[i];
  }
  overlaps->resize(overlaps->size() - skipped);

  return skipped;
}

void print_contigs_info(const vector<StringGraphWalk*>& walks, const vector<Read*>& reads) {

  for (uint32_t i = 0; i < walks.size(); ++i) {
    Contig contig(walks[i]);
    const auto& parts = contig.getParts();
    const auto& last_part = contig.getParts().back();

    fprintf(stdout, "contig %u; length: ≈%lu, reads: %lu\n",
        i, last_part.offset + reads[last_part.src]->getLength(), parts.size()
    );
    for (const auto& p: parts) {
      fprintf(stdout, "%d ", p.src);
    }
    fprintf(stdout, "\n");
  }
}

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("reads", 'r', "reads file", true);
  args.add<string>("reads_format", 's', "reads format; supported: fasta, fastq, afg", false, "fasta");
  args.add<string>("overlaps", 'x', "overlaps file", true);
  args.add<string>("settings", 'b', "settings file", false);
  args.add<string>("directory", 'd', "assembly_directory", false, ".");

  args.parse_check(argc, argv);
}

void read_args() {
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  reads_filename = args.get<string>("reads");
  reads_format = args.get<string>("reads_format");
  overlaps_filename = args.get<string>("overlaps");
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

int extract_contig_walks(std::vector<StringGraphWalk*>* contig_walks, const StringGraph* graph) {

  int written = 0;

  std::vector<StringGraphComponent*> components;
  graph->extractComponents(components);

  for (const auto& component : components) {
    const auto& contig_walk_orig = component->longestWalk();

    if (contig_walk_orig == nullptr) {
      continue;
    }

    StringGraphWalk* contig_walk_cpy = new StringGraphWalk(*contig_walk_orig);
    contig_walks->emplace_back(contig_walk_cpy);
    written++;
  }

  for (auto c : components) {
    delete c;
  }

  return written;
}

void write_contigs_to_file(const vector<StringGraphWalk*> walks, const char* filename) {

  int idx = 0;

  auto contigs_fast = must_fopen(filename, "w");
  for (const auto& walk : walks) {
    string seq;
    walk->extractSequence(seq);

    fprintf(contigs_fast, ">seq%d|len:%lu\n", idx, seq.size());
    fprintf(contigs_fast, "%s\n", seq.c_str());
    idx++;
  }
  fclose(contigs_fast);
}

int main(int argc, char **argv) {

  init_args(argc, argv);
  read_args();

  if (settings_file.size() > 0) {
    FILE* settings_fd = must_fopen(settings_file.c_str(), "r");
    read_settings(settings_fd);
    fclose(settings_fd);
  }

  auto run_args_file = must_fopen((assembly_directory + "/run_args.txt").c_str(), "w");
  write_version(run_args_file);
  write_call_cmd(run_args_file, argc, argv);
  write_settings(run_args_file);

  write_version(stderr);
  write_call_cmd(stderr, argc, argv);
  write_settings(stderr);
  fclose(run_args_file);

  vector<DovetailOverlap*> all_overlaps, overlaps, filtered;
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

  std::cerr << "Read " << reads.size() << " reads" << std::endl;

  readAfgOverlaps(all_overlaps, overlaps_filename.c_str());

  overlaps = all_overlaps;

  int had_overlaps = overlaps.size();
  fprintf(stderr, "%lu overlaps read\n", overlaps.size());

  auto filtered_duplicates = filter_best_overlap_per_pair(&overlaps);
  fprintf(stderr, "%d (%.2lf%%) overlaps filtered because had more than one overlap per read pair\n",
      filtered_duplicates, 100. * filtered_duplicates / had_overlaps);

  must_one_overlap_per_pair(overlaps);

  for (auto o: overlaps) {
    const auto a = o->a();
    const auto b = o->b();
    if (reads[a] == nullptr) {
      cerr << "Read " << a << " not found" << endl;
      exit(1);
    }
    if (reads[b] == nullptr) {
      cerr << "Read " << b << " not found" << endl;
      exit(1);
    }

    o->set_read_a(reads[a]);
    o->set_read_b(reads[b]);
  }

  had_overlaps = overlaps.size();
  int bad_len_filtered = filter_overlaps_by_min_read_len(&overlaps, READS_MIN_LEN);
  fprintf(stderr, "%d (%.2lf%%) overlaps filtered because incident reads shorter than %lu\n",
      bad_len_filtered, 100. * bad_len_filtered / had_overlaps, READS_MIN_LEN);

  had_overlaps = overlaps.size();
  int bad_qual_filtered = filter_overlaps_by_min_qual(&overlaps, OVERLAPS_MIN_QUALITY);
  fprintf(stderr, "%d (%.2lf%%) overlaps filtered because quality worse than %0.4lf\n",
      bad_qual_filtered, 100. * bad_qual_filtered / had_overlaps, OVERLAPS_MIN_QUALITY);

  createReverseComplements(reads, thread_num);

  StringGraph* graph = new StringGraph(reads, overlaps);
  graph->simplify();

  vector<Overlap*> simplified_overlaps;
  graph->extractOverlaps(simplified_overlaps);
  writeOverlaps(simplified_overlaps, (assembly_directory + "/simplified.afg").c_str());

  fprintf(stderr, "Simplified string graph: %lu vertices, %lu edges\n", graph->getNumVertices(), graph->getNumEdges());

  std::vector<StringGraphWalk*> unitig_walks;
  graph->extract_unitigs(&unitig_walks);
  write_contigs_to_file(unitig_walks, (assembly_directory + "/unitigs_fast.fasta").c_str());

  std::cerr << "number of unitigs " << unitig_walks.size() << std::endl;

  vector<Contig*> unitigs;
  for (const auto& unitig_walk : unitig_walks) {
    Contig *unitig = new Contig(unitig_walk);
    unitigs.push_back(unitig);
  }
  writeAfgContigs(unitigs, (assembly_directory + "/unitigs.afg").c_str());

  std::vector<StringGraphWalk*> contig_walks;
  extract_contig_walks(&contig_walks, graph);

  write_contigs_to_file(contig_walks, (assembly_directory + "/contigs_fast.fasta").c_str());

  std::cerr << "number of contigs " << contig_walks.size() << std::endl;
  print_contigs_info(contig_walks, reads);

  vector<Contig*> contigs;
  for (const auto& contig_walk : contig_walks) {
    Contig *contig = new Contig(contig_walk);
    contigs.push_back(contig);
  }
  writeAfgContigs(contigs, (assembly_directory + "/contigs.afg").c_str());

  for (auto r: reads)           delete r;
  for (auto o: all_overlaps)    delete o;
  for (auto c: contig_walks)    delete c;
  for (auto u: unitig_walks)    delete u;
  for (auto c: contigs)         delete c;

  delete graph;

  return 0;
}