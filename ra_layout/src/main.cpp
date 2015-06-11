#include <stdio.h>
#include <stdlib.h>

#include "ra/ra.hpp"

const struct option options[] = {
    {"reads", required_argument, 0, 'i'},
    {"overlaps", required_argument, 0, 'j'},
    {"threads", required_argument, 0, 't'},
    {"contigs-out", required_argument, 0, 'c'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static void help();

int main(int argc, char* argv[]) {

    char* readsPath = nullptr;
    char* overlapsPath = nullptr;

    int threadLen = std::max(std::thread::hardware_concurrency(), 1U);

    char* contigsOut = nullptr;

    while (1) {

        char argument = getopt_long(argc, argv, "i:j:t:h", options, nullptr);

        if (argument == -1) {
            break;
        }

        switch (argument) {
        case 'i':
            readsPath = optarg;
            break;
        case 'j':
            overlapsPath = optarg;
            break;
        case 't':
            threadLen = atoi(optarg);
            break;
        case 'c':
            contigsOut = optarg;
            break;
        default:
            help();
            return -1;
        }
    }

    ASSERT(readsPath, "IO", "missing option -i (reads file)");
    ASSERT(overlapsPath, "IO", "missing option -j (overaps file)");

    std::vector<Read*> reads;
    readAfgReads(reads, readsPath);

    createReverseComplements(reads, threadLen);

    std::vector<Overlap*> overlaps;
    readAfgOverlaps(overlaps, overlapsPath);

    StringGraph* graph = new StringGraph(reads, overlaps);

    graph->simplify();

    std::vector<Contig*> contigs;
    graph->extractContigs(contigs);

    writeAfgContigs(contigs, contigsOut == nullptr ? "layout.afg" : contigsOut);

    for (const auto& contig : contigs) {
        delete contig;
    }

    delete graph;

    for (const auto& it : overlaps) delete it;

    for (const auto& it : reads) delete it;

    return 0;
}

static void help() {

    printf(
    "usage: ra_layout -i <reads file> -j <overlaps file> [arguments ...]\n"
    "\n"
    "arguments:\n"
    "    -i, --reads <file>\n"
    "        (required)\n"
    "        input afg reads file\n"
    "    -j, --overlaps <file>\n"
    "        (required)\n"
    "        input afg overlaps file\n"
    "    -t, --threads <int>\n"
    "        default: approx. number of processors/cores\n"
    "        number of threads used\n"
    "    --contigsOut <file>\n"
    "        output afg contigs file\n"
    "    -h, -help\n"
    "        prints out the help\n");
}
