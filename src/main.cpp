#include <stdio.h>
#include <stdlib.h>

#include "EnhancedSuffixArray.hpp"
#include "Read.hpp"
#include "IO.hpp"
#include "Preprocess.hpp"

int main(int argc, char* argv[]) {

    std::vector<Read*> reads;

    readFastqReads(reads, "test2.fa");

    errorCorrection(reads, 15, 4, "test2.fa");

    for (const auto& it : reads) {
        delete it;
    }

    return 0;
}
