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

    std::vector<Read*> filteredReads;
    filterReads(filteredReads, reads);

    correctReads(filteredReads, options->k, options->c, options->threadLen, options->readsPath);

    std::vector<Overlap*> overlaps;
    overlapReads(overlaps, filteredReads, options->minOverlapLen, options->threadLen, options->readsPath);

    writeAfgOverlaps(overlaps, "overlaps.afg");

    for (const auto& it : overlaps) delete it;

    for (const auto& it : reads) delete it;

    delete options;

    return 0;
}
