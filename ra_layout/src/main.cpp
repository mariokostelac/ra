#include <stdio.h>
#include <stdlib.h>

#include "ra/ra.hpp"

// trimming params
int READ_LEN_THRESHOLD = 100000;
uint32_t MAX_READS_IN_TIP = 2;
uint32_t MAX_DEPTH_WITHOUT_EXTRA_FORK = 5;

// BFS params in bubble popping
size_t MAX_NODES = 750;
int MAX_DISTANCE = 2500;
double MAX_DIFFERENCE = 0.05;

// contig extraction params
size_t MAX_BRANCHES = 20;
size_t MAX_START_NODES = 24;
double LENGTH_THRESHOLD = 0.0;
double QUALITY_THRESHOLD = 0.0;

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

    std::vector<Overlap*> overlaps;
    readAfgOverlaps(overlaps, overlapsPath);

    StringGraph* graph = new StringGraph(reads, overlaps);

    graph->simplify();

    std::vector<StringGraphComponent*> components;
    graph->extractComponents(components);

    Timer timer;
    timer.start();

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
                transcripts.emplace_back(new AfgRead(id, std::to_string(id), sequence, "", 1));
                ++id;
            }
        }

        writeFastaReads(transcripts, "transcripts.layout.fasta");

        for (const auto& it : transcripts) delete it;
    }

    // extract contigs from all graph components

    std::vector<Contig*> contigs;

    for (auto& component : components) {

        const auto& contig = new Contig(component->longestWalk());

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
