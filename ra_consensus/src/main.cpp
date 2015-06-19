#include <stdio.h>
#include <stdlib.h>

#include "ra/ra.hpp"

const struct option options[] = {
    {"reads", required_argument, 0, 'i'},
    {"contigs", required_argument, 0, 'j'},
    {"threads", required_argument, 0, 't'},
    {"out", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static void help();

int main(int argc, char* argv[]) {

    char* readsPath = nullptr;
    char* contigsPath = nullptr;

    int threadLen = std::max(std::thread::hardware_concurrency(), 1U);

    char* outPath = nullptr;

    while (1) {

        char argument = getopt_long(argc, argv, "i:j:t:o:h", options, nullptr);

        if (argument == -1) {
            break;
        }

        switch (argument) {
        case 'i':
            readsPath = optarg;
            break;
        case 'j':
            contigsPath = optarg;
            break;
        case 't':
            threadLen = atoi(optarg);
            break;
        case 'o':
            outPath = optarg;
            break;
        default:
            help();
            return -1;
        }
    }

    ASSERT(readsPath, "IO", "missing option -i (reads file)");
    ASSERT(contigsPath, "IO", "missing option -j (overaps file)");

    std::vector<Read*> reads;
    readAfgReads(reads, readsPath);

    createReverseComplements(reads, threadLen);

    std::vector<Contig*> contigs;
    readAfgContigs(contigs, contigsPath);

    std::vector<Read*> transcripts;

    Timer timer;
    timer.start();

    int id = 0;
    for (const auto& contig : contigs) {
        std::string consensusSeq = consensus(contig, reads);

        if (!consensusSeq.empty()) {
            transcripts.emplace_back(new AfgRead(id, std::to_string(id), consensusSeq, "", 1));
            ++id;
        }
    }

    timer.stop();
    timer.print("Consensus", "poa");

    writeFastaReads(transcripts, outPath);

    for (const auto& it : transcripts) delete it;

    for (const auto& it : contigs) delete it;

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
    "    -j, --contigs <file>\n"
    "        (required)\n"
    "        input afg contigs file\n"
    "    -t, --threads <int>\n"
    "        default: approx. number of processors/cores\n"
    "        number of threads used\n"
    "    -o, --out <file>\n"
    "        default: cout\n"
    "        output fasta transcripts file\n"
    "    -h, -help\n"
    "        prints out the help\n");
}
