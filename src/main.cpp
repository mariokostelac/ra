#include <stdio.h>
#include <stdlib.h>

#include "IO.hpp"
#include "Preprocess.hpp"
#include "Overlap.hpp"
#include "StringGraph.hpp"
#include "EditDistance.hpp"

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

    std::vector<Overlap*> notContained;
    filterContainedOverlaps(notContained, overlaps, reads);

    std::vector<Overlap*> notTransitive;
    filterTransitiveOverlaps(notTransitive, notContained, options->threadLen);

    StringGraph graph(filteredReads, notTransitive);

    graph.trim();

    std::vector<Overlap*> trimmed;
    graph.extractOverlaps(trimmed);

    graph.popBubbles();

    std::vector<Overlap*> final;
    graph.extractOverlaps(final);

    writeAfgOverlaps(notTransitive, "notTransitive.afg");
    writeAfgOverlaps(trimmed, "trimmed.afg");
    writeAfgOverlaps(final, "final.afg");

    for (const auto& it : overlaps) delete it;

    for (const auto& it : reads) delete it;

    delete options;

    return 0;
}
