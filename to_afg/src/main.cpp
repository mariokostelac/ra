#include <stdio.h>
#include <stdlib.h>

#include "ra/ra.hpp"

const struct option options[] = {
    {"reads", required_argument, 0, 'i'},
    {"fastq", no_argument, 0, 'q'},
    {"out", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static void help();

int main(int argc, char* argv[]) {

    char* readsPath = nullptr;

    bool fastq = false;

    char* outPath = nullptr;

    while (1) {

        char argument = getopt_long(argc, argv, "i:o:h", options, nullptr);

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
        case 'q':
            fastq = true;
            break;
        default:
            help();
            return -1;
        }
    }

    ASSERT(readsPath, "IO", "missing option -i (reads file)");

    std::vector<Read*> reads;

    if (fastq) {
        readFastqReads(reads, readsPath);
    } else {
        readFastaReads(reads, readsPath);
    }

    writeAfgReads(reads, outPath);

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
    "        input fasta/fastq reads file\n"
    "    --fastq\n"
    "        default: fasta format\n"
    "        format of input reads file\n"
    "    -o, --out <file>\n"
    "        output afg reads file\n"
    "    -h, -help\n"
    "        prints out the help\n");
}