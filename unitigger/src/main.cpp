
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
string settings_file;
string depot_path;
string working_directory;

void must_one_overlap_per_pair(const vector<Overlap*>& overlaps) {
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

uint32_t filter_best_overlap_per_pair(vector<Overlap*>* overlaps) {
  map<pair<uint32_t, uint32_t>, Overlap*> best;

  for (const auto& overlap: *overlaps) {
    uint32_t a = min(overlap->a(), overlap->b());
    uint32_t b = max(overlap->a(), overlap->b());

    auto key = make_pair(a, b);
    if (best.count(key)) {
      if (overlap->err_rate() < best[key]->err_rate()) {
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

void print_contigs_info(const vector<StringGraphWalk*>& walks, const vector<Read*>& reads) {

  for (uint32_t i = 0; i < walks.size(); ++i) {
    Contig contig(walks[i]);
    const auto& parts = contig.getParts();
    const auto& last_part = contig.getParts().back();

    fprintf(stdout, "contig %u; length: ≈%u, reads: %lu\n",
        i, last_part.offset + reads[last_part.src]->length(), parts.size()
    );
    for (const auto& p: parts) {
      fprintf(stdout, "%d ", p.src);
    }
    fprintf(stdout, "\n");
  }
}

void init_args(int argc, char** argv) {
  // input params
  args.add<string>("settings", 'b', "settings file", false);
  args.add<string>("depot", 'd', "depot path", true);
  args.add<string>("working_directory", 'w', "working directory", false, ".");

  args.parse_check(argc, argv);
}

void read_args() {
  thread_num = std::max(std::thread::hardware_concurrency(), 1U);
  settings_file = args.get<string>("settings");
  depot_path = args.get<string>("depot");
  working_directory = args.get<string>("working_directory");
}

void read_settings(FILE *fd) {
  char buff[4096];

  while (fgets(buff, 4096, fd) != nullptr) {

    // comment
    if (buff[0] == '#') continue;

    if (sscanf(buff, "READ_LEN_THRESHOLD: %d", &READ_LEN_THRESHOLD)) {
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
    }
  }
}

void write_settings(FILE *fd) {
  fprintf(fd, "# filter reads parameters\n");

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

    fprintf(stderr, "Extracting contig from graph component with %lu vertices\n", component->vertices().size());

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
    FILE* settings_fd = must_fopen(settings_file, "r");
    read_settings(settings_fd);
    fclose(settings_fd);
  }

  auto run_args_file = must_fopen(working_directory + "/run_args.txt", "w");
  write_version(run_args_file);
  write_call_cmd(run_args_file, argc, argv);
  write_settings(run_args_file);

  write_version(stderr);
  write_call_cmd(stderr, argc, argv);
  write_settings(stderr);
  fclose(run_args_file);

  vector<Read*> reads;
  vector<Overlap*> overlaps;

  Depot depot(depot_path);

  depot.load_reads(reads);
  fprintf(stderr, "Read %lu reads\n", reads.size());

  depot.load_overlaps(overlaps, reads);
  fprintf(stderr, "Read %lu overlaps\n", overlaps.size());

  int had_overlaps = overlaps.size();

  auto filtered_duplicates = filter_best_overlap_per_pair(&overlaps);
  fprintf(stderr, "%d (%.2lf%%) overlaps filtered because had more than one overlap per read pair\n",
      filtered_duplicates, 100. * filtered_duplicates / had_overlaps);

  must_one_overlap_per_pair(overlaps);

  createReverseComplements(reads, thread_num);

  StringGraph* graph = new StringGraph(reads, overlaps);
  graph->simplify();

  vector<Overlap*> simplified_overlaps;
  graph->extractOverlaps(simplified_overlaps);
  write_overlaps(simplified_overlaps, working_directory + "/simplified.afg");

  fprintf(stderr, "Simplified string graph: %lu vertices, %lu edges\n", graph->getNumVertices(), graph->getNumEdges());

  std::vector<StringGraphWalk*> unitig_walks;
  graph->extract_unitigs(&unitig_walks);
  write_contigs_to_file(unitig_walks, (working_directory + "/unitigs_fast.fasta").c_str());

  std::cerr << "number of unitigs " << unitig_walks.size() << std::endl;

  vector<Contig*> unitigs;
  for (const auto& unitig_walk : unitig_walks) {
    Contig *unitig = new Contig(unitig_walk);
    unitigs.push_back(unitig);
  }
  writeAfgContigs(unitigs, (working_directory + "/unitigs.afg").c_str());

  std::vector<StringGraphWalk*> contig_walks;
  extract_contig_walks(&contig_walks, graph);

  write_contigs_to_file(contig_walks, (working_directory + "/contigs_fast.fasta").c_str());

  std::cerr << "number of contigs " << contig_walks.size() << std::endl;
  print_contigs_info(contig_walks, reads);

  vector<Contig*> contigs;
  for (const auto& contig_walk : contig_walks) {
    Contig *contig = new Contig(contig_walk);
    contigs.push_back(contig);
  }
  writeAfgContigs(contigs, (working_directory + "/contigs.afg").c_str());

  for (auto r: reads)           delete r;
  for (auto o: overlaps)        delete o;
  for (auto c: contig_walks)    delete c;
  for (auto u: unitig_walks)    delete u;
  for (auto c: contigs)         delete c;

  delete graph;

  return 0;
}
