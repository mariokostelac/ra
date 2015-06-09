#include <stdio.h>
#include <stdlib.h>

#include "ra/ra.hpp"

const struct option options[] = {
    {"reads", required_argument, 0, 'i'},
    {"kmer", required_argument, 0, 'k'},
    {"threshold", required_argument, 0, 'c'},
    {"threads", required_argument, 0, 't'},
    {"out", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static void help();

int main(int argc, char* argv[]) {

    char* readsPath = nullptr;

    int k = -1;
    int c = -1;

    int threadLen = std::max(std::thread::hardware_concurrency(), 1U);

    char* outPath = nullptr;

    while (1) {

        char argument = getopt_long(argc, argv, "i:o:k:c:t:h", options, nullptr);

        if (argument == -1) {
            break;
        }

        switch (argument) {
        case 'i':
            readsPath = optarg;
            break;
        case 'o':
            outPath = optarg;
            break;
        case 'k':
            k = atoi(optarg);
            break;
        case 'c':
            c = atoi(optarg);
            break;
        case 't':
            threadLen = atoi(optarg);
            break;
        default:
            help();
            return -1;
        }
    }

    ASSERT(readsPath, "IO", "missing option -i (reads file)");
    ASSERT(threadLen > 0, "IO", "invalid thread number");

    std::vector<Read*> reads;
    readAfgReads(reads, readsPath);

    if (correctReads(reads, k, c, threadLen, readsPath)) {
        writeAfgReads(reads, outPath);
    }

    for (const auto& read : reads) delete read;

    return 0;
}

static void help() {

    printf(
    "usage: to_afg -i <reads file> [arguments ...]\n"
    "\n"
    "arguments:\n"
    "    -i, --reads <file>\n"
    "        (required)\n" 
    "        input afg reads file\n"
    "    -k, --kmer <int>\n"
    "        default: based on dataset\n"
    "        length of k-mers used in error correction\n"
    "   -c, --threshold <int>\n"
    "        default: based on dataset\n"
    "        minimal number of occurrences for a k-mer to not be erroneous\n"
    "    -o, --out <file>\n"
    "        default: cout\n"
    "        output afg corrected reads file\n"
    "    -h, -help\n"
    "        prints out the help\n");
}