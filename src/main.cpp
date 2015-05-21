#include <stdio.h>
#include <stdlib.h>

#include "IO.hpp"
#include "Preprocess.hpp"
#include "Overlap.hpp"

int main(int argc, char* argv[]) {

    Options* options = Options::parseOptions(argc, argv);

    if (options == NULL) return -1;

    std::vector<Read*> reads;
    readFastqReads(reads, options->readsPath);

    // correctReads(reads, options->k, options->c, options->threadLen, options->readsPath);

    std::vector<Read*> filteredReads;
    filterReads(filteredReads, reads);

    for (const auto& it : filteredReads) {
        delete it;
    }

    std::vector<Overlap*> overlaps;
    overlapReads(overlaps, reads, options->minOverlapLen, options->threadLen, options->readsPath);

    fprintf(stderr, "Overlaps num = %zu\n", overlaps.size());

    for (const auto& it : overlaps) {
        delete it;
    }

    for (const auto& it : reads) {
        delete it;
    }

    delete options;

    return 0;
}
