#include <stdio.h>
#include <stdlib.h>

#include "EnhancedSuffixArray.hpp"
#include "Read.hpp"
#include "IO.hpp"
#include "Preprocess.hpp"

int main(int argc, char* argv[]) {

    std::vector<Read*> reads;

    readFastqReads(reads, "test2.fa");

    errorCorrection(reads, 15, 4);

    EnhancedSuffixArray* esa = new EnhancedSuffixArray(reads);

    for (const auto& it : reads) {
        const auto& str = it->getSequence();

        std::vector<std::vector<int>> overlaps;
        esa->getOverlaps(overlaps, &str[0], str.size());
    }

    delete esa;

    for (const auto& it : reads) {
        delete it;
    }

    return 0;
}
