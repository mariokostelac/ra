#include <stdio.h>
#include <stdlib.h>

#include "ra/ra.hpp"

const struct option options[] = {
    {"reads", required_argument, 0, 'i'},
    {"min-overlap-length", required_argument, 0, 'm'},
    {"threads", required_argument, 0, 't'},
    {"reads-out", required_argument, 0, 'r'},
    {"overlaps-out", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static void help();

int main(int argc, char* argv[]) {

    char* readsPath = nullptr;

    int minOverlapLen = 5;
    int threadLen = std::max(std::thread::hardware_concurrency(), 1U);

    char* readsOut = nullptr;
    char* overlapsOut = nullptr;

    while (1) {

        char argument = getopt_long(argc, argv, "i:m:t:h", options, nullptr);

        if (argument == -1) {
            break;
        }

        switch (argument) {
        case 'i':
            readsPath = optarg;
            break;
        case 'm':
            minOverlapLen = atoi(optarg);
            break;
        case 't':
            threadLen = atoi(optarg);
            break;
        case 'r':
            readsOut = optarg;
            break;
        case 'o':
            overlapsOut = optarg;
            break;
        default:
            help();
            return -1;
        }
    }

    ASSERT(readsPath, "IO", "missing option -i (reads file)");
    ASSERT(minOverlapLen > 0, "IO", "invalid minimal overlap length");
    ASSERT(threadLen > 0, "IO", "invalid thread number");

    std::vector<Read*> reads;
    readAfgReads(reads, readsPath);

    std::vector<Read*> filtered;
    filterReads(filtered, reads);

    createReverseComplements(filtered, threadLen);

    std::vector<Overlap*> overlaps;
    overlapReads(overlaps, filtered, minOverlapLen, threadLen, readsPath);

    std::vector<Overlap*> notContained;
    filterContainedOverlaps(notContained, overlaps, filtered);

    std::vector<Overlap*> notTransitive;
    filterTransitiveOverlaps(notTransitive, notContained, threadLen);

    updateOverlapIds(notTransitive, filtered);

    writeAfgOverlaps(notTransitive, overlapsOut == nullptr ? "overlaps.afg" : overlapsOut);
    writeAfgReads(reads, readsOut == nullptr ? "reads.afg" : readsOut);

    for (const auto& it : overlaps) delete it;

    for (const auto& it : reads) delete it;

    return 0;
}

static void help() {

    printf(
    "usage: ra_overlap -i <reads file> [arguments ...]\n"
    "\n"
    "arguments:\n"
    "    -i, --reads <file>\n"
    "        (required)\n"
    "        input afg reads file\n"
    "    -t, --threads <int>\n"
    "        default: approx. number of processors/cores\n"
    "        number of threads used\n"
    "    -m, --min-overlap-length <int>\n"
    "        default: 5\n"
    "        minimal length of exact overlap between two reads\n"
    "    --reads-out <file>\n"
    "        output afg updated reads file\n"
    "    --overlaps-out <file>\n"
    "        output afg overlaps file\n"
    "    -h, -help\n"
    "        prints out the help\n");
}