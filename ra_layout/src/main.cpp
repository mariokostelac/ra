#include <stdio.h>
#include <stdlib.h>

#include "ra/ra.hpp"

const struct option options[] = {
    {"reads", required_argument, 0, 'i'},
    {"overlaps", required_argument, 0, 'j'},
    {"threads", required_argument, 0, 't'},
    {"out", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

static void help();

int main(int argc, char* argv[]) {

    char* readsPath = nullptr;
    char* overlapsPath = nullptr;

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
            overlapsPath = optarg;
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
    ASSERT(overlapsPath, "IO", "missing option -j (overaps file)");

    std::vector<Read*> reads;
    readAfgReads(reads, readsPath);

    createReverseComplements(reads, threadLen);

    std::vector<Overlap*> overlaps;
    readAfgOverlaps(overlaps, overlapsPath);

    StringGraph* graph = new StringGraph(reads, overlaps);

    graph->simplify();

    std::vector<StringGraphComponent*> components;
    graph->extractComponents(components);

    {
        // extraction of transcripts is done in the consensus phase BUT as the
        // overlapper is exact, it can be done in layout phase as well (for now
        // its here for testing purposes)

        std::vector<Read*> transcripts;
        int id = 0;

        for (const auto& component : components) {

            std::string sequence;
            component->extractSequence(sequence);

            if (!sequence.empty()) {
                transcripts.emplace_back(new Read(id, std::to_string(id), sequence, "", 1));
                ++id;
            }
        }

        writeFastaReads(transcripts, "transcripts.layout.fasta");

        for (const auto& it : transcripts) delete it;
    }

    // extract contigs from all graph components

    Timer timer;
    timer.start();

    std::vector<Contig*> contigs;

    for (const auto& component : components) {

        Contig* contig = component->createContig();

        if (contig != nullptr) {
            contigs.emplace_back(contig);
        }
    }

    fprintf(stderr, "[Layout][contig extraction]: number of contigs %zu\n", contigs.size());

    timer.stop();
    timer.print("Layout", "contig extraction");

    writeAfgContigs(contigs, outPath);

    for (const auto& it : contigs) delete it;

    for (const auto& it : components) delete it;

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
    "    -o --out <file>\n"
    "        default: cout\n"
    "        output afg contigs file\n"
    "    -h, -help\n"
    "        prints out the help\n");
}
