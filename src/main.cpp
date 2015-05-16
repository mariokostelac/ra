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

    // errorCorrection(reads, options->k, options->c, options->threadLen, options->readsPath);

    std::vector<Overlap*> overlaps;
    getOverlaps(overlaps, reads, options->minOverlapLen, options->threadLen, options->readsPath);

    for (const auto& it : overlaps) {
        it->print();
        delete it;
    }

    for (const auto& it : reads) {
        delete it;
    }

    delete options;

    return 0;
}
